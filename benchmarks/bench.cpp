#include "performancecounters/benchmarker.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <filesystem>
extern "C" {
#include <libzscanner/scanner.h>
}
#include "simdzonehelper.h"

static const void error(const char *str) {
  fprintf(stderr, "%s\n", str);
  exit(1);
}

// for knot
static void accept_rr(zs_scanner_t *scanner) {
  *(size_t *)scanner->process.data += 1;
}

void pretty_print(size_t volume, size_t bytes, std::string name,
                  event_aggregate agg) {
  printf("%-45s : ", name.c_str());
  double best_speed = bytes / agg.fastest_elapsed_ns();
  double avg_speed = bytes / agg.elapsed_ns();
  double range = (best_speed - avg_speed) / avg_speed * 100.0;
  printf(" %5.2f GB/s (%2.0f %%) ", bytes / agg.fastest_elapsed_ns(), range);
  if (collector.has_events()) {
    printf(" %5.2f GHz ", agg.fastest_cycles() / agg.fastest_elapsed_ns());
    printf(" %5.2f c/b ", agg.fastest_cycles() / bytes);
    printf(" %5.2f i/b ", agg.fastest_instructions() / bytes);
    printf(" %5.2f i/c ", agg.fastest_instructions() / agg.fastest_cycles());
  }
  printf("\n");
}

void benchmark(std::filesystem::path filename) {
  size_t volume = std::filesystem::file_size(filename);
  printf("Benchmarking file %s\n", filename.c_str());
  printf("Volume: %zu bytes\n", volume);
  const kernel_t *kernel;
  if (!(kernel = select_kernel(NULL))) {
    error("Could (K)not select simdzone kernel");
  }
  pretty_print(1, volume, std::string("simdzone") + kernel->name,
               bench([&filename, kernel]() {
                 zone_parser_t parser;
                 memset(&parser, 0, sizeof(parser));
                 zone_options_t options;
                 memset(&options, 0, sizeof(options));
                 options.pretty_ttls = true;
                 static uint8_t root[] = {0};
                 options.origin.octets = root;
                 options.origin.length = 1;
                 options.accept.callback = &bench_accept;
                 options.default_ttl = 3600;
                 options.default_class = 1;

                 zone_name_buffer_t owner;
                 zone_rdata_buffer_t rdata;
                 zone_buffers_t buffers = {1, &owner, &rdata};

                 if (zone_open(&parser, &options, &buffers, filename.c_str(),
                               NULL) < 0) {
                   error("simdzone: could not open zone file");
                 }
                 if (bench_parse(&parser, kernel) < 0) {
                   error("simdzone: some error occurred during benchmarking");
                 }

                 zone_close(&parser);
               }));
  pretty_print(1, volume, "knot", bench([&filename]() {
                 zs_scanner_t scanner;
                 size_t rrs = 0;
                 if (zs_init(&scanner, "com.", 1, 3600) == -1) {
                   error("Could (K)not initialize scanner");
                 }
                 if (zs_set_processing(&scanner, &accept_rr, 0, &rrs) == -1) {
                   error("Could (K)not set scanner callbacks");
                 }
                 if (zs_set_input_file(&scanner, filename.c_str()) == -1) {
                   error("Could (K)not set input string");
                 }
                 if (zs_parse_all(&scanner) == -1) {
                   error("Could (K)not parse input");
                 }
               }));
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "please provide a zone file to parse\n";
    return EXIT_FAILURE;
  }
  std::string filename = argv[1];

  if (!std::filesystem::exists(filename)) {
    std::cerr << "File " << filename << " does not exist.\n";
    return EXIT_FAILURE;
  }
  benchmark(filename);
  return EXIT_SUCCESS;
}

