
@--------------------------------------------------------------------
@  Compression GBA : lz77
@--------------------------------------------------------------------
	
	.file "lz77.gba.s"

@--------------------------------------------------------------------
	
	.section .cart, "ax", %progbits;									
	.thumb;
	.thumb_func;
	.align 2;			
	.global lz77_decompress_wram;		
	.syntax unified					
	.type lz77_decompress_wram STT_FUNC;

	@ convert to arm instructions so no instruction set jump
	@ consider an not bios, optimized version of this function 

@--------------------------------------------------------------------
lz77_decompress_wram:
	swi 0x11
	
@--------------------------------------------------------------------
.lz77_decompress_wram.end:
	bx lr
	.size	lz77_decompress_wram, .-lz77_decompress_wram
	
@--------------------------------------------------------------------
	


@--------------------------------------------------------------------
	
	.section .cart, "ax", %progbits;									
	.thumb;
	.thumb_func;
	.align 2;			
	.global lz77_decompress_vram;		
	.syntax unified					
	.type lz77_decompress_vram STT_FUNC;

	@ convert to arm instructions so no instruction set jump
	@ consider an not bios, optimized version of this function 

@--------------------------------------------------------------------
lz77_decompress_vram:
	swi 0x12
	
@--------------------------------------------------------------------
.lz77_decompress_vram.end:
	bx lr
	.size	lz77_decompress_vram, .-lz77_decompress_vram
	
@--------------------------------------------------------------------
	


@ EOF
