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

#endif
