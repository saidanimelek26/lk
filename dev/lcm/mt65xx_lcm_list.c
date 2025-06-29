/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/
#include <lcm_drv.h>
#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>
#else
#include <linux/delay.h>
#include <mach/mt_gpio.h>
#endif

/* used to identify float ID PIN status */
#define LCD_HW_ID_STATUS_LOW      0
#define LCD_HW_ID_STATUS_HIGH     1
#define LCD_HW_ID_STATUS_FLOAT 0x02
#define LCD_HW_ID_STATUS_ERROR  0x03

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL, fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

extern LCM_DRIVER hct_otm1285a_dsi_vdo_hd_boe;
extern LCM_DRIVER s6d78a0_qhd_dsi_vdo;
extern LCM_DRIVER hct_ili9881_dsi_vdo_hd_cpt;
extern LCM_DRIVER hct_hx8394f_dsi_vdo_hd_cmi;
extern LCM_DRIVER hct_otm1282_dsi_vdo_hd_auo;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_cpt;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_tm_50_xld;
extern LCM_DRIVER hct_nt35521s_dsi_vdo_hd_boe_50_xld;
extern LCM_DRIVER hct_hx8394d_dsi_vdo_hd_cmi;
extern LCM_DRIVER gc9503p_fwp_dsi_vdo_jt_ivo_ba2_lcm_drv;;

LCM_DRIVER *lcm_driver_list[] = {
#if defined(GC9503P_FWP_DSI_VDO_JT_IVO_BA2)
   &gc9503p_fwp_dsi_vdo_jt_ivo_ba2_lcm_drv,
#endif

#if defined(S6D78A0_QHD_DSI_VDO)
   &s6d78a0_qhd_dsi_vdo,
#endif

#if defined(HCT_ILI9881_DSI_VDO_HD_CPT)
       &hct_ili9881_dsi_vdo_hd_cpt,
#endif

#if defined(HCT_HX8394F_DSI_VDO_HD_CMI)
       &hct_hx8394f_dsi_vdo_hd_cmi,
#endif

#if defined(HCT_OTM1282_DSI_VDO_HD_AUO)
       &hct_otm1282_dsi_vdo_hd_auo,
#endif

#if defined(HCT_RM68200_DSI_VDO_HD_CPT)
       &hct_rm68200_dsi_vdo_hd_cpt,
#endif

#if defined(HCT_RM68200_DSI_VDO_HD_TM_50_XLD)
       &hct_rm68200_dsi_vdo_hd_tm_50_xld,
#endif

#if defined(HCT_NT35521S_DSI_VDO_HD_BOE_50_XLD)
       &hct_nt35521s_dsi_vdo_hd_boe_50_xld,
#endif

#if defined(HCT_HX8394D_DSI_VDO_HD_CMI)
       &hct_hx8394d_dsi_vdo_hd_cmi,
#endif
};

#define LCM_COMPILE_ASSERT(condition) LCM_COMPILE_ASSERT_X(condition, __LINE__)
#define LCM_COMPILE_ASSERT_X(condition, line) LCM_COMPILE_ASSERT_XX(condition, line)
#define LCM_COMPILE_ASSERT_XX(condition, line) char assertion_failed_at_line_##line[(condition)?1 :  -1]

unsigned int lcm_count = sizeof(lcm_driver_list) / sizeof(LCM_DRIVER *);
LCM_COMPILE_ASSERT(0 != sizeof(lcm_driver_list) / sizeof(LCM_DRIVER *));
#if defined(NT35520_HD720_DSI_CMD_TM) | defined(NT35520_HD720_DSI_CMD_BOE) | defined(NT35521_HD720_DSI_VDO_BOE) | defined(NT35521_HD720_DSI_VIDEO_TM)
#ifdef BUILD_LK
extern void mdelay(unsigned long msec);
#endif
static unsigned char lcd_id_pins_value = 0xFF;


