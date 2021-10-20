
#ifndef LIST_H
#define LIST_H

#include "common/types.h"


//-----------------------------------------------------------------------------
typedef struct int16_list_s 
{
	uint16_t length;
	uint16_t capacity;

	int16_t* data;	

} int16_list_t;

typedef int16_list_t* int16_list_ptr;

//	remove the ability to resize !!! ?? should be working under known memory conditions !! 
//		fixed size list would be good 

//-----------------------------------------------------------------------------
void list_new(int16_list_ptr list, uint16_t initial_capacity, uint8_t memory_sector);

//-----------------------------------------------------------------------------
void list_delete(int16_list_ptr list);

//-----------------------------------------------------------------------------
int16_t list_append(int16_list_ptr list, int16_t value);

//-----------------------------------------------------------------------------
int16_t list_resize(int16_list_ptr list, uint16_t new_size);

//-----------------------------------------------------------------------------
int16_t list_insert(int16_list_ptr list, int16_t value, uint16_t index);

//-----------------------------------------------------------------------------
void list_remove_at_index(int16_list_ptr list, uint16_t index);

//-----------------------------------------------------------------------------
int16_t list_index_of(int16_list_ptr list, int16_t value);

//-----------------------------------------------------------------------------
void list_reverse(int16_list_ptr list);


#endif
