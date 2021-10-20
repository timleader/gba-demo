
#ifndef STRING_H
#define STRING_H

#include "common/types.h"

int32_t string_compare(const char* s1, const char* s2);

uint32_t string_length(const char* s);

int32_t string_index_of(const char* s, char c);

int32_t string_index_of_any(const char* str, const char* c, uint16_t c_count);

void string_wrap(const char* str, char* output, uint8_t width);

int32_t string_character_count(const char* str, char c);

#endif
