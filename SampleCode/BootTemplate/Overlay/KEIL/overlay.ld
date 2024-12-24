; Copyright (c) Nuvoton Technology Corp. All rights reserved.


LR_ROM      0x00000000
{
	ER_STARTUP +0
    {
        startup_nuc505Series.o(RESET, +First)   ; vector table
        *(InRoot$$Sections)                     ; library init
        ; If neither (+ input_section_attr) nor (input_section_pattern) is specified, the default is +RO.
        startup_nuc505Series.o                  ; startup
        system_nuc505Series.o(.text.SystemInit)    
	}
    
    ER_RO       +0
    {
		*(+RO)
	}
    
    ; Relocate vector table in SRAM for fast interrupt handling.
    ER_VECTOR2  0x20000000  EMPTY   0x00000400
    {
    }
    
    ; Critical code located in SRAM. Loaded by ARM C library at startup.
    ER_FASTCODE_INIT +0
    {
        clk.o(+RO)          ; CLK_SetCoreClock() may take a long time if it is run on SPI Flash.
        ovlymgr.o(+RO)    ; Overlay manager itself must locate in SRAM because it will be responsible for loading code 
                            ; through SPIM DMA Read.
    }
    
    ER_RW       +0
	{
		*(+RW)
	}
    
    ER_ZI       +0
    {
        *(+ZI)
    }
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ER_OVERLAY_A +0  OVERLAY NOCOMPRESS
    {
        *(overlay_a)
    }
    ER_OVERLAY_B +0  OVERLAY NOCOMPRESS
    {
        *(overlay_b)
    }
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
    ; Serve to separate overlay regions.
    ER_SEPARATOR_1  +0  EMPTY   0   {}
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ER_OVERLAY_C +0  OVERLAY NOCOMPRESS
    {
        *(overlay_c)
    }
    ER_OVERLAY_D +0  OVERLAY NOCOMPRESS
    {
        *(overlay_d)
    }
    ER_OVERLAY_E +0  OVERLAY NOCOMPRESS
    {
        *(overlay_e)
    }
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
    ; Serve to seperate overlay regions.
    ER_SEPARATOR_2  +0  EMPTY   0   {}
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ER_OVERLAY_F +0  OVERLAY NOCOMPRESS
    {
        *(overlay_f)
    }
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
    ; Serve to mark end of used SRAM.
    ER_INDICATOR_END  +0  EMPTY   0 {}
}
ScatterAssert(LoadLimit(LR_ROM) <= 0x00200000)
ScatterAssert(ImageLimit(ER_INDICATOR_END) <= 0x20020000)
