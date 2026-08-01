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
 * Comincia una sessione di profilazione.
 *
 * @return Valore di ritorno.
 */
int beg_pmu();

/**
 * Termina una sessione di profilazione.
 *
 * @param cycles Valore di ritorno per i cicli di clock.
 *
 * @return Valore di ritorno.
 */
int end_pmu(uint64_t *cycles);

#endif
