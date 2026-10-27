TERSER ?= terser

SOURCES := \
	js/util.js \
	js/core.js \
	js/arm.js \
	js/thumb.js \
	js/mmu.js \
	js/io.js \
	js/audio.js \
	js/video.js \
	js/video/proxy.js \
	js/video/software.js \
	js/irq.js \
	js/keypad.js \
	js/sio.js \
	js/savedata.js \
	js/gpio.js \
	js/gba.js \
	resources/biosbin.js

.PHONY: all
all: gbajs2.js

gbajs2.js: $(SOURCES) COPYING Makefile
	$(TERSER) $(SOURCES) --compress --mangle \
		--format "preamble='/*! gbajs2 - Copyright (c) 2012 Jeffrey Pfau; (c) 2020 Andrew Chase. All rights reserved. BSD license: see COPYING. */'" \
		--output $@
