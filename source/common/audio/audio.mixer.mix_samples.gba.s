
@------------------------------------------------------------------------------
@  Audio GBA : audio_mix_samples
@
@------------------------------------------------------------------------------
	
	.file "audio.mixer.mix_samples.gba.s"


@------------------------------------------------------------------------------
	
	.extern _audio_left_buffer 
	.extern _audio_right_buffer 
	.extern _audio_left_volume 
	.extern _audio_right_volume 
	.extern _audio_source_buffer 
	.extern _audio_sample_count 

	
@------------------------------------------------------------------------------
	
	.section .iwram, "ax", %progbits;								
	.arm;									
	.align 4;								
	.global audio_mix_samples;		
	.syntax unified					
	.type audio_mix_samples STT_FUNC;

	
@------------------------------------------------------------------------------
audio_mix_samples:

	push { r4, r5, r6, r7, r8, r9, r10, r11, r12, lr }	
	
	@--------------------------
	@ r7	= _audio_sample_count
	@ r8	= _audio_left_volume
	@ r9	= _audio_right_volume
	@ r10	= _audio_source_buffer
	@ r11	= _audio_left_buffer
	@ r12	= _audio_right_buffer
	@--------------------------
	
	ldr r7, =_audio_sample_count
	ldrsh r7, [r7]

	ldr r8, =_audio_left_volume
	ldrsh r8, [r8]

	ldr r9, =_audio_right_volume
	ldrsh r9, [r9]

	ldr r10, =_audio_source_buffer				@ int8_t* _audio_source_buffer		
	ldr r10, [r10]

	ldr r11, =_audio_left_buffer				@ int8_t* _audio_left_buffer		
	ldr r11, [r11]
	
	ldr r12, =_audio_right_buffer				@ int8_t* _audio_right_buffer		
	ldr r12, [r12]

								@ should we have one frame in the bank but only mix one frame at a time... 
.audio_mix_samples.loop:		@ 32,694 cycles for 1638 samples per channel or (820 for 1 frame at 20 fps)				--- ~131,000 cycles for 4 channels   or		~67,000 cycles  
								@ 4 channels => 8% cpu time		/		6 channels => 12% cpu time 


	@--------------------------
	@ r5	= destination samples x4
	@ r6	= source samples x4
	@ r7	= _audio_sample_count
	@ r8	= _audio_left_volume
	@ r9	= _audio_right_volume
	@ r10	= _audio_source_buffer
	@ r11	= _audio_left_buffer
	@ r12	= _audio_right_buffer
	@-------------------------- 


	@	would be more efficient with mixer buffer in iwram 		
	@	could be a signed / unsigned issue, where things are scaling up too much ...


	@	load 4 samples at the same time ,   [ r1, r6, lr ] => 12 samples   r2, r3 ? => 20 samples 
	ldmia r10!, { r1, r6, lr }		@@ doing more that 8 samples doesnt seem to offer much improvement 
	


	@ sample 1
	lsl r3, r1, #24
	asr r3, r3, #24		
	@ sample 2
	lsl r4, r1, #16
	asr r4, r4, #24


	@ left
	ldrsb r5, [r11], #1	
	mul r2, r3, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]

	ldrsb r5, [r11], #1
	mul r2, r4, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]
		

	@ right
	ldrsb r5, [r12], #1
	mul r2, r3, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]

	ldrsb r5, [r12], #1
	mul r2, r4, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]


	
	@ sample 1
	lsl r3, r1, #8
	asr r3, r3, #24		
	@ sample 2
	asr r4, r1, #24	


	@ left
	ldrsb r5, [r11], #1
	mul r2, r3, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]

	ldrsb r5, [r11], #1
	mul r2, r4, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]
		

	@ right
	ldrsb r5, [r12], #1
	mul r2, r3, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]

	ldrsb r5, [r12], #1
	mul r2, r4, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]

	
	@----------

	
	@ sample 1
	lsl r3, r6, #24
	asr r3, r3, #24		
	@ sample 2
	lsl r4, r6, #16
	asr r4, r4, #24


	@ left
	ldrsb r5, [r11], #1
	mul r2, r3, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]

	ldrsb r5, [r11], #1
	mul r2, r4, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]
		

	@ right
	ldrsb r5, [r12], #1
	mul r2, r3, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]

	ldrsb r5, [r12], #1
	mul r2, r4, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]


	
	@ sample 1
	lsl r3, r6, #8
	asr r3, r3, #24		
	@ sample 2
	asr r4, r6, #24	


	@ left
	ldrsb r5, [r11], #1
	mul r2, r3, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]

	ldrsb r5, [r11], #1
	mul r2, r4, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]
		

	@ right
	ldrsb r5, [r12], #1
	mul r2, r3, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]

	ldrsb r5, [r12], #1
	mul r2, r4, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]

	
	@----------

	
	@ sample 1
	lsl r3, lr, #24
	asr r3, r3, #24		
	@ sample 2
	lsl r4, lr, #16
	asr r4, r4, #24


	@ left
	ldrsb r5, [r11], #1
	mul r2, r3, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]

	ldrsb r5, [r11], #1
	mul r2, r4, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]
		

	@ right
	ldrsb r5, [r12], #1
	mul r2, r3, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]

	ldrsb r5, [r12], #1
	mul r2, r4, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]


	
	@ sample 1
	lsl r3, lr, #8
	asr r3, r3, #24		
	@ sample 2
	asr r4, lr, #24	


	@ left
	ldrsb r5, [r11], #1
	mul r2, r3, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]

	ldrsb r5, [r11], #1
	mul r2, r4, r8
	add r0, r5, r2, asr #8
	strb r0, [r11, #-1]
		

	@ right
	ldrsb r5, [r12], #1
	mul r2, r3, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]

	ldrsb r5, [r12], #1
	mul r2, r4, r9
	add r0, r5, r2, asr #8
	strb r0, [r12, #-1]


	subs r7, r7, #12
	bgt .audio_mix_samples.loop


@------------------------------------------------------------------------------
.audio_mix_samples.end:

	pop { r4, r5, r6, r7, r8, r9, r10, r11, r12, pc }


	.size	audio_mix_samples, .-audio_mix_samples

	
@ EOF
