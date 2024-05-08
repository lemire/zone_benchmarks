
#ifndef BENCHMARKS_SIMDZONEHELPER_H_
#define BENCHMARKS_SIMDZONEHELPER_H_
#include <assert.h>
#include <stdint.h>
#include <strings.h>

extern "C" {
#include <zone.h>
// I think that these be under zone/ e.g., zone/config.h, zone/isadetection.h.
#include <attributes.h>
#include <config.h>
#include <diagnostic.h>
#include <isadetection.h>
extern int32_t zone_fallback_parse(zone_parser_t *);
#if HAVE_HASWELL
extern int32_t zone_haswell_parse(zone_parser_t *);
#endif
#if HAVE_WESTMERE
extern int32_t zone_westmere_parse(zone_parser_t *);
#endif
extern int32_t zone_open(zone_parser_t *, const zone_options_t *,
                         zone_buffers_t *, const char *, void *user_data);
extern void zone_close(zone_parser_t *);
}

typedef struct kernel kernel_t;
struct kernel {
  const char *name;
  uint32_t instruction_set;
  int32_t (*parse)(zone_parser_t *);
};
static const kernel_t kernels[] = {
#if HAVE_HASWELL
    {"haswell", AVX2, &zone_haswell_parse},
#endif
#if HAVE_WESTMERE
    {"westmere", SSE42, &zone_westmere_parse},
#endif
    {"fallback", DEFAULT, &zone_fallback_parse}};
static const kernel_t *select_kernel(const char *name) {
  const size_t n = sizeof(kernels) / sizeof(kernels[0]);
  const uint32_t supported = detect_supported_architectures();
  const kernel_t *kernel = NULL;

  if ((!name || !*name) && !(name = getenv("ZONE_KERNEL"))) {
    for (size_t i = 0; !kernel && i < n; i++) {
      if ((kernels[i].instruction_set & supported) ==
          kernels[i].instruction_set)
        kernel = &kernels[i];
    }
    assert(kernel != NULL);
  } else {
    for (size_t i = 0; !kernel && i < n; i++) {
      if (strcasecmp(name, kernels[i].name) == 0)
        kernel = &kernels[i];
    }

    if (!kernel ||
        (kernel->instruction_set && !(kernel->instruction_set & supported))) {
      fprintf(stderr, "Target %s is unavailable\n", name);
      return NULL;
    }
  }
  return kernel;
}

typedef zone_parser_t parser_t;

static int32_t bench_accept(parser_t *parser, const zone_name_t *owner,
                            uint16_t type, uint16_t classt, uint32_t ttl,
                            uint16_t rdlength, const uint8_t *rdata,
                            void *user_data) {
  (void)parser;
  (void)owner;
  (void)type;
  (void)classt;
  (void)ttl;
  (void)rdlength;
  (void)rdata;
  (*(size_t *)user_data)++;
  return ZONE_SUCCESS;
}

static int32_t bench_parse(zone_parser_t *parser, const kernel_t *kernel) {
  size_t records = 0;
  int32_t result;

  parser->user_data = &records;
  result = kernel->parse(parser);

  return result;
}
#endif // BENCHMARKS_SIMDZONEHELPER_H_