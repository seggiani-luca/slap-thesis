#include "../utils/utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/qos.h>

// Valore minimo di ITERS
#define MIN_ITERS 10

// Valore massimo di ITERS
#define MAX_ITERS 1000

// Passo nello scorrimento fra MIN_ITERS e MAX_ITERS
#define ITERS_STEP 10

// Byte di stride
#define STRIDE_BYTES 32

// Valore di S
#define S (STRIDE_BYTES / sizeof(int))

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

  // avvia la profilazione
  if (beg_pmu()) {
    printf("Errore avvio PMU\n");
  }

  // effettua la wet run
  dep = 0;
  for (int i = 0; i < ITERS; ++i) {
    dep = array[dep];
  }

  // termina la profilazione
  uint64_t ret;
  if (!end_pmu(&ret)) {
    printf("Errore termine PMU\n");
  }

  // restituisci tempo impiegato dalla wet run
  return ret;
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

  // avvia la profilazione
  if (beg_pmu()) {
    printf("Errore avvio PMU\n");
  }

  // effettua la wet run
  dep = 0;
  for (int i = 0; i < ITERS; ++i) {
    dep += min(array[dep], S);
  }

  // termina la profilazione
  uint64_t ret;
  if (end_pmu(&ret)) {
    printf("Errore termine PMU\n");
  }

  // restituisci tempo impiegato dalla wet run
  return ret;
}

/**
 * Misura la latenza degli accessi in memoria in due diverse configurazioni di
 * accesso a un array:
 * - Array striding: ogni elemento punta al successivo con uno stride fisso;
 * - Array casuale: ottenuto tramite una permutazione casuale. Si permuta per
 *   evitare cicli nei collegamenti (0 -> 1, 1 -> 0, invalidano i risultati).
 *
 * @param ITERS Numero di iterazioni della sequenza di accessi.
 * @param link_time puntatore al ritorno del tempo per l'array striding
 * @param rand_time puntatore al ritorno del tempo per l'array casuale
 */
void measure(int ITERS, uint64_t *link_time, uint64_t *rand_time,
             uint64_t *sarv_time) {
  // usa un unico array
  int array[S * ITERS];

  // inizializza array striding
  for (int i = 0; i < ITERS - 1; i++) {
    array[i * S] = (i + 1) * S;
  }

  // misura array striding
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

  // inizializza array SA + RV con multipli casuali di S
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
 * striding, casuale e sa+rv. I tempi mediani ottenuti vengono stampati,
 * formato CSV:
 *
 *     ITERS, link_median, random_median, sarv_median
 *
 * @return EXIT_SUCCESS.
 */
int main() {
  // classe di QOS per P-core
  pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);

  // esegui passaggi da MIN_ITERS a MAX_ITERS
  for (int ITERS = MIN_ITERS; ITERS <= MAX_ITERS; ITERS += ITERS_STEP) {
    uint64_t link_time, rand_time, sarv_time;
    measure(ITERS, &link_time, &rand_time, &sarv_time);

    // stampa in formato CSV
    printf("Iterazioni, Striding, Casuale, SA + RV");
    printf("%d, %lu, %lu, %lu\n", ITERS, link_time, rand_time, sarv_time);
  }
}
