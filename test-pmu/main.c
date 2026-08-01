#include "../utils/utils.h"
#include <stdio.h>

int main() {
  // avvia la profilazione
  if (beg_pmu()) {
    printf("Errore avvio PMU\n");
  }

  volatile int a;
  for (int i = 0; i < 1000000; i++) {
    a++;
  }

  // termina la profilazione
  uint64_t ret;
  if (!end_pmu(&ret)) {
    printf("Errore termine PMU\n");
  }

  printf("Cicli\n%llu\n", ret);

  return 0;
}
