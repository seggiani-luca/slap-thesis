#include "utils.h"
#include <mach/mach_time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/qos.h>
#include <time.h>

// Valore minimo di ITERS.
#define MIN_ITERS 10

// Valore massimo di ITERS.
#define MAX_ITERS 1000

// Passo nello scorrimento fra MIN_ITERS e MAX_ITERS.
#define ITERS_STEP 10

#define STRIDE_BYTES 32

// Valore di S.
#define S (STRIDE_BYTES / sizeof(int))

// Numero di misure eseguite per ciascun valore di ITERS.
#define TRIES 100

/**
 * Compara due numeri, restituendo il minore.
 *
 * @param a Il primo numero.
 * @param b Il secondo numero.
 *
 * @return Il minimo fra a e b.
 */
int min(int a, int b) { return (a < b) ? a : b; }

/**
 * Restituisce il tempo corrente in nanosecondi. Si appoggia alla
 * mach_absolute_time per ottenere tempo indipendente dal wall clock.
 *
 * @return Tempo corrente in nanosecondi.
 */
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

/**
 * Effettua una percorrenza completa dell'array, misurando il tempo impiegato.
 * La funzione esegue due passate:
 * - Dry run: porta gli indirizzi in cache per escludere il prefetching;
 * - Wet run: misura effettivamente il tempo (e i guadagni dovuti al LAP).
 *
 * Gli accessi formano dipendenze RAW (Read After Write) per forzare
 * l'esecuzione sequenziale da parte processore.
 *
 * @param in Puntatore all'array.
 * @param ITERS Numero di iterazioni della sequenza di accessi.
 *
 * @return Tempo impiegato dalla wet run.
 */
uint64_t stride(int *in, int ITERS) {
  // array e dep volatili per evitare ottimizzazioni
  volatile int *array = in;
  volatile int dep;

  // effettua la dry run
  dep = 0;
  for (int i = 0; i < ITERS; ++i) {
    dep = array[dep];
  }

  // avvia il timer
  uint64_t start = get_time();

  // effettua la wet run
  dep = 0;
  for (int i = 0; i < ITERS; ++i) {
    dep = array[dep];
  }

  // ferma il timer
  uint64_t end = get_time();

  // restituisci tempo impiegato dalla wet run
  return end - start;
}

/**
 * Effettua una percorrenza completa dell'array, misurando il tempo impiegato,
 * secondo l'eseprimento Stride Address + Random Value.
 * Come la stride, la funzione esegue due passate:
 * - Dry run: porta gli indirizzi in cache per escludere il prefetching;
 * - Wet run: misura effettivamente il tempo (e i guadagni dovuti al LAP).
 *
 * Gli accessi formano dipendenze RAW (Read After Write) per forzare
 * l'esecuzione sequenziale da parte processore.
 *
 * @param in Puntatore all'array.
 * @param ITERS Numero di iterazioni della sequenza di accessi.
 *
 * @return Tempo impiegato dalla wet run.
 */
uint64_t sarv(int *in, int ITERS) {
  // array e dep volatili per evitare ottimizzazioni
  volatile int *array = in;
  volatile int dep;

  // effettua la dry run
  dep = 0;
  for (int i = 0; i < ITERS; ++i) {
    dep += min(array[dep], S);
  }

  // avvia il timer
  uint64_t start = get_time();

  // effettua la wet run
  dep = 0;
  for (int i = 0; i < ITERS; ++i) {
    dep += min(array[dep], S);
  }

  // ferma il timer
  uint64_t end = get_time();

  // restituisci tempo impiegato dalla wet run
  return end - start;
}

/**
 * Misura la latenza degli accessi in memoria in due diverse configurazioni di
 * accesso a un array:
 * - Array collegato: ogni elemento punta al successivo con uno stride fisso;
 * - Array casuale: ottenuto tramite una permutazione casuale. Si permuta per
 *   evitare cicli nei collegamenti (0 -> 1, 1 -> 0, invalidano i risultati).
 *
 * @param ITERS Numero di iterazioni della sequenza di accessi.
 * @param link_time puntatore al ritorno del tempo per l'array collegato
 * @param rand_time puntatore al ritorno del tempo per l'array casuale
 */
void measure(int ITERS, uint64_t *link_time, uint64_t *rand_time,
             uint64_t *sarv_time) {
  // usa un unico array
  int array[S * ITERS];

  // inizializza array collegato
  for (int i = 0; i < ITERS - 1; i++) {
    array[i * S] = (i + 1) * S;
  }

  // misura array collegato
  *link_time = stride(array, ITERS);

  // inizializza array casuale permutando per evitare cicli
  int *perm = malloc(ITERS * sizeof(int));
  for (int i = 0; i < ITERS; i++) {
    perm[i] = i;
  }
  for (int i = ITERS - 1; i > 1; i--) {
    int j = 1 + rand() % i;
    int t = perm[i];
    perm[i] = perm[j];
    perm[j] = t;
  }
  for (int i = 0; i < ITERS; i++) {
    array[i * S] = perm[i] * S;
  }
  free(perm);

  // misura array casuale
  *rand_time = stride(array, ITERS);

  // inizializza array SA + RV con valori casuali multipli di S 
  for (int i = 0; i < ITERS; i++) {
    array[i * S] = ((rand() % (ITERS - 1)) + 1) * S;
  }

  // misura array SA + RV
  *sarv_time = sarv(array, ITERS);
}

/**
 * Helper che confronta due uint64_t, usato per l'ordinamento.
 *
 * @param a Il primo uint64_t.
 * @param b Il secondo uint64_t.
 *
 * @return Un valore di confronto fra a e b.
 */
int compare(const void *a, const void *b) {
  uint64_t x = *(uint64_t *)a;
  uint64_t y = *(uint64_t *)b;

  if (x < y)
    return -1;
  if (x > y)
    return 1;
  return 0;
}

/**
 * Esegue la serie di misure della latenza di accesso alla memoria.
 *
 * Per ciascun valore di ITERS vengono effettuate TRIES misure dei pattern
 * collegato e casuale. I tempi mediani ottenuti vengono stampati, formato CSV:
 *
 *     ITERS, link_time, random_time
 *
 * @return EXIT_SUCCESS.
 */
int main() {
  pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);

  // esegui passaggi da MIN_ITERS a MAX_ITERS
  for (int ITERS = MIN_ITERS; ITERS <= MAX_ITERS; ITERS += ITERS_STEP) {
    // inizializza contatori
    uint64_t link_times[TRIES];
    uint64_t rand_times[TRIES];
    uint64_t sarv_times[TRIES];

    // esegui TRIES test
    for (int j = 0; j < TRIES; j++) {
      measure(ITERS, &link_times[j], &rand_times[j], &sarv_times[j]);
    }

	// ordina i valori 
    qsort(link_times, TRIES, sizeof(uint64_t), compare);
    qsort(rand_times, TRIES, sizeof(uint64_t), compare);
    qsort(sarv_times, TRIES, sizeof(uint64_t), compare);

	// calcola la mediana
    uint64_t link_median = link_times[TRIES / 2];
    uint64_t rand_median = rand_times[TRIES / 2];
    uint64_t sarv_median = sarv_times[TRIES / 2];

    // stampa in formato CSV
    printf("%d, %llu, %llu, %llu\n", ITERS, link_median, rand_median,
           sarv_median);
  }
}
