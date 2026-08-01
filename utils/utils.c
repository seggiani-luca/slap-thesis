#include "utils.h"

#include <mach/mach_time.h>

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
