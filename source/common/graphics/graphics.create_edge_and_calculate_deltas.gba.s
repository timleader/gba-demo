
@------------------------------------------------------------------------------
@  Graphics GBA : create_edge_and_calculate_deltas
@------------------------------------------------------------------------------
	
	.file "graphics.create_edge_and_calculate_deltas.gba.s"
	
	.extern reciprocal_lut

	.extern debug_assert
	
@------------------------------------------------------------------------------
	
	.section .iwram, "ax", %progbits;								
	.arm;									
	.align 4;								
	.global create_edge_and_calculate_deltas;		
	.syntax unified					
	.type create_edge_and_calculate_deltas STT_FUNC;

	
@------------------------------------------------------------------------------
create_edge_and_calculate_deltas:

	@--------------------------
	@ r0	= edge_t* edge
	@ r1	= vertex_t* v1
	@ r2	= vertex_t* v2
	@--------------------------

	push { r4, r5, r6, r7, r8, r9, r10 }

	ldr r3, [r1, #4]				@ v1->position.y
	ldr r4, [r2, #4]				@ v2->position.y


	cmp r3, r4
	
	mov r5, r1
	movgt r1, r2
	movgt r2, r5

	mov r5, r3
	movgt r3, r4
	movgt r4, r5

	@ 


	ldr r6, .create_edge_and_calculate_deltas.ceil_mask
	mvn r7, r6


	mov r10, #1 

	and r8, r3, r7
	ands r9, r3, r6
	addgt r3, r8, r10, lsl #16

	str r3, [r0, #24]				@ edge->y = fixed16_ceil(tv1->position.y);

	
	and r8, r4, r7
	ands r9, r4, r6
	addgt r4, r8, r10, lsl #16

	sub r4, r4, r3
	str r4, [r0, #28]				@ edge->height = fixed16_ceil(tv2->position.y) - edge->y;

	cmp r4, #1024					@ debug_assert(fixed16_to_int(edge->height) < 1024, "graphics::createEdgeAndCalculateDeltas - edge->height >= 1024");
	ble .create_edge_and_calculate_deltas.proxy_debug_assert

.create_edge_and_calculate_deltas.proxy_debug_assert_end:
						
						                                                                                                                                                                                   
	ldr r0, =reciprocal_lut			@ fixed16_t[] 
	lsl r1, lr, #2
	ldr r0, [r0, r1]				@ fixed16_t overHeight = reciprocal_lut[fixed16_to_int(edge->height)];
									
	
	@sub r3, r9, r4
	@asr r3, r3, #8
	@mul r0, r3, r0 				@ edge->dx = fixed16_mul_approx2(tv2->position.x - tv1->position.x, overHeight);	

	
	@sub r3, r9, r4
	@asr r3, r3, #8
	@mul r0, r3, r0					@ edge->du = fixed16_mul_approx2(tv2->uv.x - tv1->uv.x, overHeight);

	
	@sub r3, r9, r4
	@asr r3, r3, #8
	@mul r0, r3, r0 				@ edge->dv = fixed16_mul_approx2(tv2->uv.y - tv1->uv.y, overHeight);

	sub r0, r1						@ fixed16_t subPix = edge->y - tv1->position.y;		@	 do this earlier 
	
	@sub r3, r9, r4
	@asr r3, r3, #8
	@mul r0, r3, r0					@ edge->x = tv1->position.x + fixed16_mul_approx2(edge->dx, subPix);	
									
	
	ldr r8, [r1, #16]
	str r8, [r1, #8]				@ edge->u = tv1->uv.x;

	ldr r8, [r1, #20]
	str r8, [r1, #16]				@ edge->v = tv1->uv.y;


@------------------------------------------------------------------------------
.create_edge_and_calculate_deltas.end:

	pop { r4, r5, r6, r7, r8, r9, r10 }

	bx lr 
	
@------------------------------------------------------------------------------
.create_edge_and_calculate_deltas.proxy_debug_assert:

	mov r1, .create_edge_and_calculate_deltas.debug_assert_msg
	bl debug_assert

	b .create_edge_and_calculate_deltas.proxy_debug_assert_end

@------------------------------------------------------------------------------
	.align 4

.create_edge_and_calculate_deltas.ceil_mask:
	.word	0x0000FFFF

.create_edge_and_calculate_deltas.debug_assert_msg:
	.asciz  "graphics::createEdgeAndCalculateDeltas - edge->height >= 1024"
	
@------------------------------------------------------------------------------

	.size	create_edge_and_calculate_deltas, .-create_edge_and_calculate_deltas


@ EOF
