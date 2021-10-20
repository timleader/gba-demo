
// Define REGISTERs and SHIT

//	Aim to replace the use of libgba in devkitpro
//#include "gba_video.h"
//#include "gba_systemcalls.h"
//#include "gba_input.h"
//#include "gba_interrupt.h"

//	Also look at taking some of TONC

// Simplify some of the math even more..

#ifndef GBA_H
#define GBA_H

#include "common/types.h"

#ifdef  __GBA__ 

/*! \def VRAM

	\brief Base address of gba video ram.

*/
#define	VRAM		0x06000000
/*! \def IWRAM

	\brief Base address of gba internal work ram.

*/
#define	IWRAM		0x03000000
/*! \def EWRAM

	\brief Base address of gba external work ram.

*/
#define	EWRAM		0x02000000
#define	EWRAM_END	0x02040000
/*! \def SRAM

	\brief Base address of gba cart save ram.

*/
#define	SRAM		0x0E000000
/*! \def REG_BASE

	\brief Base address of gba hardware registers.

*/
#define	REG_BASE	0x04000000

#ifndef	NULL
#define	NULL	0
#endif

//---------------------------------------------------------------------------------
/*! \def SystemCall(Number)

	\brief helper macro to insert a bios call.
		\param Number swi number to call

		Inserts a swi of the correct format for arm or thumb code.

*/
#if	defined	( __thumb__ )
#define	SystemCall(Number)	 __asm ("SWI	  "#Number"\n" :::  "r0", "r1", "r2", "r3")
#else
#define	SystemCall(Number)	 __asm ("SWI	  "#Number"	<< 16\n" :::"r0", "r1", "r2", "r3")
#endif


#define IWRAM_CODE	__attribute__((section(".iwram"), long_call))
#define EWRAM_CODE	__attribute__((section(".ewram"), long_call))

#define IWRAM_DATA	__attribute__((section(".iwram")))
#define EWRAM_DATA	__attribute__((section(".ewram")))
#define EWRAM_BSS	__attribute__((section(".sbss")))

//---------------------------------------------------------------------------------
// modes for DMA and CPU(Fast)Set
//---------------------------------------------------------------------------------
enum DMA_MODES {
	FILL = (1 << 24),
	COPY16 = (0 << 26),
	COPY32 = (1 << 26)
};


/*! \def BG_COLORS
*/
#define BG_COLORS		((uint16_t *)0x05000000)	// Background color table
#define BG_PALETTE		((uint16_t *)0x05000000)	// Background color table
/*! \def OBJ_COLORS
*/
#define	OBJ_COLORS		((uint16_t *)0x05000200)	// Sprite color table
#define	SPRITE_PALETTE	((uint16_t *)0x05000200)	// Sprite color table

/*! \def REG_DISPCNT

	\brief LCD control register

	This register controls all aspects of the GBA display.
*/
#define	REG_DISPCNT		*((vuint16_t *)(REG_BASE + 0x00))

