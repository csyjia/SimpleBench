#include "benchmarking.h"

#define bench_mmap(_s)      \
      _bench_mmap((size_t)(_s), __FILE__, __LINE__)

void *_bench_mmap(size_t size, const char *name, int line);
