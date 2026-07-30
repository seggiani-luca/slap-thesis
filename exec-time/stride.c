#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mach/mach_time.h>

// uint64_t get_time() {
//     struct timespec ts; 
//     clock_gettime(CLOCK_MONOTONIC, &ts);
//     return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec; 
// }

uint64_t get_time() {
    static mach_timebase_info_data_t info = {0};

    if (info.denom == 0)
        mach_timebase_info(&info);

    uint64_t t = mach_absolute_time();

    return t * info.numer / info.denom;
}

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

