
#ifndef TRIGONOMETRY_H
#define TRIGONOMETRY_H

#include "fixed16.h"

fixed16_t fixed16_sine(fixed16_t x);
fixed16_t fixed16_cosine(fixed16_t x);
fixed16_t fixed16_tangent(fixed16_t x);

fixed16_t fixed16_arcsine(fixed16_t x);
fixed16_t fixed16_arccosine(fixed16_t x);

fixed16_t fixed16_arctangent2(fixed16_t y, fixed16_t x);

#endif
