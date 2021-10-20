
#include "common/types.h"

/*! \var typedef void ( * IntFn)(void)
	\brief A type definition for an interrupt function pointer
*/
typedef void (*IntFn)(void);

struct IntTable { IntFn handler; uint32_t mask; };

#define MAX_INTS	15

#define	REG_BASE	0x04000000

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
typedef enum irqMASKS 
{
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
void irqInit();

/*! \fn IntFn *irqSet(irqMASK mask, IntFn function)
	\brief sets the interrupt handler for a particular interrupt.
	\param mask
	\param function
*/
IntFn* irqSet(irqMASK mask, IntFn function);

//---------------------------------------------------------------------------------
struct IntTable IntrTable[MAX_INTS];
void dummy(void) {};


//---------------------------------------------------------------------------------
void InitInterrupt(void) 
{
	irqInit();
}

void IntrMain();

//---------------------------------------------------------------------------------
void irqInit() 
{
	int i;

	// Set all interrupts to dummy functions.
	for (i = 0; i < MAX_INTS; i++)
	{
		IntrTable[i].handler = dummy;
		IntrTable[i].mask = 0;
	}

	INT_VECTOR = IntrMain;
}

//---------------------------------------------------------------------------------
IntFn* SetInterrupt(irqMASK mask, IntFn function)
{
	return irqSet(mask, function);
}

//---------------------------------------------------------------------------------
IntFn* irqSet(irqMASK mask, IntFn function) 
{
	int i;

	for (i = 0;; i++) {
		if (!IntrTable[i].mask || IntrTable[i].mask == mask) break;
	}

	if (i >= MAX_INTS) return NULL;

	IntrTable[i].handler = function;
	IntrTable[i].mask = mask;

	return &IntrTable[i].handler;

}

#define	REG_DISPSTAT * ((vuint16_t*)(REG_BASE + 0x04))
enum LCDC_IRQ
{
	LCDC_VBL_FLAG = (1 << 0),
	LCDC_HBL_FLAG = (1 << 1),
	LCDC_VCNT_FLAG = (1 << 2),
	LCDC_VBL = (1 << 3),
	LCDC_HBL = (1 << 4),
	LCDC_VCNT = (1 << 5)
};
//---------------------------------------------------------------------------------
void irqEnable(int mask) 
{
	REG_IME = 0;

	if (mask & IRQ_VBLANK) REG_DISPSTAT |= LCDC_VBL;
	if (mask & IRQ_HBLANK) REG_DISPSTAT |= LCDC_HBL;
	if (mask & IRQ_VCOUNT) REG_DISPSTAT |= LCDC_VCNT;
	REG_IE |= mask;
	REG_IME = 1;
}

//---------------------------------------------------------------------------------
void irqDisable(int mask)
{
	REG_IME = 0;

	if (mask & IRQ_VBLANK) REG_DISPSTAT &= ~LCDC_VBL;
	if (mask & IRQ_HBLANK) REG_DISPSTAT &= ~LCDC_HBL;
	if (mask & IRQ_VCOUNT) REG_DISPSTAT &= ~LCDC_VCNT;
	REG_IE &= ~mask;

	REG_IME = 1;
}

//---------------------------------------------------------------------------------
void EnableInterrupt(irqMASK mask)
{
	irqEnable(mask);
}

//---------------------------------------------------------------------------------
void DisableInterrupt(irqMASK mask) 
{
	irqDisable(mask);
}