
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef volatile uint8_t vuint8_t;
typedef volatile uint16_t vuint16_t;
typedef volatile uint32_t vuint32_t;

typedef volatile int8_t vint8_t;
typedef volatile int16_t vint16_t;
typedef volatile int32_t vint32_t;

typedef uint8_t bool8_t;
typedef uint16_t bool16_t;

typedef void* void_ptr;

#define TRUE  1
#define FALSE 0

#ifndef NULL
#define NULL 0
#endif


/*! \def BIT(number)

	\brief Macro for bit settings.

	\param number bit number to set
*/
#define BIT(number) (1<<(number))


#endif // !TYPES_H
