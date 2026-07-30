#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mach/mach_time.h>

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
uint64_t stride(int* in, size_t S, int ITERS) {
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
 * @param argc Numero di argomenti passati da linea di comando.
 * @param argv Argomenti:
 *             argv[1] = stride S,
 *             argv[2] = numero di iterazioni ITERS.
 *
 * @return 0 se l'esecuzione termina correttamente, 1 in caso di errore.
 */
int main(int argc, char* argv[]) {
	// ottieni input, ./stride [S] [ITERS]
	if(argc < 3) {
		printf("Troppi pochi argomenti\n");
		return 1;
	}

	size_t S = atoi(argv[1]);
	int ITERS = atoi(argv[2]);

	int array[S * ITERS];

	// inizializza array collegato
	for(int i = 0; i < ITERS; i++){
		array[i * S] = (i + 1) * S;
	}

	// misura array collegato 
	uint64_t link_time = stride(array, S, ITERS);

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
	uint64_t rand_time = stride(array, S, ITERS);

	// stampa risultati
	printf("%llu, %llu\n", link_time, rand_time);

	return 0;
}

