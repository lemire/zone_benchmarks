#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libzscanner/scanner.h>

static const void error(const char *str)
{
  fprintf(stderr, "%s\n", str);
  exit(1);
}

static const void accept_rr(zs_scanner_t *scanner)
{
  *(size_t *)scanner->process.data += 1;
}

int main(int argc, char *argv[])
{
  zs_scanner_t scanner;
  size_t rrs = 0;

  if (argc != 2)
    return 1;

  if (zs_init(&scanner, "com.", 1, 3600) == -1)
    error("Could (K)not initialize scanner");
  if (zs_set_processing(&scanner, &accept_rr, 0, &rrs) == -1)
    error("Could (K)not set scanner callbacks");
  if (zs_set_input_file(&scanner, argv[1]) == -1)
    error("Could (K)not set input string");
  if (zs_parse_all(&scanner) == -1)
    error("Could (K)not parse input");

  printf("parsed %zu records!\n", rrs);

  return 0;
}
