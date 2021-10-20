
#include "common/graphics/image.h"

#include "common/memory.h"
#include "common/debug/debug.h"


palette_ptr palette_new_from_copy(palette_ptr pal, uint8_t memory_sector)
{
	uint16_t pal_size = pal->size;

	palette_ptr pal_new = (palette_ptr)memory_allocate(sizeof(palette_t) + (pal_size * sizeof(uint16_t)), memory_sector);

	pal_new->id = ~0;
	pal_new->offset = pal->offset;
	pal_new->size = pal_size;

	memory_copy(pal_new->color555, pal->color555, (pal_size << 1));

	return pal_new;
}

palette_ptr palette_new(uint16_t pal_size, uint8_t memory_sector)
{
	palette_ptr pal = (palette_ptr)memory_allocate(sizeof(palette_t) + (pal_size * sizeof(uint16_t)), memory_sector);

	pal->id = ~0;
	pal->offset = 0;
	pal->size = pal_size;

	return pal;
}

void palette_delete(palette_ptr pal)
{
	debug_assert(pal != NULL, "palette_delete, pal is null");

	memory_free(pal);
	pal = NULL;
}

void palette_lerp(palette_ptr original_pal, palette_ptr destination_pal, color555_t target_color, fixed16_t t)
{
	debug_assert(original_pal != NULL && destination_pal != NULL, "palette_lerp, original_pal or destination_pal is null");
	debug_assert(original_pal->offset == destination_pal->offset, "palette_lerp, original_pal and destination_pal offset values are different");
	debug_assert(original_pal->size == destination_pal->size, "palette_lerp, original_pal and destination_pal size values are different");

	fixed16_t target_r = fixed16_from_int((target_color >> 0) & 0x1F);
	fixed16_t target_g = fixed16_from_int((target_color >> 5) & 0x1F);
	fixed16_t target_b = fixed16_from_int((target_color >> 10) & 0x1F);

	uint16_t idx = 0;
	uint16_t color;
	uint8_t r, g, b;

	while (idx < destination_pal->size) // I'm sure there is a faster way to do this !! //	this is corrupting the heap
	{
		color = original_pal->color555[idx];

		r = (color >> 0) & 0x1F;
		g = (color >> 5) & 0x1F;
		b = (color >> 10) & 0x1F;

		r = fixed16_to_int(fixed16_lerp(fixed16_from_int(r), target_r, t));
		g = fixed16_to_int(fixed16_lerp(fixed16_from_int(g), target_g, t));
		b = fixed16_to_int(fixed16_lerp(fixed16_from_int(b), target_b, t));

		color =
			(r << 0) |
			(g << 5) |
			(b << 10);

		destination_pal->color555[idx] = color;

		++idx;
	}
}

uint32_t palette_size_in_bytes(palette_ptr palette)
{
	debug_assert(palette != NULL, "palette_size_in_bytes, pal is null");

	return (uint32_t)(sizeof(palette_t) + (palette->size << 1));
}

color555_t color555_lerp(color555_t source_color, color555_t target_color, fixed16_t t)
{
	//	my god, optimize this 

	color555_t color = 0;

	fixed16_t source_r = fixed16_from_int((source_color >> 0) & 0x1F);
	fixed16_t source_g = fixed16_from_int((source_color >> 5) & 0x1F);
	fixed16_t source_b = fixed16_from_int((source_color >> 10) & 0x1F);

	fixed16_t target_r = fixed16_from_int((target_color >> 0) & 0x1F);
	fixed16_t target_g = fixed16_from_int((target_color >> 5) & 0x1F);
	fixed16_t target_b = fixed16_from_int((target_color >> 10) & 0x1F);

	uint8_t r = fixed16_to_int(fixed16_lerp(source_r, target_r, t));
	uint8_t g = fixed16_to_int(fixed16_lerp(source_g, target_g, t));
	uint8_t b = fixed16_to_int(fixed16_lerp(source_b, target_b, t));

	color =
		(r << 0) |
		(g << 5) |
		(b << 10);

	return color;
}


image_ptr image_new(uint8_t width, uint8_t height, uint8_t memory_sector)
{
	image_ptr image = (image_ptr)memory_allocate(sizeof(image_t) + (width * height), memory_sector);
	image->id = ~0;
	image->palette_id = ~0;
	image->width = width;
	image->height = height;
	return image;
}

void image_delete(image_ptr image)
{
	debug_assert(image != NULL, "image_delete, image is null");

	memory_free(image);
	image = NULL;
}

uint32_t image_size_in_bytes(image_ptr image)
{
	debug_assert(image != NULL, "image_size_in_bytes, image is null");

	return (uint32_t)(sizeof(image_t) + (image->width * image->height));
}


tiledimage_ptr tiledimage_new(uint8_t tile_width, uint8_t tile_height, uint8_t memory_sector)
{
	tiledimage_ptr image = (tiledimage_ptr)memory_allocate(sizeof(tiledimage_t) + (sizeof(tile_t) * tile_width * tile_height), memory_sector);
	image->id = ~0;
	image->palette_id = ~0;
	image->tile_width = tile_width;
	image->tile_height = tile_height;
	return image;
}

void tiledimage_delete(tiledimage_ptr image)
{
	debug_assert(image != NULL, "tiledimage_delete, tiledimage is null");

	memory_free(image);
	image = NULL;
}

uint32_t tiledimage_size_in_bytes(tiledimage_ptr image)
{
	debug_assert(image != NULL, "tiledimage_size_in_bytes, tiledimage is null");

	return (uint32_t)(sizeof(tiledimage_t) + (sizeof(tile_t) * image->tile_width * image->tile_height));
}
