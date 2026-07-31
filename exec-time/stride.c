#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <sys/qos.h>
#include "utils.h"

// Valore minimo di ITERS.
#define MIN_ITERS 10

// Valore massimo di ITERS.
#define MAX_ITERS 10000

// Numero di passi tra MIN_ITERS e MAX_ITERS.
#define ITERS_STEPS 15

// Valore di S.
#define S 16

// Numero di misure eseguite per ciascun valore di ITERS.
#define TRIES 10000

/**
 * Restituisce il tempo corrente in nanosecondi. Si appoggia alla
 * mach_absolute_time per ottenere tempo indipendente dal wall clock.
 *
 * @return Tempo corrente in nanosecondi.
 */
uint64_t get_time() {
    // ottieni info da mach
    static mach_timebase_info_data_t info = {0};
    if(info.denom == 0) mach_timebase_info(&info);

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
uint64_t stride(int* in, int ITERS) {
    // array e dep volatili per evitare ottimizzazioni
    volatile int* array = in;
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

uint64_t sarv(int* in, int ITERS) {
    // array e dep volatili per evitare ottimizzazioni
    volatile int* array = in;
    volatile int dep;

    // effettua la dry run
    dep = 0;
	for (int i = 0; i < ITERS; ++i){
		dep += min(array[dep], S);
	}

    // avvia il timer
    uint64_t start = get_time();

    // effettua la wet run
    dep = 0;
	for (int i = 0; i < ITERS; ++i){
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
void measure(int ITERS, int* link_time, int* rand_time, int* sarv_time) {
    // usa un unico array
    int array[S * ITERS];

    // inizializza array collegato
    for(int i = 0; i < ITERS - 1 ; i++){
        array[i * S] = (i + 1) * S;
    }
	array[(ITERS - 1) * S] = 0;

    // misura array collegato 
    *link_time += stride(array, ITERS);

    // inizializza array casuale, permutando per evitare cicli
    int *perm = malloc(ITERS * sizeof(int));
	if (perm == NULL)
    	return;
    for(int i = 0; i < ITERS; i++) {
        perm[i] = i;
    }    
	for(int i = ITERS - 1; i > 1; i--) {
		int j = 1 + rand() % i;
		int t = perm[i];
		perm[i] = perm[j];
		perm[j] = t;
	}
	for(int i = 0; i < ITERS - 1; i++) {
		array[perm[i] * S] = perm[i + 1] * S;
	}
    array[perm[ITERS - 1] * S] = perm[0] * S;

    // misura array casuale 
    *rand_time += stride(array, ITERS);

	for(int i = 0; i < ITERS; i++) {
		array[i * S] = (perm[i] + 1) * S;
	}
	free(perm);

	// misura array sa+rv
	*sarv_time += sarv(array, ITERS);

}

/**
 * Esegue la serie di misure della latenza di accesso alla memoria.
 *
 * Per ciascun valore di ITERS vengono effettuate TRIES misure dei pattern
 * collegato e casuale. I tempi medi ottenuti vengono stampati in formato CSV:
 *
 *     ITERS, link_time, random_time
 *
 * @return EXIT_SUCCESS.
 */


int main() {
	pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);

    // fai ITER_STEPS passi 
    for(int i = 0; i < ITERS_STEPS; i++) {
        int ITERS = MIN_ITERS + ((MAX_ITERS - MIN_ITERS) / (ITERS_STEPS - 1)) * i;

        // inizializza contatori
        int link_time, rand_time, sarv_time;
        link_time = rand_time = sarv_time = 0;

        // esegui TRIES test
        for(int j = 0; j < TRIES; j++) {
            measure(ITERS, &link_time, &rand_time, &sarv_time);
        }

        // calcola valor medio
        link_time /= TRIES;
        rand_time /= TRIES;
		sarv_time /= TRIES;
	

        // stampa in formato CSV
        printf("%d, %d, %d, %d\n", ITERS, link_time, rand_time, sarv_time);
    }
}
