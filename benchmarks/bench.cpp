#include "performancecounters/benchmarker.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <filesystem>
extern "C" {
#include <libzscanner/scanner.h>
#include <libzscanner/version.h>
}
#include "simdzonehelper.h"

// Function to read a file block by block into a buffer, and return the total
// number of bytes read This function is not actually 'useful' but it is used to
// measure how quickly we can read the contents of a file into a buffer.
size_t read_file_into_buffer(const char *filename) {
  FILE *fp = fopen(filename, "rb"); // Open the file in binary mode
  if (fp == NULL) {
    perror("Error opening file");
    return 0;
  }
  const size_t buffer_size = 1024; // Initial buffer size
  char buffer[buffer_size];        // Buffer to store the file contents

  if (buffer == NULL) {
    perror("Error allocating memory for buffer");
    fclose(fp);
    return 0;
  }

  size_t total_bytes_read = 0;
  size_t bytes_read;

  while ((bytes_read = fread(buffer, 1, buffer_size, fp)) > 0) {
    total_bytes_read += bytes_read;
  }

  fclose(fp);
  return total_bytes_read;
}
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

void benchmark(std::filesystem::path filename, std::string filter) {
  size_t volume = std::filesystem::file_size(filename);
  std::string benchname;
  printf("Benchmarking file %s\n", filename.c_str());
  printf("Volume: %zu bytes\n", volume);
  volatile size_t bytes_read = 0;
  pretty_print(1, volume, "read_file", bench([&filename, &bytes_read]() {
                 bytes_read += read_file_into_buffer(filename.c_str());
               }));
  const kernel_t *kernel;
  for (const char *kernel_name : {"haswell", "westmere", "fallback"}) {
    if (!(kernel = select_kernel(kernel_name))) {
      printf("# skipping %s\n", kernel_name);
      continue;
    }
    benchname = std::string("simdzone") + kernel->name;
    if (benchname.find(filter) == std::string::npos) {
      continue;
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
  }
  benchname = "knot";
  benchname += std::to_string(ZSCANNER_VERSION_MAJOR);
  benchname += ".";
  benchname += std::to_string(ZSCANNER_VERSION_MINOR);
  benchname += ".";
  benchname += std::to_string(ZSCANNER_VERSION_PATCH);
  if (benchname.find(filter) != std::string::npos) {
    pretty_print(1, volume, benchname, bench([&filename]() {
                   zs_scanner_t scanner;
                   size_t rrs = 0;
                   if (zs_init(&scanner, "com.", 1, 3600) == -1) {
                     error("Could not initialize scanner");
                   }
                   if (zs_set_processing(&scanner, &accept_rr, 0, &rrs) == -1) {
                     error("Could not set scanner callbacks");
                   }
                   if (zs_set_input_file(&scanner, filename.c_str()) == -1) {
                     error("Could not set input string");
                   }
                   if (zs_parse_all(&scanner) == -1) {
                     error("Could not parse input");
                   }
                 }));
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "please provide a zone file to parse\n";
    return EXIT_FAILURE;
  }
  std::string filename = argv[1];
  std::string filter;
  if (argc > 2) {
    filter = argv[2];
  }

  if (!std::filesystem::exists(filename)) {
    std::cerr << "File " << filename << " does not exist.\n";
    return EXIT_FAILURE;
  }
  benchmark(filename, filter);
  return EXIT_SUCCESS;
}