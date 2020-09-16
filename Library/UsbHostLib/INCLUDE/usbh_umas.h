/**************************************************************************//**
 * @file     usbh_umas.h
 * @version  V1.00
 * $Revision 2 $
 * $Date: 14/10/07 5:46p $
 * @brief   USB Host Mass Storage driver header file
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef _USBH_UMAS_H_
#define _USBH_UMAS_H_

#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup NUC505_Device_Driver NUC505 Device Driver
  @{
*/

/** @addtogroup NUC505_USBH_MASS_Driver USB Host Mass Storage Driver
  @{
*/

/** @addtogroup NUC505_USBH_MASS_EXPORTED_STRUCTURES USB Host Mass Storage Driver Exported Structures
  @{
*/

typedef struct                         /*! USB Mass Storage disk                */
{
    USB_DEV_T     *pusb_dev;           /*!< This USB device.                    */
    uint16_t      vendor_id;           /*!< Vendor ID in device descriptor      */
    uint16_t      product_id;          /*!< Product ID in device descriptor     */
    uint32_t      sector_size;         /*!< Bytes per sector                    */
    uint32_t      sector_number;       /*!< Total number of sectors             */
} mass_disk_t;                         /*! USB Mass Storage disk                */

/*@}*/ /* end of group NUC505_USBH_MASS_EXPORTED_STRUCTURES */


/// @cond HIDDEN_SYMBOLS

/*----------------------------
 * APIs for FATFS
 *----------------------------*/
extern int usbh_umas_disk_status(void);
extern DRESULT usbh_umas_ioctl(int cmd, void *buff);
extern DRESULT usbh_umas_read(uint8_t *buff, uint32_t sector_no, int number_of_sector);
extern DRESULT usbh_umas_write(uint8_t *buff, uint32_t sector_no, int number_of_sector);

/// @endcond HIDDEN_SYMBOLS



/** @addtogroup NUC505_USBH_MASS_EXPORTED_FUNCTIONS USB Host Mass Storage Driver Exported Functions
  @{
*/

extern int  USBH_MassInit(void);
extern int  USBH_MassGetDiskList(mass_disk_t * dlist[], int max);
extern int  USBH_MassRawRead(mass_disk_t *disk, uint32_t sectorN, int32_t scnt, uint8_t *buff);
extern int  USBH_MassRawWrite(mass_disk_t *disk, uint32_t sectorN, int32_t scnt, uint8_t *buff);

/*@}*/ /* end of group NUC505_USBH_MASS_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC505_USBH_MASS_Driver */

/*@}*/ /* end of group NUC505_Device_Driver */


#ifdef __cplusplus
}
#endif

#endif

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/


