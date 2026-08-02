#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * Restituisce il tempo corrente in nanosecondi. Si appoggia alla
 * mach_absolute_time per ottenere tempo indipendente dal wall clock.
 *
 * @return Tempo corrente in nanosecondi.
 */
uint64_t get_time();

/**
 * Calcola la mediana di un array di valori di dimensione nota.
 *
 * @param arr Array di valori.
 * @param n Dimensione dell'array.
 *
 * @return Mediana dell'array.
 */
double median(uint64_t arr[], int n);

/**
 * Restituisce un numero casuale in un range min, max.
 *
 * @param min Minimo del range.
 * @param max Massimo del range.
 *
 * @return Numero casuale fra min e max.
 */
int rand_range(int min, int max);

/**
 * Compara due numeri, restituendo il minore.
 *
 * @param a Il primo numero.
 * @param b Il secondo numero.
 *
 * @return Il minimo fra a e b.
 */
int min(int a, int b);

/**
 * Comincia una sessione di profilazione.
 *
 */
void beg_pmu();

/**
 * Termina una sessione di profilazione.
 *
 * @return Valore di ritorno per i cicli di clock.
 */
uint64_t end_pmu();

#endif
