
#ifndef OBJ_ATTR_H
#define OBJ_ATTR_H

#include "common/math/point.h"


// --- OAM attribute 0 -------------------------------------------------

#define ATTR0_REG				 0	//!< Regular object
#define ATTR0_AFF			0x0100	//!< Affine object
#define ATTR0_HIDE			0x0200	//!< Inactive object
#define ATTR0_AFF_DBL		0x0300	//!< Double-size affine object
#define ATTR0_AFF_DBL_BIT	0x0200
#define ATTR0_BLEND			0x0400	//!< Enable blend
#define ATTR0_WINDOW		0x0800	//!< Use for object window
#define ATTR0_MOSAIC		0x1000	//!< Enable mosaic
#define ATTR0_4BPP				 0	//!< Use 4bpp (16 color) tiles
#define ATTR0_8BPP			0x2000	//!< Use 8bpp (256 color) tiles
#define ATTR0_SQUARE			 0	//!< Square shape
#define ATTR0_WIDE			0x4000	//!< Tall shape (height &gt; width)
#define ATTR0_TALL			0x8000	//!< Wide shape (height &lt; width)

#define ATTR0_Y_MASK		0x00FF
#define ATTR0_Y_SHIFT			 0
#define ATTR0_Y(n)		((n)<<ATTR0_Y_SHIFT)

#define ATTR0_MODE_MASK		0x0300
#define ATTR0_MODE_SHIFT		 8
#define ATTR0_MODE(n)		((n)<<ATTR0_MODE_SHIFT)

#define ATTR0_SHAPE_MASK	0xC000
#define ATTR0_SHAPE_SHIFT		14
#define ATTR0_SHAPE(n)	((n)<<ATTR0_SHAPE_SHIFT)


#define ATTR0_BUILD(y, shape, bpp, mode, mos, bld, win)				\
(																	\
	((y)&255) | (((mode)&3)<<8) | (((bld)&1)<<10) | (((win)&1)<<11) \
	| (((mos)&1)<<12) | (((bpp)&8)<<10)| (((shape)&3)<<14)			\
)


// --- OAM attribute 1 -------------------------------------------------

#define ATTR1_HFLIP			0x1000	//!< Horizontal flip (reg obj only)
#define ATTR1_VFLIP			0x2000	//!< Vertical flip (reg obj only)
// Base sizes
#define ATTR1_SIZE_8			 0
#define ATTR1_SIZE_16		0x4000
#define ATTR1_SIZE_32		0x8000
#define ATTR1_SIZE_64		0xC000
// Square sizes
#define ATTR1_SIZE_8x8			 0	//!< Size flag for  8x8 px object
#define ATTR1_SIZE_16x16	0x4000	//!< Size flag for 16x16 px object
#define ATTR1_SIZE_32x32	0x8000	//!< Size flag for 32x32 px object
#define ATTR1_SIZE_64x64	0xC000	//!< Size flag for 64x64 px object
// Tall sizes
#define ATTR1_SIZE_8x16			 0	//!< Size flag for  8x16 px object
#define ATTR1_SIZE_8x32		0x4000	//!< Size flag for  8x32 px object
#define ATTR1_SIZE_16x32	0x8000	//!< Size flag for 16x32 px object
#define ATTR1_SIZE_32x64	0xC000	//!< Size flag for 32x64 px object
// Wide sizes
#define ATTR1_SIZE_16x8			 0	//!< Size flag for 16x8 px object
#define ATTR1_SIZE_32x8		0x4000	//!< Size flag for 32x8 px object
#define ATTR1_SIZE_32x16	0x8000	//!< Size flag for 32x16 px object
#define ATTR1_SIZE_64x32	0xC000	//!< Size flag for 64x64 px object


#define ATTR1_X_MASK		0x01FF
#define ATTR1_X_SHIFT			 0
#define ATTR1_X(n)			((n)<<ATTR1_X_SHIFT)

#define ATTR1_AFF_ID_MASK		0x3E00
#define ATTR1_AFF_ID_SHIFT		 9
#define ATTR1_AFF_ID(n)		((n)<<ATTR1_AFF_ID_SHIFT)

#define ATTR1_FLIP_MASK		0x3000
#define ATTR1_FLIP_SHIFT		12
#define ATTR1_FLIP(n)		((n)<<ATTR1_FLIP_SHIFT)

