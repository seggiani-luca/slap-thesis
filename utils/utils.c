#include "utils.h"
#include <mach/mach_time.h>
#include <stdlib.h>

uint64_t get_time() {
  // ottieni info da mach
  static mach_timebase_info_data_t info = {0};
  if (info.denom == 0)
    mach_timebase_info(&info);

  // ottieni tick corrente
  uint64_t t = mach_absolute_time();

  // restituisci tempo corrente in millisecondi
  return t * info.numer / info.denom;
}

// Funzione di comparazione per qsort
int compare(const void *a, const void *b) {
  return (*(uint64_t *)a > *(uint64_t *)b) - (*(uint64_t *)a < *(uint64_t *)b);
}

double median(uint64_t arr[], int n) {
  qsort(arr, n, sizeof(uint64_t), compare);
  if (n % 2 == 0) {
    return (double)(arr[n / 2 - 1] + arr[n / 2]) / 2.0;
  } else {
    return (double)arr[n / 2];
  }
}

int rand_range(int min, int max) { return min + rand() % (max - min + 1); }

int min(int a, int b) { return (a < b) ? a : b; }