//!  LCDC bits.
/*!
  These bits are used in conjuction with REG_DISPCNT to control the GBA display hardware.
*/
//---------------------------------------------------------------------------------
typedef enum LCDC_BITS {
	//---------------------------------------------------------------------------------
	MODE_0 = 0,	/*!< BG Mode 0 */
	MODE_1 = 1,	/*!< BG Mode 1 */
	MODE_2 = 2,	/*!< BG Mode 2 */
	MODE_3 = 3,	/*!< BG Mode 3 */
	MODE_4 = 4,	/*!< BG Mode 4 */
	MODE_5 = 5,	/*!< BG Mode 5 */

	BACKBUFFER = BIT(4),		/*!< buffer display select			*/
	OBJ_1D_MAP = BIT(6),		/*!< sprite 1 dimensional mapping	*/
	LCDC_OFF = BIT(7),		/*!< LCDC OFF						*/
	BG0_ON = BIT(8),		/*!< enable background 0			*/
	BG1_ON = BIT(9),		/*!< enable background 1			*/
	BG2_ON = BIT(10),	/*!< enable background 2			*/
	BG3_ON = BIT(11),	/*!< enable background 3			*/
	OBJ_ON = BIT(12),	/*!< enable sprites					*/
	WIN0_ON = BIT(13),	/*!< enable window 0				*/
	WIN1_ON = BIT(14),	/*!< enable window 1				*/
	OBJ_WIN_ON = BIT(15),	/*!< enable obj window				*/

	BG0_ENABLE = BG0_ON,		/*!< enable background 0	*/
	BG1_ENABLE = BG1_ON, 	/*!< enable background 1	*/
	BG2_ENABLE = BG2_ON, 	/*!< enable background 2	*/
	BG3_ENABLE = BG3_ON,		/*!< enable background 3	*/
	OBJ_ENABLE = OBJ_ON, 	/*!< enable sprites			*/
	WIN0_ENABLE = WIN0_ON,	/*!< enable window 0		*/
	WIN1_ENABLE = WIN1_ON,	/*!< enable window 1		*/
	OBJ_WIN_ENABLE = OBJ_WIN_ON, /*!< enable obj window		*/

	BG_ALL_ON = BG0_ON | BG1_ON | BG2_ON | BG3_ON, 	    /*!< All Backgrounds on.		*/
	BG_ALL_ENABLE = BG0_ON | BG1_ON | BG2_ON | BG3_ON	    /*!< All Backgrounds active.	*/

} LCDC_BITS;

/*! \def REG_DISPSTAT

	\brief General LCD Status.

	This register controls the LCD interrupts.
*/
#define	REG_DISPSTAT	*((vuint16_t *)(REG_BASE + 0x04))

//---------------------------------------------------------------------------------
// LCDC Interrupt bits
//---------------------------------------------------------------------------------
enum LCDC_IRQ {
	LCDC_VBL_FLAG = (1 << 0),
	LCDC_HBL_FLAG = (1 << 1),
	LCDC_VCNT_FLAG = (1 << 2),
	LCDC_VBL = (1 << 3),
	LCDC_HBL = (1 << 4),
	LCDC_VCNT = (1 << 5)
};

static inline uint32_t VCOUNT(int m) { return m << 8; }


/*! \def REG_VCOUNT

	\brief

*/
#define	REG_VCOUNT		*((vuint16_t *)(REG_BASE + 0x06))

/*! \def BGCTRL

	\brief Array definition for background control registers.

	BGCTRL[0] references background 0 control register.<BR>
	BGCTRL[1] references background 1 control register.<BR>
	BGCTRL[2] references background 2 control register.<BR>
	BGCTRL[3] references background 3 control register.<BR>

*/
#define BGCTRL		((vuint16_t *)(REG_BASE + 0x08))
/*! \def REG_BG0CNT

	\brief Background 0 control register.

*/
#define REG_BG0CNT	*((vuint16_t *)(REG_BASE + 0x08))
/*! \def REG_BG1CNT

	\brief Background 1 control register.

*/
#define REG_BG1CNT	*((vuint16_t *)(REG_BASE + 0x0a))
/*! \def REG_BG2CNT

	\brief Background 2 control register.

*/
#define REG_BG2CNT	*((vuint16_t *)(REG_BASE + 0x0c))
/*! \def REG_BG3CNT

	\brief Background 3 control register.

*/
#define REG_BG3CNT	*((vuint16_t *)(REG_BASE + 0x0e))

typedef struct {
	vuint16_t x;
	vuint16_t y;
} bg_scroll;

#define BG_OFFSET ((bg_scroll *)(REG_BASE + 0x10))

