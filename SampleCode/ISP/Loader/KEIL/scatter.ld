LR_ROM 0x00000000 0x00200000 ; 2MB (SPI Flash)
{
   ; here is the SPI Flash section (code and const data) that just use once only at startup
   ER_CODE1_CONSTDATA1 +0
   {
      startup_NUC505Series.o(RESET, +First)
      startup_NUC505Series.o
      system_NUC505Series.o
      anon$$obj.o
      __main.o
      __scatter*.o
   }

   ; here are the SRAM sections moved at startup for fast accessing.
   ; Relocate vector table in SRAM for fast interrupt handling.
   ER_VECTOR2          0x1FF19000 EMPTY 0x00000400
   {
   }

   ER_CODE2_CONSTDATA2 0x1FF19400       0x00004000
   {
      *(+RO) ; text code and const data
   }

   ER_DATA_ZI          0x1FF1D400       0x00002C00
   {
      *(+RW, +ZI) ; data and others
   }
}
