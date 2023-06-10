
## todo


-------------------------------------

- switching to IRQ enabled gives up access to and alternative r8-r14 (I think r7 might be required for fiq_on / off)

```
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
