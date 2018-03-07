[ISP demo code architecture]

      SPI Flash
  +----------------+  0x00000000 
  |     Loader     | 
  +----------------+  0x00008000 
  |      ISP       |  
  +----------------+  0x00010000
  |                |
  |  App Firmware  |
  |                |
  +----------------+  0x00200000


[ISP demo Projects]
 - The ISP demo code include 4 projects: Loader, ISP, DefaultApp, UpdateApp.
    - Create binary files in each folder.
    - Loader project
      - Merge 3 binary files (Loader.bin / ISP.bin / DefaultApp.bin) to ISP_demo.bin in ISP_Demo root folder[Note 3]
      - Copy DefaultApp.bin (Rename to UpdateDefaultApp.bin) and UpdateApp.bin into ISP_Demo root folder[Note 3]. 
    - Because it will use the binary files from other projects. Please build it at last.
    - Developer can burn ISP_demo.bin by ICP Tool.

 - Binary files location
    - [ISP demo code] - [Loader]     - Loader.bin
                      - [ISP]        - ISP.bin
                      - [DefaultApp] - DefaultApp.bin
                      - [UpdateApp]  - UpdateApp.bin
                      - ISP_demo.bin         (Merge Loader.bin, ISP.bin, and DefaultApp.bin)
                      - UpdateDefaultApp.bin (The same as DefaultApp.bin for Update Test)
                      - UpdateApp.bin        (UpdateApp.bin for Update Test) 

[ISP demo code flow]
- Loader 
   - Check Bootup mode
     - SYS->BOOTSET == 0xF : Jump to App Firmware (SPI Flash 0x10000) if Tag is valid[Note 1]. Otherwise, Jump to ISP mode.
     - SYS->BOOTSET == 0xE : Load and jump to ISP (SRAM 0x20000000) if Tag is valid[Note 1].

- ISP
   - Work as a Virtual USB Mass storage device with 2MB capacity.
   - The Label name of the disk is the currect App Firmware version.
   - Developer can update the App Firmware by copying firmware binary file to the disk. After the copy operation is done, NUC505 will reboot automatically.
      - Developer can use the binary files of UpdateDefaultApp.bin & UpdateApp.bin to test the update function.
   - ISP updates SPI Flash when the file name meets the file name definition in define.h (ISP_Demo root folder).[Note 2].
      - ISP will show the file name and file size befire updating SPI Flash.

- App Firmware 
   - The way to enter ISP mode: Set SYS->BOOTSET to 0XE and set CPU reset.



[Note 1]
- Loader uses Tag to determine ISP or App Firmware is valid or not.
- The Tag structure is defined in define.h (ISP_Demo root folder).
  +-------+----------------+---------+-------+
  | Tag 0 | End Tag Offset | Version | Tag 1 |
  +-------+----------------+---------+-------+

  /* Tag Definition */
  #define TAG0_INDEX             0x00
  #define END_TAG_OFFSET_INDEX   0x01	
  #define VER_INDEX              0x02
  #define TAG1_INDEX             0x03
  #define TAG_LEN                0x10	 
  #define TAG0                   0xAA554257	 
  #define TAG1                   0x63594257	 	
  #define END_TAG                0x4257AA55	

  /* Version Number */  
  #define ISP_VERSION         0x20170613
  #define FIRMWARE_VERSION01  0x20170614
  #define FIRMWARE_VERSION02  0x20170615

- The Tag is located at a specified offset which is defined in define.h (ISP_Demo root folder). Developer can modify and rebuild all projects to get new binary files.
  #define ISP_TAG_OFFSET         0x00100          /*  256B */ 	 
  #define ISP_ENDTAG_OFFSET      0x05000          /*  20KB - The End Tag is added by Post-Build*/	 
  #define FIRMWARE_TAG_OFFSET    0x00100          /*  256B */
  #define FIRMWARE_ENDTAG_OFFSET 0x80000          /* 512KB - The End Tag is added by Post-Build*/	 
	
