 
# gba-demo [WIP]
 
A GBA demo / vertical slice built from the ground up, point and click adventure style demo with 3d entities over pre-rendered 2d backgrounds, also including video playback and 3d audio. The demo is based on an existing game made by Ben 'Yahtzee' Croshaw, [7 Days a Skeptic](http://www.fullyramblomatic.com/7days/). This is a personal project to aid developing my own skillset, inspiration / desire for this project came about after seeing [Raylight Studio's Blue Roses GBA Tech](https://danielprimed.com/2009/09/visual-connection-%E2%80%93-raylight-studios-blue-roses-gba-tech/), in particular the Resident Evil 2 tech demo. 

Builds for Windows, Linux (SDL2), and GBA.

## Linux

The Linux executable targets 32-bit x86, including on x86-64 hosts. Install GCC
multilib, Make, pkg-config, 32-bit SDL2 development headers/libraries, and GDB.
On Debian/Ubuntu x86-64:

```sh
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install build-essential gcc-multilib pkg-config libsdl2-dev:i386 gdb
make -f Makefile.linux -j4
make -f Makefile.linux run
```

Compilation and linking both enforce `-m32`. The Makefile selects 32-bit SDL2
pkg-config metadata; override `PKG_CONFIG_LIBDIR` if your distribution stores it
elsewhere. Existing objects are rebuilt when the Makefile changes.

Run from the repository root so the resource pack in `data/` can be found.
Debug builds are written to `build/linux/debug/`; use `CONFIG=release` for
`build/linux/release/`. Both configurations use `build/linux/7days.sav`, leaving
the checked-in save file untouched. Set `GBA_SAVE_PATH` to use another save file.
Debug logs are written to `build/linux/log.txt`.

```sh
make -f Makefile.linux CONFIG=release -j4
```

Both desktop builds use `source/main.desktop.c` and share `.desktop.c` backends, including SDL2 audio, input,
display timing, and profiling, plus portable desktop-only fallbacks. The SDL
application wrapper lives in `source/common/platform/sdl2/`; code shared with
the ROM uses plain `.c`. OS services retain `.win32.c` or `.linux.c` names: resource/save
mappings, Linux coroutine stacks, and the Windows window icon hook. Linux
selects shared, desktop, SDL platform, and Linux files without compiling Windows backends.
The debug host build retains optimization for the engine's inline functions,
so some variables may be optimized out in GDB.

Controls: arrows or WASD for the D-pad, X/Z for A/B, Q/E for L/R,
Enter for Start, and Backspace for Select.

## GBA ROM

Install devkitPro's GBA development tools (`gba-dev`), including devkitARM and
`gbafix`. Export the toolchain paths before launching VS Code:

```sh
export DEVKITPRO=/opt/devkitpro
export DEVKITARM="$DEVKITPRO/devkitARM"
export PATH="$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH"
make -j4
make DEBUG=1 -j4
```

Release outputs are `<repository-name>.gba` and `<repository-name>.elf`;
debug outputs have a `-debug` suffix. Objects are isolated in `build/gba/`
and `build/gba-debug/`. `make clean` cleans release outputs;
`make clean DEBUG=1` cleans debug outputs. The ROM Makefile retains legacy
type-conversion diagnostics as warnings for compatibility with newer GCC.

## Browser player

The static page in [`web/`](web/index.html) plays the built ROM using
[gbajs2](https://github.com/andychase/gbajs2). Build the release ROM with `make`,
then serve the **repository root** over HTTP, for example:

```sh
python3 -m http.server 8000 --bind 127.0.0.1
```

Open <http://127.0.0.1:8000/web/> and click **Play** to enable playback and audio.
Opening the HTML directly via `file://` does not work. The player loads
`gba-demo.gba` from the repository root without caching it, so refresh after a
rebuild. To play a debug build, visit
<http://127.0.0.1:8000/web/?rom=../gba-demo-debug.gba>. The `rom` query parameter
also supports other output names; paths are relative to the player page.

Controls: arrows for the D-pad, X/Z for A/B, Q/E for L/R, Enter for Start, and
Backspace for Select. The player offers pause/resume, restart, and mute, and
automatically pauses when focus is lost. In-game saves use gbajs2's browser
local storage; they do not read or modify the repository's `.sav` files.
Restart resets the console, not the saved game.

The generated `web/gbajs2.js` bundle is included, so no frontend build step
or runtime CDN access is needed. To deploy on a static
host, publish `web/` together with the built ROM in its parent directory (or
set `?rom=` to the ROM's deployed URL). ROMs are ignored by Git, so include
the build output explicitly when deploying.

The unmodified gbajs2 runtime in `web/js/` and its replacement BIOS in
`web/resources/biosbin.js` are vendored from commit
`5f575bf5db71e6e5bc88eed7af41f426d36ce95d`; its redistribution license is
[`web/COPYING`](web/COPYING). The page loads the minified `web/gbajs2.js`,
which combines the emulator scripts and replacement BIOS in dependency order.
The renderer still runs separately: preserve `web/js/video/worker.js` and
`web/js/video/software.js`, which it loads relative to the player page.

After changing the vendored sources, regenerate the bundle with Make and
[Terser](https://terser.org/) installed:

```sh
make -C web
```

Commit the regenerated `web/gbajs2.js` alongside source changes. The other
files in `web/js/` and `web/resources/` are retained as readable build inputs.

## VS Code

Install the recommended Microsoft C/C++ extension. The default build task
(Ctrl+Shift+B) builds the GBA ROM. **Terminal > Run Task** also offers a debug
ROM build, Linux debug/release builds, Linux run, and clean tasks.
Select **Linux: Debug** and press F5 to build and debug the native executable.
Use **C/C++: Select IntelliSense Configuration** to switch between Linux and GBA.
Adjust the Linux SDL include path if your distribution installs it elsewhere.

To debug the ROM, build the debug ROM, open it in mGBA, and start mGBA's GDB
server on port 2345. Select **GBA: Attach to mGBA GDB Server** in VS Code.
This uses devkitARM's GDB and the debug ELF; the emulator must already be running.

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
