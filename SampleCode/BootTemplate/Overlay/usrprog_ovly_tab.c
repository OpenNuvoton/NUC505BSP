/**************************************************************************//**
 * @file        usrprog_ovly_tab.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       User program overlay table
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "usrprog_ovly_tab.h"

/* Define overlay table, which must be consistent with your linker script file. */

/* FIXME: Define your overlays here. */
#if defined ( __CC_ARM )
DEFINE_OVERLAY(ER_OVERLAY_A, ovly_A, ovly_reg_1)
DEFINE_OVERLAY(ER_OVERLAY_B, ovly_B, ovly_reg_1)
DEFINE_OVERLAY(ER_OVERLAY_C, ovly_C, ovly_reg_2)
DEFINE_OVERLAY(ER_OVERLAY_D, ovly_D, ovly_reg_2)
DEFINE_OVERLAY(ER_OVERLAY_E, ovly_E, ovly_reg_2)
DEFINE_OVERLAY(ER_OVERLAY_F, ovly_F, ovly_reg_3)

#elif defined (__ICCARM__)
#pragma section = "overlay_a"
#pragma section = "overlay_a_init"
#pragma section = "overlay_b"
#pragma section = "overlay_b_init"
#pragma section = "overlay_c"
#pragma section = "overlay_c_init"
#pragma section = "overlay_d"
#pragma section = "overlay_d_init"
#pragma section = "overlay_e"
#pragma section = "overlay_e_init"
#pragma section = "overlay_f"
#pragma section = "overlay_f_init"
DEFINE_OVERLAY(overlay_a, ovly_A, ovly_reg_1)
DEFINE_OVERLAY(overlay_b, ovly_B, ovly_reg_1)
DEFINE_OVERLAY(overlay_c, ovly_C, ovly_reg_2)
DEFINE_OVERLAY(overlay_d, ovly_D, ovly_reg_2)
DEFINE_OVERLAY(overlay_e, ovly_E, ovly_reg_2)
DEFINE_OVERLAY(overlay_f, ovly_F, ovly_reg_3)

#endif

/* FIXME: Define your overlay regions here. */
DEFINE_2OVERLAY_REGION(ovly_reg_1, ovly_A, ovly_B)
DEFINE_3OVERLAY_REGION(ovly_reg_2, ovly_C, ovly_D, ovly_E)
DEFINE_1OVERLAY_REGION(ovly_reg_3, ovly_F)
