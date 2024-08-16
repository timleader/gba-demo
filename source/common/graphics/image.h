
#ifndef IMAGE_H
#define IMAGE_H


#include "common/types.h"
#include "common/math/fixed16.h"

/**
	Setup assetcompiler to be able to create these




	--------------------------------------

	objects:

		palette
			:palette id 
			:size
			:data

		image 
			palette id 
			data
			size

*/

//	whats the diff between sprite and texture ??? 

typedef uint16_t color555_t;

typedef struct palette_s	//	maybe bake both light and dark palettes into one palette 
{
	uint8_t id;
	uint8_t offset;			//	this is destination offset, offset at which this should be copied into hardware
	uint16_t reserved;

	uint16_t size;			//	size of palette, this only needs to be a uint8_t 
	uint16_t reserved2;

	//	should store the slice idx between light and dark palette 

	//	should store size of 2nd palette 

	color555_t color555[0];

} palette_t;

typedef palette_t* palette_ptr;


typedef struct image_s		
{
	/**
		static int invertedbitwiseWidth = 11;
		static int invertedbitwiseHeight = 11;
		static int bitwiseWidth = 5;
		static int bitwiseHeight = 5;	//eg . 4 => 16
		static int bitMaskWidth = 0x1F;
		static int bitMaskHeight = 0x1F;
	 */

	uint8_t id;
	uint8_t format;
	uint8_t width;		//	consider bitwise width	//	reduce to int8
	uint8_t height;

	int16_t palette_id;		//	+1 for backlit ?? 
	uint16_t reserved;		//	backlit vs non-backlit palette 

	uint16_t frame_count;		//	can be used to treat this as an image_array 
	uint16_t size;

	uint8_t data[0];	

} image_t;

typedef image_t* image_ptr;

typedef struct depthmap_s		//	 2 bits per pixel for 4 layers 
{
	uint8_t id;
	uint8_t width;
	uint8_t height;
	uint8_t reserved;

	uint16_t reserved2;
	uint16_t size;

	uint8_t data[0];

} depthmap_t;

typedef depthmap_t* depthmap_ptr;

typedef struct tile_s		//	8x8  4BBP 
{
	uint32_t data[8];

} tile_t;		

typedef tile_t* tile_ptr;

typedef struct tiledimage_s		
{
	uint8_t id;
	uint8_t palette_id;
	uint8_t tile_width;		//	consider bitwise width	//	reduce to int8
	uint8_t tile_height;

	uint16_t frame_count;
	uint16_t size;

	tile_t tiles[0];

} tiledimage_t;

typedef tiledimage_t* tiledimage_ptr;


palette_ptr palette_new_from_copy(palette_ptr pal, uint8_t memory_sector);

palette_ptr palette_new(uint16_t pal_size, uint8_t memory_sector);

void palette_delete(palette_ptr pal);

uint32_t palette_size_in_bytes(palette_ptr palette);

void palette_lerp(palette_ptr original_pal, palette_ptr destination_pal, color555_t target_color, fixed16_t t);

color555_t color555_lerp(color555_t source_color, color555_t target_color, fixed16_t t);



image_ptr image_new(uint8_t width, uint8_t height, uint8_t memory_sector);

void image_delete(image_ptr image);

uint32_t image_size_in_bytes(image_ptr image);



tiledimage_ptr tiledimage_new(uint8_t tile_width, uint8_t tile_height, uint8_t memory_sector);

void tiledimage_delete(tiledimage_ptr image);

uint32_t tiledimage_size_in_bytes(tiledimage_ptr image);


#endif // !IMAGE_H