/******************************************************************************
Function:       which_lcd_module_triple
  Description:    read LCD ID PIN status,could identify three status:highlowfloat
  Input:           none
  Output:         none
  Return:         LCD ID1|ID0 value
  Others:
******************************************************************************/
unsigned char which_lcd_module_triple(void)
{
	unsigned char high_read0 = 0;
	unsigned char low_read0 = 0;
	unsigned char high_read1 = 0;
	unsigned char low_read1 = 0;
	unsigned char lcd_id0 = 0;
	unsigned char lcd_id1 = 0;
	unsigned char lcd_id = 0;
	/* Solve Coverity scan warning : check return value */
	unsigned int ret = 0;
	/* only recognise once */
	if (0xFF != lcd_id_pins_value) {
		return lcd_id_pins_value;
	}
	/* Solve Coverity scan warning : check return value */
	ret = mt_set_gpio_mode(GPIO_DISP_ID0_PIN, GPIO_MODE_00);
	if (0 != ret) {
		LCD_DEBUG("ID0 mt_set_gpio_mode fail\n");
	}
	ret = mt_set_gpio_dir(GPIO_DISP_ID0_PIN, GPIO_DIR_IN);
	if (0 != ret) {
		LCD_DEBUG("ID0 mt_set_gpio_dir fail\n");
	}
	ret = mt_set_gpio_pull_enable(GPIO_DISP_ID0_PIN, GPIO_PULL_ENABLE);
	if (0 != ret) {
		LCD_DEBUG("ID0 mt_set_gpio_pull_enable fail\n");
	}
	ret = mt_set_gpio_mode(GPIO_DISP_ID1_PIN, GPIO_MODE_00);
	if (0 != ret) {
		LCD_DEBUG("ID1 mt_set_gpio_mode fail\n");
	}
	ret = mt_set_gpio_dir(GPIO_DISP_ID1_PIN, GPIO_DIR_IN);
	if (0 != ret) {
		LCD_DEBUG("ID1 mt_set_gpio_dir fail\n");
	}
	ret = mt_set_gpio_pull_enable(GPIO_DISP_ID1_PIN, GPIO_PULL_ENABLE);
	if (0 != ret) {
		LCD_DEBUG("ID1 mt_set_gpio_pull_enable fail\n");
	}
	/* pull down ID0 ID1 PIN */
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
	if (0 != ret) {
		LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");
	}
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
	if (0 != ret) {
		LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");
	}
	/* delay 100ms , for discharging capacitance */
	mdelay(100);
	/* get ID0 ID1 status */
	low_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	low_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);
	/* pull up ID0 ID1 PIN */
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_UP);
	if (0 != ret) {
		LCD_DEBUG("ID0 mt_set_gpio_pull_select->UP fail\n");
	}
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_UP);
	if (0 != ret) {
		LCD_DEBUG("ID1 mt_set_gpio_pull_select->UP fail\n");
	}
	/* delay 100ms , for charging capacitance */
	mdelay(100);
	/* get ID0 ID1 status */
	high_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	high_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);

	if (low_read0 != high_read0) {
		/*float status , pull down ID0 ,to prevent electric leakage */
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
		if (0 != ret) {
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");
		}
		lcd_id0 = LCD_HW_ID_STATUS_FLOAT;
	} else if ((LCD_HW_ID_STATUS_LOW == low_read0) && (LCD_HW_ID_STATUS_LOW == high_read0)) {
		/*low status , pull down ID0 ,to prevent electric leakage */
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
		if (0 != ret) {
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");
		}
		lcd_id0 = LCD_HW_ID_STATUS_LOW;
	} else if ((LCD_HW_ID_STATUS_HIGH == low_read0) && (LCD_HW_ID_STATUS_HIGH == high_read0)) {
		/*high status , pull up ID0 ,to prevent electric leakage */
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_UP);
		if (0 != ret) {
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->UP fail\n");
		}
		lcd_id0 = LCD_HW_ID_STATUS_HIGH;
	} else {
		LCD_DEBUG(" Read LCD_id0 error\n");
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DISABLE);
		if (0 != ret) {
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Disbale fail\n");
		}
		lcd_id0 = LCD_HW_ID_STATUS_ERROR;
	}


	if (low_read1 != high_read1) {
		/*float status , pull down ID1 ,to prevent electric leakage */
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
		if (0 != ret) {
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");
		}
		lcd_id1 = LCD_HW_ID_STATUS_FLOAT;
	} else if ((LCD_HW_ID_STATUS_LOW == low_read1) && (LCD_HW_ID_STATUS_LOW == high_read1)) {
		/*low status , pull down ID1 ,to prevent electric leakage */
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
		if (0 != ret) {
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");
		}
		lcd_id1 = LCD_HW_ID_STATUS_LOW;
	} else if ((LCD_HW_ID_STATUS_HIGH == low_read1) && (LCD_HW_ID_STATUS_HIGH == high_read1)) {
		/*high status , pull up ID1 ,to prevent electric leakage */
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_UP);
		if (0 != ret) {
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->UP fail\n");
		}
		lcd_id1 = LCD_HW_ID_STATUS_HIGH;
	} else {

		LCD_DEBUG(" Read LCD_id1 error\n");
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DISABLE);
		if (0 != ret) {
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Disable fail\n");
		}
		lcd_id1 = LCD_HW_ID_STATUS_ERROR;
	}
#ifdef BUILD_LK
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id0:%d\n", lcd_id0);
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id1:%d\n", lcd_id1);
#else
	printk("which_lcd_module_triple,lcd_id0:%d\n", lcd_id0);
	printk("which_lcd_module_triple,lcd_id1:%d\n", lcd_id1);
#endif
	lcd_id = lcd_id0 | (lcd_id1 << 2);

#ifdef BUILD_LK
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id:%d\n", lcd_id);
#else
	printk("which_lcd_module_triple,lcd_id:%d\n", lcd_id);
#endif

	lcd_id_pins_value = lcd_id;
	return lcd_id;
}
#endif
