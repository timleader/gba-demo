
## todo

- Settings
    - Backlight support enabled 
    - Volume SFX, Music

- Save Game 
    - Center the 2 slots 
    - save text should represent what day your on, eg. day #1 
    - back and action buttons at buttom of screen 

- Manually created z layers 

- GBA Z Depth Code (ASM)  -- 275,468
    - employ the CPU mode switching
    - load a of z span

- Collect images of all scenes and characters 

- pre-computed visiblity culling 
    - encode the direction of a triangle as uint8_t
    - sort triangles based on direction
    - culling based on X and Z not Y

- use triangles instead of quads, 
- simplify character models 
- LUT table at a given address, so we can simplfy the lookup

- Enemy AI 
    - Behaviour

- Death 

- Credits 

- UI for picking up an item

- Animated Environment

- Content Completion ***
    - Environments 
    - Story 
    - Enemy
    - NPCs

- State_Model 
    - better camera controls 
    - animation playback support 
    - Model / Texturing improvements
    - Polygon sorting
    - rendering optimizations 
    - floor mesh crash 
- Create alternative characters 

-------------------------------------

- switching to IRQ enabled gives up access to and alternative r8-r14 (I think r7 might be required for fiq_on / off)

```     8, 9, 10, 11, 12, 
.macro fiq_on
    msr cpsr, #0x11     // switch r8-r14 to FIQ (IRQ enabled)
.endm

.macro fiq_off
    msr cpsr, #0x1F     // restore r8-r14
.endm
```

https://github.com/XProger/OpenLara/blob/master/src/platform/gba/asm/common_asm.inc

--------------------------------------

- Character Model / Animation Improvements 

- Environment Completion 
