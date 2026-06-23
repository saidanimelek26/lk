/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _SHOW_ANIMATION_COMMON_H
#define _SHOW_ANIMATION_COMMON_H

#include "show_logo_common.h"
#include "decompress_common.h"
#include "cust_display.h"

// ============================================================
// MISSING DEFINITIONS ADDED HERE
// ============================================================

#ifndef LCM_SCREEN_T_DEFINED
#define LCM_SCREEN_T_DEFINED

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int fb_size;
    unsigned int fill_dst_bits;
    unsigned int needAllign;
    unsigned int allignWidth;
    unsigned int need180Adjust;
    unsigned int rotation;
} LCM_SCREEN_T;

#endif

#ifndef RECT_REGION_T_DEFINED
#define RECT_REGION_T_DEFINED

typedef struct {
    unsigned int left;
    unsigned int top;
    unsigned int right;
    unsigned int bottom;
} RECT_REGION_T;

#endif

/* Default values for animation regions (if not defined in cust_display.h) */
#ifndef BAR_LEFT
#define BAR_LEFT        0
#endif

#ifndef BAR_TOP
#define BAR_TOP         0
#endif

#ifndef BAR_RIGHT
#define BAR_RIGHT       100
#endif

#ifndef BAR_BOTTOM
#define BAR_BOTTOM      100
#endif

#ifndef PERCENT_LEFT
#define PERCENT_LEFT    0
#endif

#ifndef PERCENT_TOP
#define PERCENT_TOP     0
#endif

#ifndef PERCENT_RIGHT
#define PERCENT_RIGHT   100
#endif

#ifndef PERCENT_BOTTOM
#define PERCENT_BOTTOM  100
#endif

#ifndef NUMBER_LEFT
#define NUMBER_LEFT     0
#endif

#ifndef NUMBER_TOP
#define NUMBER_TOP      0
#endif

#ifndef NUMBER_RIGHT
#define NUMBER_RIGHT    100
#endif

#ifndef NUMBER_BOTTOM
#define NUMBER_BOTTOM   100
#endif

#ifndef TOP_ANIMATION_LEFT
#define TOP_ANIMATION_LEFT   0
#endif

#ifndef TOP_ANIMATION_TOP
#define TOP_ANIMATION_TOP    0
#endif

#ifndef TOP_ANIMATION_RIGHT
#define TOP_ANIMATION_RIGHT  100
#endif

#ifndef TOP_ANIMATION_BOTTOM
#define TOP_ANIMATION_BOTTOM 100
#endif

// ============================================================

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int logonum;
    unsigned int logolen;
    unsigned int inaddr;
} LOGO_PARA_T;

#define VERION_OLD_ANIMATION 0
#define VERION_NEW_ANIMATION 1
#define VERION_WIRELESS_CHARGING_ANIMATION  2

// version_0 animation
#define ANIM_V0_REGIONS  4
#define ANIM_V0_SPACE_AFTER_REGION  4

static const RECT_REGION_T bar_rect = {BAR_LEFT, BAR_TOP, BAR_RIGHT, BAR_BOTTOM};
static const RECT_REGION_T percent_location_rect = {PERCENT_LEFT,PERCENT_TOP,PERCENT_RIGHT,PERCENT_BOTTOM};

// dynamic animation logo paramter
//const int top_animation_height = TOP_ANIMATION_BOTTOM - TOP_ANIMATION_TOP;
// num parameter
static const int number_pic_size = (NUMBER_RIGHT - NUMBER_LEFT)*(NUMBER_BOTTOM - NUMBER_TOP)*4;   //size
// line parameter
static const int line_pic_size = (TOP_ANIMATION_RIGHT - TOP_ANIMATION_LEFT)*4;

void fill_animation_logo(unsigned int index, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen);
void fill_animation_prog_bar(RECT_REGION_T rect_bar,
                       unsigned int fgColor,
                       unsigned int start_div, unsigned int occupied_div,
                        void *fill_addr, LCM_SCREEN_T phical_screen);

void fill_animation_dynamic(unsigned int index, RECT_REGION_T rect, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen);
void fill_animation_number(unsigned int index, unsigned int number_position, void *fill_addr,  void * logo_addr, LCM_SCREEN_T phical_screen);
void fill_animation_line(unsigned int index, unsigned int capacity_grids, void *fill_addr,  void * logo_addr, LCM_SCREEN_T phical_screen);

void fill_animation_battery_old(unsigned int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen);                               
void fill_animation_battery_new(unsigned int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen);

/* public interface function     */
void fill_animation_battery_by_ver(unsigned int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr,
                        LCM_SCREEN_T phical_screen, int version);

#ifdef __cplusplus
}
#endif

#endif /* _SHOW_ANIMATION_COMMON_H */
