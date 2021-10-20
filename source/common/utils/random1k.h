
#ifndef RANDOM_1K_H
#define RANDOM_1K_H

#include "common/types.h"


void random_seed (uint16_t seed);

// 1K random numbers in the range 0-255
uint8_t random_u8 (void);


#endif
