#include "../utils/utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/qos.h>
#include <sys/mman.h>

// Valore minimo di ITERS
#define MIN_ITERS 10

// Valore massimo di ITERS
#define MAX_ITERS 1000

// Passo nello scorrimento fra MIN_ITERS e MAX_ITERS
#define ITERS_STEP 10

// Byte di stride: 32 byte per MAX_ITERS a 1000 dà 32 KiB, che entra nei 128
// KiB di cache L1 dei P-core della serie M
#define STRIDE_BYTES 32

// Valore di stride: per int su 32 bit, con 32 byte di stride vale 8 int
#define STRIDE (STRIDE_BYTES / sizeof(int))

// Numero di tentativi per iterazione
#define REPS 100

// Dimensione di pagina su processori della serie M, che è 16 KiB
#define PAGE_SZ 16384

int main() {
  // imposta affinità CPU attraverso pthread_set_qos_class_self_np (in quanto
  // non abbiamo a disposizione KDK)
  pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);

  // imposta seed casuale
  srand((unsigned int)time(NULL));

  // stampa header del CSV
  printf("%s, %s, %s\n",
    "Loop Random Addr + Random Value",
	"Loop Striding Addr + Striding Value",
	"Loop Striding Addr + Random Value"
  );

  // esegui iterazioni da MIN_ITERS a MAX_ITERS, passo ITERS_STEP
  for (int ITERS = MIN_ITERS; ITERS <= MAX_ITERS; ITERS += ITERS_STEP) {
    // alloca un buffer abbastanza grande per il training del LAP
    const int NUM_PAGES_BUF = 1 + (ITERS * STRIDE * sizeof(int)) / PAGE_SZ;
    void *buffer = mmap(NULL, NUM_PAGES_BUF * PAGE_SZ, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // verifica fallimenti allocazione buffer
    if (buffer == MAP_FAILED) {
      printf("Failed to allocate buffer pages\n");
      return EXIT_FAILURE;
    } else {
      // ripulisci il buffer
      memset(buffer, 0x0, NUM_PAGES_BUF * PAGE_SZ);
    }

    // definisci il valore massimo che possiamo scrivere in maniera tale che,
    // se usato come indice, il risultato non sarò out of bounds
    const int WRITE_MAX = NUM_PAGES_BUF * PAGE_SZ / sizeof(int);

    // dichiarato come volatile per evitare ottimizzazioni dal compilatore
    volatile register int junk;

    // riferisci il buffer come array di interi
    int *ptr = (int *)buffer;

    // array di misura
    uint64_t timings[REPS];

    // 1. test per valori casuali + indirizzi casuali
    for (int i = 0; i < WRITE_MAX; ++i) {
      ptr[i] = rand_range(0, WRITE_MAX);
    }

    // dry run per portare gli indirizzi in cache, dimostrando che lo speedup
    // non è dovuto al prefetching in cache
    junk = 0;
    for (int i = 0; i < ITERS; ++i) {
      junk = ptr[junk];
    }

	// wet run per valori casuali + indirizzi casuali
    for (int i = 0; i < REPS; ++i) {
      junk = 0;

      // inizia profilazione
      beg_pmu();

      for (int i = 0; i < ITERS; ++i) {
        junk = ptr[junk];
      }

      // termina profilazione
      timings[i] = end_pmu();
    }
	
    printf("%f, ", median(timings, REPS));

    // 2. test per valori striding + indirizzi striding 
    int index = 0;
    while (index + STRIDE < WRITE_MAX) {
      ptr[index] = index + STRIDE;
      index += STRIDE;
    }

	// dry run
    junk = 0;
    for (int i = 0; i < ITERS; ++i) {
      junk = ptr[junk];
    }

	// wet run per valori striding + indirizzi striding
    for (int i = 0; i < REPS; ++i) {
      junk = 0;
      
	  // inizia profilazione
      beg_pmu();
      
	  for (int i = 0; i < ITERS; ++i) {
        junk = ptr[junk];
      }
      
	  // termina profilazione
      timings[i] = end_pmu();
    }
    
	printf("%f, ", median(timings, REPS));
   

	// test per il LAP: valori casuali + indirizzi striding, rendendo junk il
	// minimo fra lo stride e il valore caricato, e rendendo il valore caricato
	// almeno maggiore dello stride
    for (int i = 0; i < WRITE_MAX; ++i) {
      ptr[i] = rand_range(STRIDE, WRITE_MAX);
    }

	// dry run
    junk = 0;
    for (int i = 0; i < ITERS; ++i) {
      junk += min(ptr[junk], STRIDE);
    }

	// wet run per valori casuali + indirizzi striding
    for (int i = 0; i < REPS; ++i) {
      junk = 0;
	  
	  // inizia profilazione
      beg_pmu();
      
	  for (int i = 0; i < ITERS; ++i) {
        junk += min(ptr[junk], STRIDE);
      }
	  
	  // termina profilazione
      timings[i] = end_pmu();
    }
	printf("%f, ", median(timings, REPS));

    // ripulisci le allocazioni 
    if (munmap(buffer, NUM_PAGES_BUF * PAGE_SZ) == -1) {
      printf("Failed to deallocate buffer pages\n");
      return EXIT_FAILURE;
    }
  }
}
