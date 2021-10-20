
@--------------------------------------------------------------------
@  Video GBA : hufftree
@--------------------------------------------------------------------
	
	.file "smacker.hufftree.gba.s"

@--------------------------------------------------------------------
	
	.section .iwram, "ax", %progbits;								
	.arm;									
	.align 4;								
	.global smacker_huff8_lookup;		
	.syntax unified					
	.type smacker_huff8_lookup STT_FUNC;


@--------------------------------------------------------------------
smacker_huff8_lookup:

	@--------------------------
	@ r0	= bitstream_t* bs
	@ r1	= smk_huff8_v5_t* t
	@
	@ r2	= bs->value
	@ r3	= bs->bit_idx
	@
	@ r4	= t->branch_1_offset
	@--------------------------

	push { r4, r5, lr }

	ldr r2, [r0, #16]
	ldrb r3, [r0, #12]

	mul r4, r4, #20

.smacker_huff8_lookup.loop:

	asr r2, r2, r3
	add r3, r3, #1
	ands r2, #0x01	@ <<--- this isn't right 

	@	bit_idx overflow 

	addgt r1, r1, r4
	addlt r1, r1, #20

	@	(tp->branch.is_branch_node == 0x0F)

	b .smacker_huff8_lookup.loop


	mov r0, r1		@ set t->leaf.value

@------------------------------------------------------------------------------
.smacker_huff8_lookup.end:

	pop { r4, r5, pc }

	.size	smacker_huff8_lookup, .-smacker_huff8_lookup


@ EOF