#define ATTR1_SIZE_MASK		0xC000
#define ATTR1_SIZE_SHIFT		14
#define ATTR1_SIZE(n)		((n)<<ATTR1_SIZE_SHIFT)


#define ATTR1_BUILDR(x, size, hflip, vflip)	\
( ((x)&511) | (((hflip)&1)<<12) | (((vflip)&1)<<13) | (((size)&3)<<14) )

#define ATTR1_BUILDA(x, size, affid)			\
( ((x)&511) | (((affid)&31)<<9) | (((size)&3)<<14) )


// --- OAM attribute 2 -------------------------------------------------

#define ATTR2_ID_MASK		0x03FF
#define ATTR2_ID_SHIFT			 0
#define ATTR2_ID(n)			((n)<<ATTR2_ID_SHIFT)

#define ATTR2_PRIO_MASK		0x0C00
#define ATTR2_PRIO_SHIFT		10
#define ATTR2_PRIO(n)		((n)<<ATTR2_PRIO_SHIFT)

#define ATTR2_PALBANK_MASK	0xF000
#define ATTR2_PALBANK_SHIFT		12
#define ATTR2_PALBANK(n)	((n)<<ATTR2_PALBANK_SHIFT)


#define ATTR2_BUILD(id, pbank, prio)			\
( ((id)&0x3FF) | (((pbank)&15)<<12) | (((prio)&3)<<10) )

#define BIT_SHIFT(a, n)			( (a)<<(n) )
#define BIT_SET(word, flag)		( word |=  (flag) )
#define BIT_CLEAR(word, flag)	( word &= ~(flag) )
#define BIT_FLIP(word, flag)	( word ^=  (flag) )
#define BIT_EQ(word, flag)		( ((word)&(flag)) == (flag) )

#define BFN_PREP(x, name)		( ((x)<<name##_SHIFT) & name##_MASK )
#define BFN_GET(y, name)			( ((y) & name##_MASK)>>name##_SHIFT )
#define BFN_SET(y, x, name)		(y = ((y)&~name##_MASK) | BFN_PREP(x,name) )

// x already shifted
#define BFN_PREP2(x, name)		( (x) & name##_MASK )
#define BFN_GET2(y, name)		( (y) & name##_MASK )
#define BFN_SET2(y, x, name)		(y = ((y)&~name##_MASK) | BFN_PREP2(x,name) )

#if defined(__GBA__)
#define ALIGN(m)	__attribute__((aligned (m)))
#define ALIGN4     __attribute__((aligned(4)))
#else
#define ALIGN(m)	
#define ALIGN4   
#endif

typedef struct obj_attr_s
{
	uint16_t attr0;
	uint16_t attr1;
	uint16_t attr2;
	int16_t fill;
} ALIGN(4) obj_attr_t;

#define SPRITE_COUNT		128
#define SPRITE_WIDTH		32
#define SPRITE_HEIGHT		8

#define TILE_SIZE_IN_BYTES	32
#define TILES_PER_SPRITE	4			//	this should be * 8 due to ATTR1_SIZE_32x8 (4) and 8BPP (2) --- 	

#define INLINE inline

//! Set the attributes of an object.
INLINE obj_attr_t* obj_set_attr(obj_attr_t* obj, uint16_t a0, uint16_t a1, uint16_t a2)
{
	obj->attr0 = a0; obj->attr1 = a1; obj->attr2 = a2;
	return obj;
}

//! Set the position of \a obj
INLINE void obj_set_pos(obj_attr_t* obj, int x, int y)
{
	BFN_SET(obj->attr0, y, ATTR0_Y);
	BFN_SET(obj->attr1, x, ATTR1_X);
}

//! Hide an object.
INLINE void obj_hide(obj_attr_t* obj)
{
	BFN_SET2(obj->attr0, ATTR0_HIDE, ATTR0_MODE);
}

//! Unhide an object.
/*! \param obj	Object to unhide.
*	\param mode	Object mode to unhide to. Necessary because this affects
*	  the affine-ness of the object.
*/
INLINE void obj_unhide(obj_attr_t* obj, uint16_t mode)
{
	BFN_SET2(obj->attr0, mode, ATTR0_MODE);
}

#endif // !OBJ_ATTR_H