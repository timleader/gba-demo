
#include "string.h"

int32_t string_compare(const char* s1, const char* s2) 
{
    unsigned char c1, c2;
    while ((c1 = *s1++) == (c2 = *s2++)) {
        if (c1 == '\0')
            return 0;
    }
    return c1 - c2;
} 

uint32_t string_length(const char* str)
{
    const char* s;
    for (s = str; *s; ++s) ;
    return (s - str);
}

int32_t string_index_of(const char* str, char c)
{
    const char* s;
    for (s = str; *s && *s != c; ++s);

    if (*s == c)
        return (s - str);

    return -1;
}


int32_t string_index_of_any(const char* str, const char* c, uint16_t c_count)
{
	int16_t len = 0;

	for (;;)
	{
		for (uint16_t i = 0; i < c_count; ++i)
		{
			if (*str == c[i])
				goto end;
		}

		len++;
		str++;
	}

	end:

	return len;
}

void string_wrap(const char* str, char* output, uint8_t width)
{
	const char delimiter_characters[] = { ' ', '\n', 0 };
	const uint16_t delimiter_character_count = 3;

	int16_t line_pos = 0;
	int16_t len = 0;
	char* write_buf = output;

	for (;;)
	{
		len = string_index_of_any(str, delimiter_characters, delimiter_character_count);

		if (line_pos + len > width &&
			len < width)
		{
			*(write_buf - 1) = '\n';
			line_pos = 0;
		}

		line_pos += len;
		while (len--)
		{
			*write_buf++ = *str++;
		}

		if (*str == 0)
			break;

		line_pos++;
		*write_buf++ = *str++;
	}

	*write_buf = 0;
}

int32_t string_character_count(const char* str, char c)
{
	const char* s;
	int32_t count = 0;
	for (s = str; *s; ++s)
	{
		if (*s == c)
			count++;
	}
	return count;
}
