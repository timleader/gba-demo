
#ifndef CODEGEN_ARMV4_H
#define CODEGEN_ARMV4_H

#include "common/types.h"
#include "common/debug/debug.h"

typedef enum armv4_register_e 
{
	armv4_register_r0	= 0,
	armv4_register_r1	= 1,
	armv4_register_r2	= 2,
	armv4_register_r3	= 3,
	armv4_register_r4	= 4,
	armv4_register_r5	= 5,
	armv4_register_r6	= 6,
	armv4_register_r7	= 7,
	armv4_register_r8	= 8,
	armv4_register_r9	= 9,
	armv4_register_r10	= 10,

} armv4_register_t;

typedef enum armv4_opcode_e
{
	armv4_opcode_mov	= 0xE3A0,

	armv4_opcode_and	= 0xE000,

} armv4_opcode_t;

typedef uint32_t armv4_instruction_t;

#define inline __inline


static inline armv4_instruction_t codegen_armv4_mov_instruction(armv4_register_t Rd, uint8_t immediate, uint8_t ror_value)
{
	//	how do we decompose a 32 bit number into the below 

	//	all 1s have to be within 8 bits of each other ... 
	//	then there is also an even restriction ... 

	//	4 bits = x		//	x has to be even and within range of 0 -> 31
	//	8 bits = y		//	y has to be in range of 8 bits  

	//	y ror (x << 1)



	//	0x0FCO							  00001111 11000000

	//										13 -- so (13 << 1) is the number of bits to ror 
	//  0x0D3F							  00001101 00111111

	//					00000000 00000000 00000000 00000000

	debug_assert(Rd <= 0x0F, "destination register idx is out of bounds");

	ror_value >>= 1;

	armv4_instruction_t instruction = (armv4_opcode_mov << 16) | (Rd << 12) | (ror_value << 8) | immediate;
	return instruction;
}

static inline armv4_instruction_t codegen_armv4_and_instruction(armv4_register_t Rd, armv4_register_t Rn, armv4_register_t Rm, uint8_t shift_op, uint8_t shift_val)
{
	shift_op = 4;

	armv4_instruction_t instruction = (armv4_opcode_and << 16) | (Rn << 16) | (Rd << 12) | (shift_val << 7) | (shift_op << 4) | Rm;
	return instruction;
}

/*
	initial support 
		
		//	and r2, r5, r4, asr (invertedbitwiseWidth)

		//	mov r5, bitMaskWidth   0xE3 A0 50 00 | (bitMaskWidth & 0x00FF);
*/

#endif
