; Copyright (c) Nuvoton Technology Corp. All rights reserved.

LR_ROM      0x00000000  0x00200000  ; 2MB (SPI FLash)
{
	ER_STARTUP +0
    {
        startup_nuc505Series.o(RESET, +First)
	}
    
    ER_RO +0
    {
		*(+RO)
	}
    ER_CLKSWITCH   0x20000000  0x00004000
    {
        CLK.o
		usbd.o
		hid_vendor.o
	}
	
    ER_RW           +0
	{
		*(+RW)
	}
}

LR_RAM      0x20004000  0x0000C000
{	
	ER_ZI +0
	{
		*(+ZI)
	}
}
