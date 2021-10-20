
#ifndef SEQUENCE_H
#define SEQUENCE_H


#include "games/7days/dialogue.h"
#include "games/7days/inventory.h"

#include "common/types.h"
#include "common/utils/timer.h"

/*typedef struct sequence_event_s		//	 these are addressable by 'seq_id'
{
	int16_t next;			//	next sequence event		//	this is relative and can now be int8_t       
	uint16_t type;			//	this is bigger than needed only needs to be uint8_t		uint5_t  5^2

	uint32_t extra_data;	//	 --can this be a fixed size ? ?		uint22_t 

	//	I think we could pack all of this into one uint32_t 

	0 -> 4  => next
	5 -> 9 => type
	10 -> 31 => extra

} sequence_event_t;*/

//-----------------------------------------------------------------------------
typedef uint32_t sequence_event_t;


//-----------------------------------------------------------------------------
#define SEQUENCE_EVENT_NEXT(x) ((x & 0x00000010) ? ((int32_t)((x & 0x0000000F) | 0xFFFFFFF0)) : (int32_t)(x & 0x0000000F))
//#define SEQUENCE_EVENT_NEXT(x) ((x & 0x00000010) ? (-(int32_t)~(x & 0x0000000F)) : (int32_t)(x & 0x0000000F))
#define SEQUENCE_EVENT_TYPE(x) ((x >> 5) & 0x0000001F)
#define SEQUENCE_EVENT_EXTRA(x) (x >> 10)

//-----------------------------------------------------------------------------
typedef enum sequence_completion_mode_e
{
	SEQUENCE_COMPLETION_IMMEDIATE,
	SEQUENCE_COMPLETION_NEXT_TICK,
	SEQUENCE_COMPLETION_WAIT_ON_FLAG,

} sequence_completion_mode_t;

//-----------------------------------------------------------------------------
typedef enum sequence_player_state_e
{
	PLAYER_STATE_IDLE, 
	PLAYER_STATE_PLAYING,
	PLAYER_STATE_WAITING,

} sequence_player_state_t;

//-----------------------------------------------------------------------------
struct sequence_player_s;
typedef struct sequence_player_s sequence_player_t;
typedef sequence_player_t* sequence_player_ptr;

typedef sequence_completion_mode_t(*sequence_event_handler_t)(sequence_player_ptr player, sequence_event_t event);

//-----------------------------------------------------------------------------
typedef struct sequence_store_s
{
	uint16_t event_count;
	uint16_t reserved;

	sequence_event_t events[0];

} sequence_store_t;

typedef sequence_store_t* sequence_store_ptr;

//-----------------------------------------------------------------------------
typedef struct sequence_channel_persistent_s
{
	int16_t scheduled_event;
	int16_t current_event;

} sequence_channel_persistent_t;

//-----------------------------------------------------------------------------
typedef struct sequence_channel_ephermeral_s
{
	sequence_player_state_t state;					
	int8_t* completion_flag;						

} sequence_channel_ephermeral_t;

//-----------------------------------------------------------------------------
typedef struct sequence_player_s
{
	uint32_t* persistent_state;							//	PERSISTENT 
	sequence_channel_persistent_t* persistent_channels;	//	PERSISTENT
	
	sequence_channel_ephermeral_t* ephermeral_channels;	//	EPHERMERAL
 
	sequence_store_ptr store;							//	CONSTANT
	sequence_event_handler_t* event_handlers;			//	CONSTANT
	uint16_t channel_count;								//	CONSTANT
	void_ptr context;									//	CONSTANT

} sequence_player_t;


//	=============================
//
//	push a state to handle the cutscene sequence and block play state input... 
//
//	=============================


//-----------------------------------------------------------------------------
void sequence_reset(sequence_player_ptr player);

//-----------------------------------------------------------------------------
void sequence_schedule(sequence_player_ptr player, int16_t sequence_id);

//-----------------------------------------------------------------------------
void sequence_update(sequence_player_ptr player);		//	this runs only in st_play / world update


#endif