/*
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if _WIN32
# include "getopt.h"
#else
# include <strings.h>
# include <unistd.h>
#endif

#include "attributes.h"
#include "config.h"
#include "diagnostic.h"
#include "isadetection.h"
#include "zone.h"

#if _MSC_VER
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
#endif

typedef zone_parser_t parser_t;

#if HAVE_HASWELL
extern int32_t zone_bench_haswell_lex(zone_parser_t *, size_t *);
extern int32_t zone_haswell_parse(zone_parser_t *);
#endif

#if HAVE_WESTMERE
extern int32_t zone_bench_westmere_lex(zone_parser_t *, size_t *);
extern int32_t zone_westmere_parse(zone_parser_t *);
#endif

extern int32_t zone_bench_fallback_lex(zone_parser_t *, size_t *);
extern int32_t zone_fallback_parse(zone_parser_t *);

typedef struct kernel kernel_t;
struct kernel {
  const char *name;
  uint32_t instruction_set;
  int32_t (*bench_lex)(zone_parser_t *, size_t *);
  int32_t (*parse)(zone_parser_t *);
};


extern int32_t zone_open(
  zone_parser_t *,
  const zone_options_t *,
  zone_buffers_t *,
  const char *,
  void *user_data);

extern void zone_close(
  zone_parser_t *);

static int32_t bench_lex(zone_parser_t *parser, const kernel_t *kernel)
{
  size_t tokens = 0;
  int32_t result;

  if ((result = kernel->bench_lex(parser, &tokens)) < 0)
    return result;

  printf("Lexed %zu tokens\n", tokens);
  return 0;
}

static int32_t bench_accept(
  parser_t *parser,
  const zone_name_t *owner,
  uint16_t type,
  uint16_t class,
  uint32_t ttl,
  uint16_t rdlength,
  const uint8_t *rdata,
  void *user_data)
{
  (void)parser;
  (void)owner;
  (void)type;
  (void)class;
  (void)ttl;
  (void)rdlength;
  (void)rdata;
  (*(size_t *)user_data)++;
  return ZONE_SUCCESS;
}

static int32_t bench_parse(zone_parser_t *parser, const kernel_t *kernel)
{
  size_t records = 0;
  int32_t result;

  parser->user_data = &records;
  result = kernel->parse(parser);

  printf("Parsed %zu records\n", records);
  return result;
}

diagnostic_push()
msvc_diagnostic_ignored(4996)


diagnostic_pop()

static void help(const char *program)
{
  const char *format =
    "Usage: %s [OPTION] <lex or parse> <zone file>\n"
    "\n"
    "Options:\n"
    "  -h         Display available options.\n"
    "  -t target  Select target (default:%s)\n"
    "\n"
    "Kernels:\n";

  printf(format, program, kernels[0].name);

  for (size_t i=0, n=sizeof(kernels)/sizeof(kernels[0]); i < n; i++)
    printf("  %s\n", kernels[i].name);
}

static void usage(const char *program)
{
  fprintf(stderr, "Usage: %s [OPTION] <lex or parse> <zone file>\n", program);
  exit(EXIT_FAILURE);
}

static uint8_t root[] = { 0 };

int main(int argc, char *argv[])
{
  const char *name = NULL, *program = argv[0];

  for (const char *slash = argv[0]; *slash; slash++)
    if (*slash == '/' || *slash == '\\')
      program = slash + 1;

  for (int option; (option = getopt(argc, argv, "ht:")) != -1;) {
    switch (option) {
      case 'h':
        help(program);
        exit(EXIT_SUCCESS);
      case 't':
        name = optarg;
        break;
      default:
        usage(program);
    }
  }

  if (optind > argc || argc - optind < 2)
    usage(program);

  int32_t (*bench)(zone_parser_t *, const kernel_t *) = 0;
  if (strcasecmp(argv[optind], "lex") == 0)
    bench = &bench_lex;
  else if (strcasecmp(argv[optind], "parse") == 0)
    bench = &bench_parse;
  else
    usage(program);

  const kernel_t *kernel;
  if (!(kernel = select_kernel(name)))
    exit(EXIT_FAILURE);

  zone_parser_t parser;
  memset(&parser, 0, sizeof(parser));
  zone_options_t options;
  memset(&options, 0, sizeof(options));
  options.pretty_ttls = true;
  options.origin.octets = root;
  options.origin.length = 1;
  options.accept.callback = &bench_accept;
  options.default_ttl = 3600;
  options.default_class = 1;

  zone_name_buffer_t owner;
  zone_rdata_buffer_t rdata;
  zone_buffers_t buffers = { 1, &owner, &rdata };

  if (zone_open(&parser, &options, &buffers, argv[argc-1], NULL) < 0)
    exit(EXIT_FAILURE);
  if (bench(&parser, kernel) < 0)
    exit(EXIT_FAILURE);

  zone_close(&parser);
  return EXIT_SUCCESS;
}*/