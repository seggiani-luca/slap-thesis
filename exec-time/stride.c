#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mach/mach_time.h>

// valori minimo e massimo di ITERS
#define MIN_ITERS 10
#define MAX_ITERS 10000

// passi intermedi di ITERS da minimo a massimo
#define ITERS_STEPS 10

// valore di S
#define S 32

// numero di test fatti ad ogni passo
#define TRIES 100

/**
 * La funzione converte i tick ottenuti in nanosecondi utilizzando il
 * rapporto di conversione fornito da mach_timebase_info().
 * @return Tempo corrente espresso in nanosecondi.
 */
uint64_t get_time() {
    static mach_timebase_info_data_t info = {0};

    if (info.denom == 0)
        mach_timebase_info(&info);

    uint64_t t = mach_absolute_time();

    return t * info.numer / info.denom;
}

/**
 * La funzione esegue due passate:
 * - una prima fase di "dry run" per portare gli indirizzi di memoria nella cache,
 *   riducendo l'effetto del prefetching hardware;
 * - una seconda fase di "wet run" durante la quale viene misurato il tempo
 *   necessario per completare gli accessi.
 *
 * Gli accessi sono RAW (Read After Write) dipendenti perché il valore letto
 * determina l'indirizzo del successivo accesso, impedendo l'esecuzione
 * parallela delle operazioni da parte della CPU.
 *
 * @param in Puntatore all'array contenente la catena di accessi.
 * @param S Dimensione dello stride utilizzato nella costruzione dell'array.
 * @param ITERS Numero di iterazioni della sequenza di accessi.
 *
 * @return Tempo impiegato dalla wet run.
 */
uint64_t stride(int* in, int ITERS) {
    volatile int* array = in;

    // dry run
    volatile int dep = 0;
    for (int i = 0; i < ITERS; ++i)
        dep = array[dep];

    // wet run
    dep = 0;
    uint64_t start = get_time();
    for (int i = 0; i < ITERS; ++i)
        dep = array[dep];
    uint64_t end = get_time();
    return end - start;
}

/**
 * Misura la latenza degli accessi in memoria.
 * Crea due diverse configurazioni di accesso a un array:
 * - una con collegamenti sequenziali (linked array), dove ogni elemento punta
 *   al successivo con uno stride fisso;
 * - una con collegamenti casuali, ottenuti tramite una permutazione casuale.
 * @param S Valore di stride.
 * @param ITERS Iterazioni dello stride.
 * @param link_time puntatore al ritorno del tempo per l'array collegato
 * @param rand_time puntatore al ritorno del tempo per l'array casuale 
 */
void measure(int ITERS, int* link_time, int* rand_time) {
	int array[S * ITERS];

	// inizializza array collegato
	for(int i = 0; i < ITERS; i++){
		array[i * S] = (i + 1) * S;
	}

	// misura array collegato 
	*link_time += stride(array, ITERS);

	// inizializza array casuale
	// permuta array per evitare cicli del tipo 0->1 1->0
	int *perm = malloc(ITERS * sizeof(int));
	for(int i = 0; i < ITERS; i++) {
		perm[i] = i;
	}
	
	for(int i = ITERS - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		int t = perm[i];
		perm[i] = perm[j];
		perm[j] = t;
	}
	for(int i = 0; i < ITERS; i++) {
		array[i * S] = perm[i] * S;
	}

	// misura array casuale 
	*rand_time += stride(array, ITERS);

}

int main() {
	// fai ITER_STEPS passi 
	for(int i = 0; i < ITERS_STEPS; i++) {
		int ITERS = MIN_ITERS + ((MAX_ITERS - MIN_ITERS) / ITERS_STEPS) * i;

		// inizializza contatori
		int link_time, rand_time;
		link_time = rand_time = 0;

		// esegui TRIES test
		for(int j = 0; j < TRIES; j++) {
			measure(ITERS, &link_time, &rand_time);
		}

		// calcola valor medio
		link_time /= TRIES;
		rand_time /= TRIES;

		// stampa
		printf("%d, %d, %d\n", ITERS, link_time, rand_time);
	}
}
