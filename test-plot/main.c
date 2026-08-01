#include <stdio.h>
#include <stdlib.h>

// numero di punti dati da stampare
#define NUM_POINTS 100

// stampa NUM_POINTS punti dati casuali, con header
int main() {
	printf("Data A, Data B, Data C\n");
	for(int i = 0; i < NUM_POINTS; i++) {
		printf("%d, %d, %d\n", rand() % 100, rand() % 100, rand() % 100);
	}

	return 0;
}
