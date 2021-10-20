
@--------------------------------------------------------------------
@  Graphics GBA : clip_polygon
@
@--------------------------------------------------------------------
	
	.file "graphics.clip_polygon.gba.s"


@------------------------------------------------------------------------------
	
	.extern reciprocal_lut
	.extern g_graphics_context


@--------------------------------------------------------------------
	
	.section .iwram, "ax", %progbits;										
	.arm;									
	.align 4;								
	.global clip_polygon;		
	.syntax unified					
	.type clip_polygon STT_FUNC;


@--------------------------------------------------------------------
clip_polygon:

	push { r4, r5, r6, r7, r8, r9, r10, r11, r12, lr }	





@------------------------------------------------------------------------------
.clip_polygon.end:

	pop { r4, r5, r6, r7, r8, r9, r10, r11, r12, pc }


	.size	clip_polygon, .-clip_polygon


@ EOF
