
@--------------------------------------------------------------------
@  Containers GBA : sort
@--------------------------------------------------------------------
	
	.file "sort.gba.s"

@--------------------------------------------------------------------
	
	.section .iwram, "ax", %progbits;								
	.arm;									
	.align 4;								
	.global sort_insertion;		
	.syntax unified					
	.type sort_insertion STT_FUNC;


@--------------------------------------------------------------------
sort_insertion:

	@--------------------------
	@ r0	= sort_index_t* lo
	@ r1	= const uint32_t num
	@ r2	= sort_index_t* hi
	@ r3	= sort_index_t* max
	@ r4	= p
	@ r5	= ?
	@--------------------------

	push { r4, r5, lr }
	
	sub r1, r1, #1 
	add r2, r0, r1, lsl #2


.sort_insertion.main_loop:

	mov r3, r0
	ldrh r1, [r3]

	add r4, r0, #4
	cmp r4, r2
	bgt .sort_insertion.inner_loop_end


.sort_insertion.inner_loop:

	ldrh r5, [r4]		 
			
	cmp r5, r1
	movgt r3, r4
	movgt r1, r5
	
	add r4, r4, #4
	cmp r4, r2
	bgt .sort_insertion.inner_loop_end


	ldrh r5, [r4]		 
			
	cmp r5, r1
	movgt r3, r4
	movgt r1, r5
	
	add r4, r4, #4
	cmp r4, r2
	ble .sort_insertion.inner_loop


.sort_insertion.inner_loop_end:

	ldr r5, [r2]
	ldr r1, [r3]
	str r1, [r2]
	str r5, [r3]

	sub r2, r2, #4
	cmp r2, r0
	bgt .sort_insertion.main_loop
	

@------------------------------------------------------------------------------
.sort_insertion.end:

	pop { r4, r5, pc }

	.size	sort_insertion, .-sort_insertion


@ EOF