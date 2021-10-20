 
# gba-demo [WIP]
 
A GBA demo / vertical slice built from the ground up, point and click adventure style demo with 3d entities over pre-rendered 2d backgrounds, also including video playback and 3d audio. The demo is based on an existing game made by Ben 'Yahtzee' Croshaw, [7 Days a Skeptic](http://www.fullyramblomatic.com/7days/). This is a personal project to aid developing my own skillset, inspiration / desire for this project came about after seeing [Raylight Studio's Blue Roses GBA Tech](https://danielprimed.com/2009/09/visual-connection-%E2%80%93-raylight-studios-blue-roses-gba-tech/), in particular the Resident Evil 2 tech demo. 

Only builds for windows and GBA 

Source artwork and content pipeline unfortunately won't be made avalible as they are under license, though everything needed to build this demo is here in this repo.

![](/screenshot_01.png)

## debugging tutorial
1. launch mGBA.exe
2. Tools -> Start GDB Server...
3. devkitPro/devkitARM/bin/arm-none-eabi-gdb.exe 
4. connect to the gba gdb server `target remote :2345`
5. load symbols into gdb `file G:/workspace/git/gba-demo/gba-demo.elf`
6. set breakpoints `b .coroutine_start_step.issue`
7. run `c`
8. get register state `i r`
9. get line / address `x/i $pc`

https://www.cs.cmu.edu/~gilpin/tutorial/

## hardware usage 

### hw timers :
- sound = { 0, 1 }
- profiler = { 2, 3 }

### dma channels :
- sound = { 1, 2 }
- memory_copy = { 3 }

## contributions
- Art contributions would be very welcome. 

## useful references
  + https://www.gbadev.org/docs.php
  + https://github.com/gbdev/awesome-gbdev
  + https://github.com/gbdev/awesome-gbadev
  + https://ia801906.us.archive.org/19/items/GameBoyProgManVer1.1/GameBoyProgManVer1.1.pdf
  + https://www.coranac.com/tonc/text/
  + https://problemkaputt.de/gbatek.htm
  + https://www.youtube.com/watch?v=Qh4ttJB48i8&list=PLiZOOWHhACurd6iF5LC-NRxKu3wpv9gX4
  + https://www.youtube.com/watch?v=sP1DHYzK_Cs