#define	REG_BG0HOFS		*((uint16_t *)(REG_BASE + 0x10))	// BG 0 H Offset
#define	REG_BG0VOFS		*((uint16_t *)(REG_BASE + 0x12))	// BG 0 V Offset
#define	REG_BG1HOFS		*((uint16_t *)(REG_BASE + 0x14))	// BG 1 H Offset
#define	REG_BG1VOFS		*((uint16_t *)(REG_BASE + 0x16))	// BG 1 V Offset
#define	REG_BG2HOFS		*((uint16_t *)(REG_BASE + 0x18))	// BG 2 H Offset
#define	REG_BG2VOFS		*((uint16_t *)(REG_BASE + 0x1a))	// BG 2 V Offset
#define	REG_BG3HOFS		*((uint16_t *)(REG_BASE + 0x1c))	// BG 3 H Offset
#define	REG_BG3VOFS		*((uint16_t *)(REG_BASE + 0x1e))	// BG 3 V Offset

#define	REG_BG2PA	*((int16_t *)(REG_BASE + 0x20))
#define	REG_BG2PB	*((int16_t *)(REG_BASE + 0x22))
#define	REG_BG2PC	*((int16_t *)(REG_BASE + 0x24))
#define	REG_BG2PD	*((int16_t *)(REG_BASE + 0x26))
#define	REG_BG2X	*((int16_t *)(REG_BASE + 0x28))
#define	REG_BG2Y	*((int16_t *)(REG_BASE + 0x2c))
#define	REG_BG3PA	*((int16_t *)(REG_BASE + 0x30))
#define	REG_BG3PB	*((int16_t *)(REG_BASE + 0x32))
#define	REG_BG3PC	*((int16_t *)(REG_BASE + 0x34))
#define	REG_BG3PD	*((int16_t *)(REG_BASE + 0x36))
#define	REG_BG3X	*((int16_t *)(REG_BASE + 0x38))
#define	REG_BG3Y	*((int16_t *)(REG_BASE + 0x3c))

#define BG_SIZE(m)		((m<<14))
/*! \enum BG_CTRL_BITS

	\brief bit values for background control
*/
enum BG_CTRL_BITS {
	BG_MOSAIC = BIT(6),		/*!< enable background mosaic			*/
	BG_16_COLOR = (0 << 7),		/*!< background uses 16 color tiles		*/
	BG_256_COLOR = BIT(7),		/*!< background uses 256 color tiles	*/
	BG_WRAP = BIT(13),	/*!< background wraps when scrolling	*/
	BG_SIZE_0 = BG_SIZE(0),	/*!< Map Size 256x256	*/
	BG_SIZE_1 = BG_SIZE(1),	/*!< Map Size 512x256	*/
	BG_SIZE_2 = BG_SIZE(2),	/*!< Map Size 256x512	*/
	BG_SIZE_3 = BG_SIZE(3),	/*!< Map Size 512x512	*/
};

#define	CHAR_BASE(m)		((m) << 2)
#define BG_TILE_BASE(m)		((m) << 2)
#define CHAR_BASE_ADR(m)	((void *)(VRAM + ((m) << 14)))
#define CHAR_BASE_BLOCK(m)	((void *)(VRAM + ((m) << 14)))
#define MAP_BASE_ADR(m)		((void *)(VRAM + ((m) << 11)))
#define SCREEN_BASE_BLOCK(m)((void *)(VRAM + ((m) << 11)))
#define SCREEN_BASE(m)		((m) << 8)
#define BG_MAP_BASE(m)		((m) << 8)

//alternate names for char and screen base
#define	TILE_BASE(m)		((m) << 2)
#define TILE_BASE_ADR(m)	((void *)(VRAM + ((m) << 14)))

#define MAP_BASE_ADR(m)		((void *)(VRAM + ((m) << 11)))
#define MAP_BASE(m)			((m) << 8)

#define BG_PRIORITY(m)		((m))
#define CHAR_PALETTE(m)		((m)<<12)

