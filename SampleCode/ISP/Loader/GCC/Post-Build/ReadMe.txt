[Pad.exe Command format]
   - Usage : [Converter] [Pattern 0/1] [Pad Size (KB)] [Input/Output File]
   - Usage : [Converter] [Pattern 0/1] [Pad Size (KB)] [Input File] [Output File]

[define.h]
If you want to change the Code / EndTag location, please make sure that the following settings of Merge.cmd are consistent with the definition in define.h.

/* Tag Setting */   
#define ISP_ENDTAG_OFFSET      0x0F800          /*  62KB - The End Tag is added when Post-Build*/
#define FIRMWARE_ENDTAG_OFFSET 0x80000          /* 512KB - The End Tag is added when Post-Build*/	 
	    
/* Execute Address */   
#define FIRMWARE_CODE_ADDR     0x20000          /* SPI Flash Offset 128KB ; Must be tha sam as the App execute address */
   
/* ISP Code address at SPI Flash */   
#define ISP_CODE_OFFSET        0x10000          /* SPI Flash Offset 64KB */

[Merge.cmd]
/**********************************************************************************************************************************
 *   ISP_Demo.bin
 *   +--------+------+
 *   | Loader | 0xFF |
 *   +--------+------+   
 *  0KB            64KB                               
 **********************************************************************************************************************************/

..\Post-build\Pad.exe 1 64 ..\..\Loader.bin ..\..\..\ISP_Demo.bin

/**********************************************************************************************************************************
 *   ISP_Demo.bin
 *   +--------+------+-----+
 *   | Loader | 0xFF | ISP |
 *   +--------+------+-----+                              
 *  0KB            64KB  (ISP_CODE_OFFSET == 64KB)  
 **********************************************************************************************************************************/

copy/B ..\..\..\ISP_Demo.bin + ..\..\..\ISP\ISP.bin ..\..\..\ISP_Demo.bin

/**********************************************************************************************************************************
 *   ISP_Demo.bin
 *   +--------+------+-----+------+
 *   | Loader | 0xFF | ISP | 0xFF | 
 *   +--------+------+-----+------+
 *  0KB            64KB         126KB                    
 **********************************************************************************************************************************/

..\Post-build\Pad.exe 1 126 ..\..\..\ISP_Demo.bin

/**********************************************************************************************************************************
 *   ISP_Demo.bin
 *   +--------+------+-----+------+--------+
 *   | Loader | 0xFF | ISP | 0xFF | ENDTAG | 
 *   +--------+------+-----+------+--------+
 *  0KB            64KB         126KB (ISP_Demo.bin) 
 *                  0KB          62KB (ISP_ENDTAG_OFFSET)                    
 **********************************************************************************************************************************/

copy/B ..\..\..\ISP_Demo.bin + ..\Post-build\EndTag.bin ..\..\..\ISP_Demo.bin

/**********************************************************************************************************************************
 *   ISP_Demo.bin
 *   +--------+------+-----+------+--------+------+
 *   | Loader | 0xFF | ISP | 0xFF | ENDTAG | 0xFF |
 *   +--------+------+-----+------+--------+------+
 *  0KB            64KB         126KB           128KB 
 **********************************************************************************************************************************/

..\Post-build\Pad.exe 1 128 ..\..\..\ISP_Demo.bin

/**********************************************************************************************************************************
 *   ISP_Demo.bin
 *   +--------+------+-----+------+--------+------+------------+
 *   | Loader | 0xFF | ISP | 0xFF | ENDTAG | 0xFF | DefaultApp |
 *   +--------+------+-----+------+--------+------+------------+
 *  0KB            64KB         126KB           128KB (FIRMWARE_CODE_ADDR is 128KB)  
 **********************************************************************************************************************************/

copy/B ..\..\..\ISP_Demo.bin + ..\..\..\DefaultApp\DefaultApp.bin ..\..\..\ISP_Demo.bin

/**********************************************************************************************************************************
 *   ISP_Demo.bin
 *   +--------+------+-----+------+--------+------+------------+------+
 *   | Loader | 0xFF | ISP | 0xFF | ENDTAG | 0xFF | DefaultApp | 0xFF |
 *   +--------+------+-----+------+--------+------+------------+------+
 *  0KB            64KB         126KB           128KB               640KB   
 **********************************************************************************************************************************/

..\Post-build\Pad.exe 1 640 ..\..\..\ISP_Demo.bin

/**********************************************************************************************************************************
 *   ISP_Demo.bin
 *   +--------+------+-----+------+--------+------+------------+------+--------+
 *   | Loader | 0xFF | ISP | 0xFF | ENDTAG | 0xFF | DefaultApp | 0xFF | ENDTAG |
 *   +--------+------+-----+------+--------+------+------------+------+--------+
 *  0KB            64KB         126KB           128KB               640KB (ISP_Demo.bin)   
 *                                                0KB               512KB
 **********************************************************************************************************************************/

copy/B ..\..\..\ISP_Demo.bin + ..\Post-build\EndTag.bin ..\..\..\ISP_Demo.bin

/**********************************************************************************************************************************
 * Copy UpdateApp.bin to the root folder of ISP demo code
 **********************************************************************************************************************************/

copy ..\..\..\UpdateApp\UpdateApp.bin ..\..\..\UpdateApp.bin

/**********************************************************************************************************************************
 *   UpdateApp.bin
 *   +-----------+------+
 *   | UpdateApp | 0xFF |
 *   +-----------+------+                               
 *  0KB               512KB 
 **********************************************************************************************************************************/

..\Post-build\Pad.exe 1 512 ..\..\..\UpdateApp.bin

/**********************************************************************************************************************************
 *   UpdateApp.bin
 *   +-----------+------+--------+
 *   | UpdateApp | 0xFF | ENDTAG |
 *   +-----------+------+--------+                               
 *  0KB               512KB
 **********************************************************************************************************************************/

copy/B ..\..\..\UpdateApp.bin + ..\Post-build\EndTag.bin ..\..\..\UpdateApp.bin

/**********************************************************************************************************************************
 * Copy DefaultApp.bin and rename to UpdateDefaultApp.bin to the root folder of ISP demo code
 **********************************************************************************************************************************/

copy ..\..\..\DefaultApp\DefaultApp.bin ..\..\..\UpdateDefaultApp.bin

/**********************************************************************************************************************************
 *   UpdateDefaultApp.bin
 *   +------------------+------+
 *   | UpdateDefaultApp | 0xFF |
 *   +------------------+------+                               
 *  0KB                      512KB 
 **********************************************************************************************************************************/

..\Post-build\Pad.exe 1 512 ..\..\..\UpdateDefaultApp.bin

/**********************************************************************************************************************************
 *   UpdateDefaultApp.bin
 *   +-----------+------+------+--------+
 *   | UpdateDefaultApp | 0xFF | ENDTAG |
 *   +-----------+------+------+--------+                               
 *  0KB               512KB
 *******************************************************************************************************************/

copy/B ..\..\..\UpdateDefaultApp.bin + ..\Post-build\EndTag.bin ..\..\..\UpdateDefaultApp.bin

