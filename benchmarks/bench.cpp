#include "performancecounters/benchmarker.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
extern "C" {
#include <libzscanner/scanner.h>
}
static const void error(const char *str) {
  fprintf(stderr, "%s\n", str);
  exit(1);
}

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