/*---------------------------------------------------------------------------------
	CHAR_BASE_ADR() is the direct equivalent to old PATRAM(),
	giving the base address of a chr bank.
	These macros pinpoint the base address of a single tile.
---------------------------------------------------------------------------------*/
#define PATRAM4(x, tn) ((u32 *)(VRAM | (((x) << 14) + ((tn) << 5)) ))
#define PATRAM8(x, tn) ((u32 *)(VRAM | (((x) << 14) + ((tn) << 6)) ))
#define SPR_VRAM(tn) ((u32 *)(VRAM | 0x10000 | ((tn) << 5)))

/*---------------------------------------------------------------------------------
	MAP_BASE_ADR() only gives the beginning of a map.
	Each cell of a text map can be accessed using 3D array notation:

	MAP[page][y][x]
---------------------------------------------------------------------------------*/
typedef uint16_t NAMETABLE[32][32];
#define MAP ((NAMETABLE *)0x06000000)

/*---------------------------------------------------------------------------------
	width and height of a GBA text map can (and probably should)
	be controlled separately.
---------------------------------------------------------------------------------*/
#define BG_WID_32 BG_SIZE_0
#define BG_WID_64 BG_SIZE_1
#define BG_HT_32  BG_SIZE_0
#define BG_HT_64  BG_SIZE_2
//---------------------------------------------------------------------------------
// Symbolic names for the rot/scale map sizes
//---------------------------------------------------------------------------------
#define ROTBG_SIZE_16  BG_SIZE_0
#define ROTBG_SIZE_32  BG_SIZE_1
#define ROTBG_SIZE_64  BG_SIZE_2
#define ROTBG_SIZE_128 BG_SIZE_3

#define TEXTBG_SIZE_256x256    BG_SIZE_0
#define TEXTBG_SIZE_512x256    BG_SIZE_1
#define TEXTBG_SIZE_256x512    BG_SIZE_2
#define TEXTBG_SIZE_512x512    BG_SIZE_3

#define ROTBG_SIZE_128x128    BG_SIZE_0
#define ROTBG_SIZE_256x256    BG_SIZE_1
#define ROTBG_SIZE_512x512    BG_SIZE_2
#define ROTBG_SIZE_1024x1024  BG_SIZE_3

//---------------------------------------------------------------------------------
// Framebuffers for mode 3 and 5
//---------------------------------------------------------------------------------
typedef uint16_t MODE3_LINE[240];
typedef uint16_t MODE5_LINE[160];

#define MODE3_FB ((MODE3_LINE *)0x06000000)
#define MODE5_FB ((MODE5_LINE *)0x06000000)
#define MODE5_BB ((MODE5_LINE *)0x0600A000)

#define	REG_WIN0H	*((vuint16_t *)(REG_BASE + 0x40))
#define	REG_WIN1H	*((vuint16_t *)(REG_BASE + 0x42))
#define	REG_WIN0V	*((vuint16_t *)(REG_BASE + 0x44))
#define	REG_WIN1V	*((vuint16_t *)(REG_BASE + 0x46))
#define	REG_WININ	*((vuint16_t *)(REG_BASE + 0x48))
#define	REG_WINOUT	*((vuint16_t *)(REG_BASE + 0x4A))

#define	REG_MOSAIC	*(vuint16_t *)(REG_BASE + 0x4c)

#define	REG_BLDCNT		*((vuint16_t *)(REG_BASE + 0x50))
#define	REG_BLDALPHA	*((vuint16_t *)(REG_BASE + 0x52))
#define	REG_BLDY		*((vuint16_t *)(REG_BASE + 0x54))

//---------------------------------------------------------------------------------
// Helper macros
//---------------------------------------------------------------------------------
static inline void SetMode(int mode) { REG_DISPCNT = mode; }

#define RGB5(r,g,b)	((r)|((g)<<5)|((b)<<10))
#define RGB8(r,g,b)	( (((b)>>3)<<10) | (((g)>>3)<<5) | ((r)>>3) )


#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

/*! \var typedef void ( * IntFn)(void)
	\brief A type definition for an interrupt function pointer
*/
typedef void (*IntFn)(void);

