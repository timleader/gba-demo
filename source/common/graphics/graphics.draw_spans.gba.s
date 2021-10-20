
@--------------------------------------------------------------------
@  Graphics GBA : draw_spans
@
@	limitations: 
@		- only supports 64x64 texture size  
@
@--------------------------------------------------------------------
	
	.file "graphics.draw_spans.gba.s"


@------------------------------------------------------------------------------
	
	.extern reciprocal_lut
	.extern g_graphics_context
	.extern texture_data_ptr
	.extern _graphics_left_edge
	.extern _graphics_right_edge


@--------------------------------------------------------------------
	
	.section .rom, "ax", %progbits;								@	will be relocated to iwram at runtime 			
	.arm;									
	.align 4;								
	.global draw_spans;		
	.syntax unified					
	.type draw_spans STT_FUNC;


@--------------------------------------------------------------------
draw_spans:

	push { r4, r5, r6, r7, r8, r9, r10, r11, r12, lr }	

	ldr r0, =_graphics_left_edge			@	relative to pc is how this is implemented 
	ldr r1, =_graphics_right_edge					


	add r0, r0, #24
	add r1, r1, #24

	ldmia r0, { r3, r5 }
	ldmia r1, { r4, r6 }


	cmp r5, r6
	movle r9, r5					@ fixed16_t fheight
	movgt r9, r6


	asrs r10, r9, #16					@ int32_t iheight = fixed16_to_int(fheight); 
	ble .draw_spans.end
	

	sub r5, r5, r9					@ _graphics_left_edge.height -= fheight;
	sub r6, r6, r9					@ _graphics_right_edge.height -= fheight;
	mov r8, r3
	add r3, r3, r9					@ _graphics_left_edge.y += fheight;
	add r4, r4, r9					@ _graphics_right_edge.y += fheight;
	
	stmia r0, { r3, r5 }
	stmia r1, { r4, r6 }
	

	ldr r2, =g_graphics_context				@	g_graphics_context
	
	asr r8, r8, #16
	ldrh r4, [r2, #20]				@ g_graphics_context.width
	mul r11, r8, r4
	
	ldr r4, [r2, #4]				@ g_graphics_context.frameBuffer
	add r11, r4, r11				@ uint16_t* output_scanline_ptr = g_graphics_context.frameBuffer + (fixed16_to_int(_graphics_left_edge.y) * g_graphics_context.width);
	


	ldr r0, =_graphics_right_edge
	ldmia r0, { r2-r6 }
	
	mov r8, r2
	mov r9, r4
	mov r1, r6

	ldr r0, =_graphics_left_edge		
	ldmia r0, { r2-r6 }
	

	ldr r12, =texture_data_ptr				@ uint8_t* texture_data_ptr		
	ldr r12, [r12]
			

@------------------------------------------------------------------------------
.draw_spans.next_row:

	str r11, [sp, #-24]				
	
												@ int32_t left_edge_floor_x_int = fixed16_to_int(_graphics_left_edge.x);
	asr lr, r8, #16								@ int32_t right_edge_floor_x_int = fixed16_to_int(_graphics_right_edge.x);
	subs lr, lr, r2, asr #16					@ int32_t width = right_edge_floor_x_int - left_edge_floor_x_int;
	ble .draw_spans.scanline_end
	@ if (width < 0 && dx is < 0) get out of the whole func 


	sub r3, r1, r6

	ldr r0, =reciprocal_lut				@ fixed16_t[] reciprocal_lut			 
	lsl r1, lr, #2
	ldr r0, [r0, r1]				@ fixed16_t overWidth = reciprocal_lut[width];
	
	asr r3, r3, #8
	mul r1, r0, r3					@fixed16_t dv = fixed16_mul_approx2(right_edge_v - left_edge_v, overWidth);
	

	sub r3, r9, r4
	asr r3, r3, #8
	mul r0, r3, r0 					@fixed16_t du = fixed16_mul_approx2(right_edge_u - left_edge_u, overWidth);
	

	asr r3, r2, #16
	asr r5, r3, #1
	add r11, r11, r5, lsl #1		@uint16_t* output_span_ptr = output_scanline_ptr + (left_edge_floor_x_int >> 1);	
	

.draw_spans.uv_mask_assignment:
	mov r5, #0x003F					@ we will need to alter these via, self-modifying code  
	mov r7, #0x0FC0


	ands r3, r3, #0x01
	beq .draw_spans.scanline_head_skip

	
@------------------------------------------------------------------------------
.draw_spans.scanline_head:

	@--------------------------
	@ r0	= du
	@ r1	= dv
	@ r2	= *	- scratch
	@ r3	= *	- scratch
	@ r4	= u
	@ r5	= u_bitmask
	@ r6	= v
	@ r7	= v_bitmask
	@ r8	= * - not used
	@ r9	= *	- not used 
	@ r10	= iheight
	@ r11	= output_span_ptr 
	@ r12	= texture_data_ptr
	@ lr	= width
	@--------------------------
	
.draw_spans.texture_sample_01:
	and r2, r5, r4, asr #10		@int32_t texel_x = (u >> 10) & 0x003F;
	and r8, r7, r6, asr #4		@int32_t texel_y = (v >> 4) & 0x0FC0;
	orr r2, r2, r8				@int32_t sample_idx = texel_y | texel_x;

	ldrb r8, [r12, r2]			@ load from texture_data_ptr  @ we can shift here, maybe we can remove one of the above operations 
	
	add r4, r4, r0, asr #8		@ u += du	@ can u and v be baked down to 1 addition ? 
	add r6, r6, r1, asr #8		@ v += dv

	ldrb r2, [r11] 

	orr r2, r2, r8, lsl #8		@ merge 2 pixels into a halfword 

	strh r2, [r11], #2			@ str output_span_ptr

	
	sub lr, lr, #1


@------------------------------------------------------------------------------
.draw_spans.scanline_head_skip:

	@--------------------------
	@ lr	= width
	@--------------------------

	cmp lr, #1
	blt .draw_spans.scanline_end
	beq .draw_spans.scanline_tail


@------------------------------------------------------------------------------
.draw_spans.scanline_loop:

	@--------------------------
	@ r0	= du
	@ r1	= dv
	@ r2	= *	- scratch
	@ r3	= *	- scratch
	@ r4	= u
	@ r5	= u_bitmask
	@ r6	= v
	@ r7	= v_bitmask
	@ r8	= *	- scratch
	@ r9	= *	- not used 
	@ r10	= iheight
	@ r11	= output_span_ptr 
	@ r12	= texture_data_ptr
	@ lr	= width
	@--------------------------
	
.draw_spans.texture_sample_02:
	and r2, r5, r4, asr #10		@int32_t texel_ x = (u >> 10) & 0x003F;
	and r8, r7, r6, asr #4		@int32_t texel_y = (v >> 4) & 0x0FC0;
	orr r2, r2, r8				@int32_t sample_idx = texel_y | texel_x;
	
	ldrb r3, [r12, r2]			@ load from texture_data_ptr  @ we can shift here, maybe we can remove one of the above operations 
	
	add r4, r4, r0, asr #8		@ u += du	@ can u and v be baked down to 1 addition ? 
	add r6, r6, r1, asr #8		@ v += dv

	
	
.draw_spans.texture_sample_03:
	and r2, r5, r4, asr #10		@int32_t texel_ x = (u >> 10) & 0x003F;
	and r8, r7, r6, asr #4		@int32_t texel_y = (v >> 4) & 0x0FC0;
	orr r2, r2, r8				@int32_t sample_idx = texel_y | texel_x;
	
	ldrb r8, [r12, r2]			@ load from texture_data_ptr  @ we can shift here, maybe we can remove one of the above operations 
	
	add r4, r4, r0, asr #8		@ u += du	@ can u and v be baked down to 1 addition ? 
	add r6, r6, r1, asr #8		@ v += dv

	
	orr r2, r3, r8, lsl #8		@ merge 2 pixels into a halfword 

	
	strh r2, [r11], #2			@ str output_span_ptr

	
	sub lr, lr, #2
	cmp lr, #1
	bgt .draw_spans.scanline_loop

	
@------------------------------------------------------------------------------
.draw_spans.scanline_tail:

	blt .draw_spans.scanline_end
	
	@--------------------------
	@ r0	= du
	@ r1	= dv
	@ r2	= *	- scratch
	@ r3	= *	- scratch
	@ r4	= u
	@ r5	= u_bitmask
	@ r6	= v
	@ r7	= v_bitmask
	@ r8	= *	- scratch
	@ r9	= *	- not used 
	@ r10	= iheight
	@ r11	= output_span_ptr 
	@ r12	= texture_data_ptr
	@ lr	= width
	@--------------------------

	ldrb r3, [r11, #1] 
	
.draw_spans.texture_sample_04:
	and r2, r5, r4, asr #10		@int32_t texel_x = (u >> 10) & 0x003F;
	and r8, r7, r6, asr #4		@int32_t texel_y = (v >> 4) & 0x0FC0;
	orr r2, r2, r8				@int32_t sample_idx = texel_y | texel_x;

	ldrb r8, [r12, r2]			@ load from texture_data_ptr  @ we can shift here, maybe we can remove one of the above operations 
	

	orr r2, r8, r3, lsl #8		@ merge 2 pixels into a halfword 
	
	strh r2, [r11]				@ str output_span_ptr

	
@------------------------------------------------------------------------------
.draw_spans.scanline_end:


	ldr r0, =_graphics_right_edge						@ _graphics_right_edge			@ can avoid this if we store this on stack 
	ldmia r0, { r2-r7 }
	
	add r8, r2, r3							@ _graphics_right_edge.x += _graphics_right_edge.dx;
	str r8, [r0, #0]

	add r9, r4, r5							@ _graphics_right_edge.u += _graphics_right_edge.du;
	str r9, [r0, #8]

	add r1, r6, r7							@ _graphics_right_edge.v += _graphics_right_edge.dv;
	str r1, [r0, #16]
	

	ldr r0, =_graphics_left_edge					@ _graphics_left_edge			@ if we ldr sp, =_graphics_left_edge, then we could just use push and pop 		
	ldmia r0, { r2-r7 }

	add r2, r2, r3							@ _graphics_left_edge.x += _graphics_left_edge.dx;
	str r2, [r0, #0]

	add r4, r4, r5							@ _graphics_left_edge.u += _graphics_left_edge.du;
	str r4, [r0, #8]

	add r6, r6, r7							@ _graphics_left_edge.v += _graphics_left_edge.dv;
	str r6, [r0, #16]


	@	width check here, then we could maybe remove one of the ldmia, or at least remove double jmp  



	@ldr r3, =g_graphics_context
	@ldrh r3, [r3, #20]						@ g_graphics_context.width
	
	ldr r11, [sp, #-24]						@ still pushing and pop so cant use sp reg 
	add r11, r11, #240						@ output_scanline_ptr += g_graphics_context.width >> 1;	@ this should be parameter

	subs r10, r10, #1
	bgt .draw_spans.next_row


@------------------------------------------------------------------------------
.draw_spans.end:

	pop { r4, r5, r6, r7, r8, r9, r10, r11, r12, pc }


	.size	draw_spans, .-draw_spans


@------------------------------------------------------------------------------
	
	.data 
	.section .rom, "ax", %progbits;	
	.global draw_spans__params		@ refactor to    draw_spans_between_edges__params
	.align	4

draw_spans__params:		
	.word .draw_spans.uv_mask_assignment

	.word .draw_spans.texture_sample_01
	.word .draw_spans.texture_sample_02
	.word .draw_spans.texture_sample_03
	.word .draw_spans.texture_sample_04

	
	.size	draw_spans__params, .-draw_spans__params


@ EOF


@	target for st_model - character  => 120,000 cycles  (currently 175,000 cycles)

@	iwram texture cache								=> 3,000 cycle saving, now we support more texture sizes is this still possible 
@	try unrolling the scanline loop once more ?	
@	look closely at opcode cycle costs				=> read gba emulator source code 
@	whats the distribution of width values 
@		consider 0 width use case carefully			=> *** is this occurring, how often, can we prevent (early exit)
@	port the rest of draw_clipped_triangle to asm 

@	depth checks are going to fuck this over ?

@	early exit when width reduces to zero, and dx is neg