[Note 2]
- Support Windows Only
- The file name for ISP update is defined in define.h (ISP_Demo root folder).
- The maximum file name is 39 characters.
- Developer can define entired file name or file name prefix.
  - The default definition is "Update" (File name must have file name prefix "Update". For example, "UpdateApp.bin").
    - Developer also can define it to "Update.bin". (File name must be "Update.bin").
  - Case Sensitive 

   /* Update File Name Length (include Filename Extension) */
   #define FILE_NAME_LENGTH    39

   /* Update File Name (0:ignore this character) - Windows Only */
   static uint8_t u8FileName[FILE_NAME_LENGTH] = {
    'U', 'p', 'd', 'a', 't', 'e',  0,   0,    0,   0,
      0,   0,   0,   0,   0,   0,  0,   0,    0,   0,
      0,   0,   0,   0,   0,   0,  0,   0,    0,   0,
      0,   0,   0,   0,   0,   0,  0,   0,    0
   };

[Note 3]
- Loader uses post-build to merge the binary files. If Developer want to modify the address of the firmwares, please remember to modify the command file - Post-build\Merge.cmd
  - Execute file for padding
    - Pad.exe Command format 
       - Usage : [Converter] [Pattern 0/1] [Pad Size (KB)] [Input/Output File]
       - Usage : [Converter] [Pattern 0/1] [Pad Size (KB)] [Input File] [Output File]

[Note 4]
- Cipher Function Enable Condition
  - Burn MTP to assign MTP Signature - MTP_SIG and Offset - MTP_OFFSET. 

- After MTP is burned, the code on SPI Flash offset (MTP_SIG) must be MTP_SIG. Please make sure the definition is the same as the MTP data.
   /* MTP Setting */   
   /* If you want to encrypt binary firmware, you can enable
      the session code and assign signature and offset here */
   #define MTP_SIG	    (0x20040929)
   #define MTP_OFFSET       (0x130)       /* Less than 16KB */

- Binary file for End-User update when Cipher is enabled.
  - If End-User uses encrypted binary file, please disable Cipher in define.h (ISP_Demo root folder). Otherwises, please enable Cipher function.
   /* Cipher function for ISP mode */
   //#define DISABLE_CIPHER

- The way to get encrypted binary file
  - Burn the code to specified address (0x10000 for ISP demo) NUC505 with MTP burned by ICP and read bakc from NUC505. The read back is encrypted file. 

[Demo Test Log]
- No Firmware Log
  - No App Firmware: Jump to ISP mode

   NUC505 Loader
   No Firmware!!
   ISP Firmware
     ISP FW Version : 0x20170613
   ISP - 20170613

- Default Firmware
  - Jump to default firmware
  - User can enter to ISP mode by UART console.

   NUC505 Loader
   Jump to Firmware!!
   +----------------------------------------+
   |         NUC505 ISP Sample Code         |
   |              APP Default               |
   +----------------------------------------+
   Version 20170614
   Press any key to ISP mode
   NUC505 Loader
   ISP
     ISP FW Version : 0x20170613
   ISP - 20170613

- New Firmware
  - Jump to default firmware
  - User can enter to ISP mode by UART console.

   NUC505 Loader
   Jump to Firmware!!
   +----------------------------------------+
   |         NUC505 ISP Sample Code         |
   |              APP Ver.02                |
   +----------------------------------------+
   Version 20170615
   Press any key to ISP mode
   NUC505 Loader
   ISP
     ISP FW Version : 0x20170613
   ISP - 20170613


- ISP Mode
  - Current Version is "App Ver.02" (UpdateApp.bin).
  - Enter to ISP mode by UART console.
  - Copy UpdateDefaultApp.bin to the disk
  - Current Version is "App Default" (UpdateDefaultApp).

   NUC505 Loader
   Jump to Firmware!!
   +----------------------------------------+
   |         NUC505 ISP Sample Code         |
   |              APP Ver.02                |
   +----------------------------------------+
   Version 20170615
   Press any key to ISP mode
   NUC505 Loader
   ISP
     ISP FW Version : 0x20170613
   ISP - 20170613
   Erase 0x90000 for EndTag
   File Name:UpdateDefaultApp.bin
   File Size: 6664B
   NUC505 Loader
   Jump to Firmware!!
   +----------------------------------------+
   |         NUC505 ISP Sample Code         |
   |              APP Default               |
   +----------------------------------------+
   Version 20170614
   Press any key to ISP mode