struct IntTable { IntFn handler; uint32_t mask; };

#define MAX_INTS	15

#define INT_VECTOR	*(IntFn *)(0x03007ffc)		// BIOS Interrupt vector
/*! \def REG_IME

	\brief Interrupt Master Enable Register.

	When bit 0 is clear, all interrupts are masked.  When it is 1,
	interrupts will occur if not masked out in REG_IE.

*/
#define	REG_IME		*(vuint16_t *)(REG_BASE + 0x208)	// Interrupt Master Enable
/*! \def REG_IE

	\brief Interrupt Enable Register.

	This is the activation mask for the internal interrupts.  Unless
	the corresponding bit is set, the IRQ will be masked out.
*/
#define	REG_IE		*(vuint16_t *)(REG_BASE + 0x200)	// Interrupt Enable
/*! \def REG_IF

	\brief Interrupt Flag Register.

	Since there is only one hardware interrupt vector, the IF register
	contains flags to indicate when a particular of interrupt has occured.
	To acknowledge processing interrupts, set IF to the value of the
	interrupt handled.

*/
#define	REG_IF		*(vuint16_t *)(REG_BASE + 0x202)	// Interrupt Request

//!  interrupt masks.
/*!
  These masks are used in conjuction with REG_IE to enable specific interrupts
  and with REG_IF to acknowledge interrupts have been serviced.
*/
typedef enum irqMASKS {
	IRQ_VBLANK = (1 << 0),		/*!< vertical blank interrupt mask */
	IRQ_HBLANK = (1 << 1),		/*!< horizontal blank interrupt mask */
	IRQ_VCOUNT = (1 << 2),		/*!< vcount match interrupt mask */
	IRQ_TIMER0 = (1 << 3),		/*!< timer 0 interrupt mask */
	IRQ_TIMER1 = (1 << 4),		/*!< timer 1 interrupt mask */
	IRQ_TIMER2 = (1 << 5),		/*!< timer 2 interrupt mask */
	IRQ_TIMER3 = (1 << 6),		/*!< timer 3 interrupt mask */
	IRQ_SERIAL = (1 << 7),		/*!< serial interrupt mask */
	IRQ_DMA0 = (1 << 8),		/*!< DMA 0 interrupt mask */
	IRQ_DMA1 = (1 << 9),		/*!< DMA 1 interrupt mask */
	IRQ_DMA2 = (1 << 10),	/*!< DMA 2 interrupt mask */
	IRQ_DMA3 = (1 << 11),	/*!< DMA 3 interrupt mask */
	IRQ_KEYPAD = (1 << 12),	/*!< Keypad interrupt mask */
	IRQ_GAMEPAK = (1 << 13)		/*!< horizontal blank interrupt mask */
} irqMASK;

extern struct IntTable IntrTable[];

/*! \fn void irqInit(void)
	\brief initialises the gba interrupt code.

*/
void InitInterrupt(void) __attribute__((deprecated));
void irqInit();

/*! \fn IntFn *irqSet(irqMASK mask, IntFn function)
	\brief sets the interrupt handler for a particular interrupt.

	\param mask
	\param function
*/
IntFn* SetInterrupt(irqMASK mask, IntFn function) __attribute__((deprecated));
IntFn* irqSet(irqMASK mask, IntFn function);
/*! \fn void irqEnable(int mask)
	\brief allows an interrupt to occur.

	\param mask
*/
void EnableInterrupt(irqMASK mask) __attribute__((deprecated));
void irqEnable(int mask);

/*! \fn void irqDisable(int mask)
	\brief prevents an interrupt occuring.

	\param mask
*/
void DisableInterrupt(irqMASK mask) __attribute__((deprecated));
void irqDisable(int mask);

void IntrMain();


#else

#define IWRAM_CODE	
#define EWRAM_CODE	

#define IWRAM_DATA	
#define EWRAM_DATA	

#endif

#endif