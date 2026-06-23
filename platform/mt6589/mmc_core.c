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

/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <printf.h>
#include <malloc.h>
#include <platform/mmc_core.h>
#include <debug.h>

#include "msdc.h"

/* ======================================================================
 * MT6589 COMPATIBILITY SHIMS
 * The new mmc_core.c was written for a newer platform. These shims
 * bridge it to the older mt6589 headers without modifying those headers.
 * ====================================================================== */

// ============================================================
// MSDC DEFINITIONS (if missing)
// ============================================================

#ifndef MSDC_CFG
#define MSDC_CFG        0
#endif

#ifndef MSDC_CFG_PIO
#define MSDC_CFG_PIO    (1 << 0)
#endif

/* BUG_ON / WARN_ON */
#ifndef BUG_ON
#define BUG_ON(cond) \
    do { if (cond) { printf("BUG_ON: %s:%d\n", __func__, __LINE__); while (1); } } while (0)
#endif
#ifndef WARN_ON
#define WARN_ON(cond) \
    do { if (cond) { printf("WARN_ON: %s:%d\n", __func__, __LINE__); } } while (0)
#endif

/* MSG log-level tokens — mt6589 dprintf uses ALWAYS/INFO/SPEW */
#ifndef ERR
#define ERR   ALWAYS
#endif
#ifndef INF
#define INF   INFO
#endif
#ifndef WRN
#define WRN   ALWAYS
#endif
#ifndef OPS
#define OPS   SPEW
#endif

/* Command timing defaults */
#ifndef CMD_RETRIES
#define CMD_RETRIES  3
#endif
#ifndef CMD_TIMEOUT
#define CMD_TIMEOUT  100
#endif

/* MMC error codes that may be missing in old headers */
#ifndef MMC_ERR_ERASE_SEQ
#define MMC_ERR_ERASE_SEQ  12
#endif

/* R1 card status bits */
#ifndef R1_READY_FOR_DATA
#define R1_READY_FOR_DATA   (1U << 8)
#endif
#ifndef R1_SWITCH_ERROR
#define R1_SWITCH_ERROR     (1U << 7)
#endif
#ifndef R1_APP_CMD
#define R1_APP_CMD          (1U << 5)
#endif
#ifndef R1_STATUS
#define R1_STATUS(x)        ((x) & 0xFFF9A000U)
#endif
#ifndef R1_CURRENT_STATE
static inline unsigned int _r1_current_state(unsigned int x) { return (x >> 9) & 0xF; }
#define R1_CURRENT_STATE(x) _r1_current_state(x)
#endif

/* CCC (card command class) bits */
#ifndef CCC_IO_MODE
#define CCC_IO_MODE    (1 << 9)
#endif
#ifndef CCC_ERASE
#define CCC_ERASE      (1 << 5)
#endif
#ifndef CCC_WRITE_PROT
#define CCC_WRITE_PROT (1 << 6)
#endif
#ifndef CCC_SWITCH
#define CCC_SWITCH     (1 << 10)
#endif

/* EXT_CSD card-type bits added in newer eMMC specs */
#ifndef EXT_CSD_CARD_TYPE_HS200_1_8V
#define EXT_CSD_CARD_TYPE_HS200_1_8V  0x10
#endif
#ifndef EXT_CSD_CARD_TYPE_HS200_1_2V
#define EXT_CSD_CARD_TYPE_HS200_1_2V  0x20
#endif
#ifndef EXT_CSD_CARD_TYPE_HS400_1_8V
#define EXT_CSD_CARD_TYPE_HS400_1_8V  0x40
#endif
#ifndef EXT_CSD_CARD_TYPE_HS400_1_2V
#define EXT_CSD_CARD_TYPE_HS400_1_2V  0x80
#endif

/* EXT_CSD HS timing value for HS400 */
#ifndef EXT_CSD_HS_TIMEING_HS400
#define EXT_CSD_HS_TIMEING_HS400  3
#endif

/* MMC capability flags that may be absent in old headers */
#ifndef MMC_CAP_EMMC_HS200
#define MMC_CAP_EMMC_HS200  0
#endif
#ifndef MMC_CAP_EMMC_HS400
#define MMC_CAP_EMMC_HS400  0
#endif

/* SD / eMMC version tokens */
#ifndef SD_VER_10
#define SD_VER_10  1
#endif
#ifndef SD_VER_20
#define SD_VER_20  2
#endif
#ifndef SD_VER_30
#define SD_VER_30  3
#endif
#ifndef EMMC_VER_42
#define EMMC_VER_42  42
#endif
#ifndef EMMC_VER_43
#define EMMC_VER_43  43
#endif
#ifndef EMMC_VER_44
#define EMMC_VER_44  44
#endif
#ifndef EMMC_VER_45
#define EMMC_VER_45  45
#endif
#ifndef EMMC_VER_50
#define EMMC_VER_50  50
#endif

/*
 * uffs() — "find first set bit" (1-based, 0 if input is 0).
 * Provided as a static inline so we never clash with a platform version.
 */
static inline int _mmc_uffs(unsigned int x)
{
    int r = 1;
    if (!x) return 0;
    while (!(x & 1)) { x >>= 1; r++; }
    return r;
}
#define uffs(x)  _mmc_uffs(x)

/*
 * msdc_hard_reset — may not exist on mt6589; provide a weak stub.
 * If the platform does supply it, the linker will use that version.
 */
void __attribute__((weak)) msdc_hard_reset(void) { }

/* ======================================================================
 * DEFINITIONS MISSING IN NEW LK (from old file)
 * ====================================================================== */

#define NR_MMC             (MSDC_MAX_NUM)
#define PART_MAX_COUNT     32
#define PART_SIZE_BMTPOOL  32
#define PART_SIZE_OTP      0

/* ======================================================================
 * TYPE DEFINITIONS (from old file)
 * ====================================================================== */

typedef unsigned char uchar;
typedef unsigned long long u64;
typedef unsigned long ulong;

/* Block device descriptor */
typedef struct {
    int dev;
    unsigned long blksz;
    unsigned long lba;
    unsigned long (*block_read)(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *dst);
    unsigned long (*block_write)(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *src);
} block_dev_desc_t;

/* Partition device */
typedef struct {
    int id;
    int init;
    void *blkdev;   /* block_dev_desc_t * */
    int (*init_dev)(int id);
} part_dev_t;

/* Address translation structures */
typedef struct {
    u32 id;
    u64 len;
} addr_trans_info_t;

typedef struct {
    u32 num;
    addr_trans_info_t *info;
} addr_trans_tbl_t;

/* ======================================================================
 * EXTERNAL DECLARATIONS
 * ====================================================================== */

extern int mt_part_register_device(part_dev_t *dev);
extern int target_is_emmc_boot(void);

/* ======================================================================
 * STATIC VARIABLES (from old file)
 * ====================================================================== */

static struct mmc_host sd_host[NR_MMC];
static struct mmc_card sd_card[NR_MMC];
static block_dev_desc_t sd_dev[NR_MMC];
static int boot_dev_found = 0;
static part_dev_t boot_dev;

/* ======================================================================
 * ADDRESS TRANSLATION (stub — no real translation needed here)
 * ====================================================================== */

static int mmc_virt_to_phys(u32 virt_blknr, u32 *phys_blknr, u32 *part_id)
{
    *phys_blknr = virt_blknr;
    *part_id = 0;
    return 0;
}

static int mmc_phys_to_virt(u32 phys_blknr, u32 part_id, u32 *virt_blknr)
{
    *virt_blknr = phys_blknr;
    return 0;
}

static void mmc_addr_trans_tbl_init(void)
{
    return;
}

/* ======================================================================
 * WRAP FUNCTIONS
 * ====================================================================== */

unsigned long mmc_wrap_bread(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *dst)
{
    return mmc_block_read(dev_num, blknr, blkcnt, dst) == MMC_ERR_NONE
           ? blkcnt : (unsigned long)-1;
}

unsigned long mmc_wrap_bwrite(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *src)
{
    return mmc_block_write(dev_num, blknr, blkcnt, src) == MMC_ERR_NONE
           ? blkcnt : (unsigned long)-1;
}

/* ======================================================================
 * mmc_legacy_init
 * ====================================================================== */

int mmc_legacy_init(int verbose)
{
    int id = verbose - 1;
    int err = MMC_ERR_NONE;
    struct mmc_host *host;
    struct mmc_card *card;
    block_dev_desc_t *bdev;

    BUG_ON(id >= NR_MMC);

    msdc_hard_reset();

    host = &sd_host[id];
    card = &sd_card[id];
    bdev = &sd_dev[id];

    /* mt6589: mmc_init_host takes (host, id) only */
    err = mmc_init_host(host, id);

    if (err == MMC_ERR_NONE) {
        err = mmc_init_card(host, card);
    }

    if (err == MMC_ERR_NONE && !boot_dev_found) {
        bdev->dev        = id;
        bdev->blksz      = MMC_BLOCK_SIZE;
        bdev->lba        = card->nblks * card->blklen / MMC_BLOCK_SIZE;
        bdev->block_read  = mmc_wrap_bread;
        bdev->block_write = mmc_wrap_bwrite;

        boot_dev.id      = id;
        boot_dev.init    = 1;
        boot_dev.blkdev  = bdev;

        mt_part_register_device(&boot_dev);
        boot_dev_found = 1;
        printf("[SD%d] boot device found\n", id);
    }

    mmc_addr_trans_tbl_init();

    return err;
}

/* ======================================================================
 * LOOKUP TABLES
 * ====================================================================== */

static const unsigned int tran_exp[] = {
    10000, 100000, 1000000, 10000000,
    0, 0, 0, 0
};

static const unsigned char tran_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static const unsigned char mmc_tran_mant[] = {
    0,  10, 12, 13, 15, 20, 26, 30,
    35, 40, 45, 52, 55, 60, 70, 80,
};

static const unsigned int tacc_exp[] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

/* ======================================================================
 * BIT EXTRACTION HELPER
 * ====================================================================== */

static u32 unstuff_bits(u32 *resp, u32 start, u32 size)
{
    const u32 __mask = (1 << (size)) - 1;
    const int __off  = 3 - ((start) / 32);
    const int __shft = (start) & 31;
    u32 __res;

    __res = resp[__off] >> __shft;
    if ((size) + __shft >= 32)
        __res |= resp[__off - 1] << (32 - __shft);
    return __res & __mask;
}

/* ======================================================================
 * PROFILING (compiled only when MMC_PROFILING is defined)
 * ====================================================================== */

#ifdef MMC_PROFILING
static void mmc_prof_card_init(void *data, ulong id, ulong counts)
{
    int err = (int)data;
    if (!err) {
        MSG(ERR, "[SD%d] Init Card, %d counts, %d us\n",
            id, counts, counts * 30 + counts * 16960 / 32768);
    }
}

static void mmc_prof_read(void *data, ulong id, ulong counts)
{
    struct mmc_op_perf *perf = (struct mmc_op_perf *)data;
    struct mmc_op_report *rpt;
    u32 blksz  = perf->host->blklen;
    u32 blkcnt = (u32)id;

    rpt = (blkcnt > 1) ? &perf->multi_blks_read : &perf->single_blk_read;

    rpt->count++;
    rpt->total_size += blkcnt * blksz;
    rpt->total_time += counts;
    if ((counts < rpt->min_time) || (rpt->min_time == 0)) rpt->min_time = counts;
    if ((counts > rpt->max_time) || (rpt->max_time == 0)) rpt->max_time = counts;

    MSG(INF, "[SD%d] Read %d bytes, %d counts, %d us, %d KB/s, Avg: %d KB/s\n",
        perf->host->id, blkcnt * blksz, counts,
        counts * 30 + counts * 16960 / 32768,
        blkcnt * blksz * 32 / (counts ? counts : 1),
        ((rpt->total_size / 1024) * 32768) / rpt->total_time);
}

static void mmc_prof_write(void *data, ulong id, ulong counts)
{
    struct mmc_op_perf *perf = (struct mmc_op_perf *)data;
    struct mmc_op_report *rpt;
    u32 blksz  = perf->host->blklen;
    u32 blkcnt = (u32)id;

    rpt = (blkcnt > 1) ? &perf->multi_blks_write : &perf->single_blk_write;

    rpt->count++;
    rpt->total_size += blkcnt * blksz;
    rpt->total_time += counts;
    if ((counts < rpt->min_time) || (rpt->min_time == 0)) rpt->min_time = counts;
    if ((counts > rpt->max_time) || (rpt->max_time == 0)) rpt->max_time = counts;

    MSG(INF, "[SD%d] Write %d bytes, %d counts, %d us, %d KB/s, Avg: %d KB/s\n",
        perf->host->id, blkcnt * blksz, counts,
        counts * 30 + counts * 16960 / 32768,
        blkcnt * blksz * 32 / (counts ? counts : 1),
        ((rpt->total_size / 1024) * 32768) / rpt->total_time);
}
#endif /* MMC_PROFILING */

/* Profiling stubs — used when MMC_PROFILING is not defined so call sites compile. */
#ifndef MMC_PROFILING
#define mmc_prof_init(id, host, card)       do {} while (0)
#define mmc_prof_start()                    do {} while (0)
#define mmc_prof_stop()                     do {} while (0)
#define mmc_prof_update(fn, a, b)           do {} while (0)
#define mmc_prof_handle(id)                 (NULL)
#define mmc_prof_read                       NULL
#define mmc_prof_write                      NULL
#define mmc_prof_card_init                  NULL
#endif

/* ======================================================================
 * DEBUG DUMP FUNCTIONS (compiled only when MMC_DEBUG is set)
 * ====================================================================== */

#if MMC_DEBUG
void mmc_dump_card_status(u32 card_status)
{
    msdc_dump_card_status(card_status);
}

static void mmc_dump_ocr_reg(u32 resp)
{
    msdc_dump_ocr_reg(resp);
}

static void mmc_dump_rca_resp(u32 resp)
{
    msdc_dump_rca_resp(resp);
}

static void mmc_dump_tuning_blk(u8 *buf)
{
    int i;
    for (i = 0; i < 16; i++) {
        MSG(INF, "[TBLK%d] %x%x%x%x%x%x%x%x\n", i,
            (buf[(i<<2)]     >> 4) & 0xF, buf[(i<<2)]     & 0xF,
            (buf[(i<<2)+1]   >> 4) & 0xF, buf[(i<<2)+1]   & 0xF,
            (buf[(i<<2)+2]   >> 4) & 0xF, buf[(i<<2)+2]   & 0xF,
            (buf[(i<<2)+3]   >> 4) & 0xF, buf[(i<<2)+3]   & 0xF);
    }
}

static void mmc_dump_csd(struct mmc_card *card)
{
    struct mmc_csd *csd = &card->csd;
    u32 *resp = card->raw_csd;
    int i;
    unsigned int csd_struct;
    static char *sd_csd_ver[]  = {"v1.0", "v2.0"};
    static char *mmc_csd_ver[] = {"v1.0", "v1.1", "v1.2", "Ver. in EXT_CSD"};
    static char *mmc_cmd_cls[] = {"basic", "stream read", "block read",
        "stream write", "block write", "erase", "write prot", "lock card",
        "app-spec", "I/O", "rsv.", "rsv."};
    static char *sd_cmd_cls[]  = {"basic", "rsv.", "block read",
        "rsv.", "block write", "erase", "write prot", "lock card",
        "app-spec", "I/O", "switch", "rsv."};

    if (mmc_card_sd(card)) {
        csd_struct = unstuff_bits(resp, 126, 2);
        MSG(INF, "[CSD] CSD %s\n", sd_csd_ver[csd_struct]);
        MSG(INF, "[CSD] TACC_NS: %d ns, TACC_CLKS: %d clks\n", csd->tacc_ns, csd->tacc_clks);
        if (csd_struct == 1)
            MSG(INF, "[CSD] Read/Write Blk Len = 512bytes\n");
        else
            MSG(INF, "[CSD] Read Blk Len = %d, Write Blk Len = %d\n",
                1 << csd->read_blkbits, 1 << csd->write_blkbits);
        MSG(INF, "[CSD] CMD Class:");
        for (i = 0; i < 12; i++)
            if ((csd->cmdclass >> i) & 0x1) MSG(INF, "'%s' ", sd_cmd_cls[i]);
        MSG(INF, "\n");
    } else {
        csd_struct = unstuff_bits(resp, 126, 2);
        MSG(INF, "[CSD] CSD %s\n", mmc_csd_ver[csd_struct]);
        MSG(INF, "[CSD] MMCA Spec v%d\n", csd->mmca_vsn);
        MSG(INF, "[CSD] TACC_NS: %d ns, TACC_CLKS: %d clks\n", csd->tacc_ns, csd->tacc_clks);
        MSG(INF, "[CSD] Read Blk Len = %d, Write Blk Len = %d\n",
            1 << csd->read_blkbits, 1 << csd->write_blkbits);
        MSG(INF, "[CSD] CMD Class:");
        for (i = 0; i < 12; i++)
            if ((csd->cmdclass >> i) & 0x1) MSG(INF, "'%s' ", mmc_cmd_cls[i]);
        MSG(INF, "\n");
    }
}

void mmc_dump_ext_csd(struct mmc_card *card)
{
    u8  *ext_csd = &card->raw_ext_csd[0];
    u32  tmp;
    char *rev[] = {"4.0","4.1","4.2","4.3","Obsolete","4.41","4.5","5.0","5.1"};

    MSG(INF, "===========================================================\n");
    MSG(INF, "[EXT_CSD] EXT_CSD rev.              : v1.%d (MMCv%s)\n",
        ext_csd[EXT_CSD_REV], rev[ext_csd[EXT_CSD_REV]]);
    MSG(INF, "[EXT_CSD] CSD struct rev.           : v1.%d\n", ext_csd[EXT_CSD_STRUCT]);
    MSG(INF, "[EXT_CSD] Supported command sets    : %xh\n", ext_csd[EXT_CSD_S_CMD_SET]);
    MSG(INF, "[EXT_CSD] HPI features              : %xh\n", ext_csd[EXT_CSD_HPI_FEATURE]);
    MSG(INF, "[EXT_CSD] BG operations support     : %xh\n", ext_csd[EXT_CSD_BKOPS_SUPP]);
    MSG(INF, "[EXT_CSD] BG operations status      : %xh\n", ext_csd[EXT_CSD_BKOPS_STATUS]);
    memcpy(&tmp, &ext_csd[EXT_CSD_CORRECT_PRG_SECTS_NUM], 4);
    MSG(INF, "[EXT_CSD] Correct prg. sectors      : %xh\n", tmp);
    MSG(INF, "[EXT_CSD] 1st init time after part. : %d ms\n",  ext_csd[EXT_CSD_INI_TIMEOUT_AP] * 100);
    MSG(INF, "[EXT_CSD] Min. write perf.(DDR,52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_DDR_W_8_52]);
    MSG(INF, "[EXT_CSD] Min. read perf. (DDR,52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_DDR_R_8_52]);
    MSG(INF, "[EXT_CSD] TRIM timeout: %d ms\n", ext_csd[EXT_CSD_TRIM_MULT] & 0xFF * 300);
    MSG(INF, "[EXT_CSD] Secure feature support: %xh\n", ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT]);
    MSG(INF, "[EXT_CSD] Secure erase timeout  : %d ms\n", 300 *
        ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * ext_csd[EXT_CSD_SEC_ERASE_MULT]);
    MSG(INF, "[EXT_CSD] Secure trim timeout   : %d ms\n", 300 *
        ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * ext_csd[EXT_CSD_SEC_TRIM_MULT]);
    MSG(INF, "[EXT_CSD] Access size           : %d bytes\n",   ext_csd[EXT_CSD_ACC_SIZE] * 512);
    MSG(INF, "[EXT_CSD] HC erase unit size    : %d kbytes\n",  ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512);
    MSG(INF, "[EXT_CSD] HC erase timeout      : %d ms\n",      ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * 300);
    MSG(INF, "[EXT_CSD] HC write prot grp size: %d kbytes\n",  512 *
        ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * ext_csd[EXT_CSD_HC_WP_GPR_SIZE]);
    MSG(INF, "[EXT_CSD] HC erase grp def.     : %xh\n",  ext_csd[EXT_CSD_ERASE_GRP_DEF]);
    MSG(INF, "[EXT_CSD] Reliable write sect count: %xh\n", ext_csd[EXT_CSD_REL_WR_SEC_C]);
    MSG(INF, "[EXT_CSD] Sleep current (VCC) : %xh\n",  ext_csd[EXT_CSD_S_C_VCC]);
    MSG(INF, "[EXT_CSD] Sleep current (VCCQ): %xh\n",  ext_csd[EXT_CSD_S_C_VCCQ]);
    MSG(INF, "[EXT_CSD] Sleep/awake timeout : %d ns\n",
        100 * (2 << ext_csd[EXT_CSD_S_A_TIMEOUT]));
    memcpy(&tmp, &ext_csd[EXT_CSD_SEC_CNT], 4);
    MSG(INF, "[EXT_CSD] Sector count : %xh\n", tmp);
    MSG(INF, "[EXT_CSD] Min. WR Perf.  (52MH,8b): %xh\n",  ext_csd[EXT_CSD_MIN_PERF_W_8_52]);
    MSG(INF, "[EXT_CSD] Min. Read Perf.(52MH,8b): %xh\n",  ext_csd[EXT_CSD_MIN_PERF_R_8_52]);
    MSG(INF, "[EXT_CSD] Min. WR Perf.  (26MH,8b,52MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_W_8_26_4_25]);
    MSG(INF, "[EXT_CSD] Min. Read Perf.(26MH,8b,52MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_R_8_26_4_25]);
    MSG(INF, "[EXT_CSD] Min. WR Perf.  (26MH,4b): %xh\n",  ext_csd[EXT_CSD_MIN_PERF_W_4_26]);
    MSG(INF, "[EXT_CSD] Min. Read Perf.(26MH,4b): %xh\n",  ext_csd[EXT_CSD_MIN_PERF_R_4_26]);
    MSG(INF, "[EXT_CSD] Power class: %x\n",  ext_csd[EXT_CSD_PWR_CLASS]);
    MSG(INF, "[EXT_CSD] Power class(DDR,52MH,3.6V): %xh\n", ext_csd[EXT_CSD_PWR_CL_DDR_52_360]);
    MSG(INF, "[EXT_CSD] Power class(DDR,52MH,1.9V): %xh\n", ext_csd[EXT_CSD_PWR_CL_DDR_52_195]);
    MSG(INF, "[EXT_CSD] Power class(26MH,3.6V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_26_360]);
    MSG(INF, "[EXT_CSD] Power class(52MH,3.6V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_52_360]);
    MSG(INF, "[EXT_CSD] Power class(26MH,1.9V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_26_195]);
    MSG(INF, "[EXT_CSD] Power class(52MH,1.9V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_52_195]);
    MSG(INF, "[EXT_CSD] Part. switch timing    : %xh\n", ext_csd[EXT_CSD_PART_SWITCH_TIME]);
    MSG(INF, "[EXT_CSD] Out-of-INTR busy timing: %xh\n", ext_csd[EXT_CSD_OUT_OF_INTR_TIME]);
    MSG(INF, "[EXT_CSD] Card type       : %xh\n", ext_csd[EXT_CSD_CARD_TYPE]);
    MSG(INF, "[EXT_CSD] Command set     : %xh\n", ext_csd[EXT_CSD_CMD_SET]);
    MSG(INF, "[EXT_CSD] Command set rev.: %xh\n", ext_csd[EXT_CSD_CMD_SET_REV]);
    MSG(INF, "[EXT_CSD] HS timing       : %xh\n", ext_csd[EXT_CSD_HS_TIMING]);
    MSG(INF, "[EXT_CSD] Bus width       : %xh\n", ext_csd[EXT_CSD_BUS_WIDTH]);
    MSG(INF, "[EXT_CSD] Erase memory content : %xh\n", ext_csd[EXT_CSD_ERASED_MEM_CONT]);
    MSG(INF, "[EXT_CSD] Partition config      : %xh\n", ext_csd[EXT_CSD_PART_CFG]);
    MSG(INF, "[EXT_CSD] Boot partition size   : %d kbytes\n", ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128);
    MSG(INF, "[EXT_CSD] Boot information      : %xh\n", ext_csd[EXT_CSD_BOOT_INFO]);
    MSG(INF, "[EXT_CSD] Boot config protection: %xh\n", ext_csd[EXT_CSD_BOOT_CONFIG_PROT]);
    MSG(INF, "[EXT_CSD] Boot bus width        : %xh\n", ext_csd[EXT_CSD_BOOT_BUS_WIDTH]);
    MSG(INF, "[EXT_CSD] Boot area write prot  : %xh\n", ext_csd[EXT_CSD_BOOT_WP]);
    MSG(INF, "[EXT_CSD] User area write prot  : %xh\n", ext_csd[EXT_CSD_USR_WP]);
    MSG(INF, "[EXT_CSD] FW configuration      : %xh\n", ext_csd[EXT_CSD_FW_CONFIG]);
    MSG(INF, "[EXT_CSD] RPMB size : %d kbytes\n", ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128);
    MSG(INF, "[EXT_CSD] Write rel. setting  : %xh\n", ext_csd[EXT_CSD_WR_REL_SET]);
    MSG(INF, "[EXT_CSD] Write rel. parameter: %xh\n", ext_csd[EXT_CSD_WR_REL_PARAM]);
    MSG(INF, "[EXT_CSD] Start background ops : %xh\n", ext_csd[EXT_CSD_BKOPS_START]);
    MSG(INF, "[EXT_CSD] Enable background ops: %xh\n", ext_csd[EXT_CSD_BKOPS_EN]);
    MSG(INF, "[EXT_CSD] H/W reset function   : %xh\n", ext_csd[EXT_CSD_RST_N_FUNC]);
    MSG(INF, "[EXT_CSD] HPI management       : %xh\n", ext_csd[EXT_CSD_HPI_MGMT]);
    memcpy(&tmp, &ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT], 4);
    MSG(INF, "[EXT_CSD] Max. enhanced area size : %xh (%d kbytes)\n",
        tmp & 0x00FFFFFF, (tmp & 0x00FFFFFF) * 512 *
        ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    MSG(INF, "[EXT_CSD] Part. support  : %xh\n", ext_csd[EXT_CSD_PART_SUPPORT]);
    MSG(INF, "[EXT_CSD] Part. attribute: %xh\n", ext_csd[EXT_CSD_PART_ATTR]);
    MSG(INF, "[EXT_CSD] Part. setting  : %xh\n", ext_csd[EXT_CSD_PART_SET_COMPL]);
    MSG(INF, "[EXT_CSD] General purpose 1 size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_GP1_SIZE_MULT+0] | ext_csd[EXT_CSD_GP1_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_GP1_SIZE_MULT+2]<<16),
        (ext_csd[EXT_CSD_GP1_SIZE_MULT+0] | ext_csd[EXT_CSD_GP1_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_GP1_SIZE_MULT+2]<<16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    MSG(INF, "[EXT_CSD] General purpose 2 size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_GP2_SIZE_MULT+0] | ext_csd[EXT_CSD_GP2_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_GP2_SIZE_MULT+2]<<16),
        (ext_csd[EXT_CSD_GP2_SIZE_MULT+0] | ext_csd[EXT_CSD_GP2_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_GP2_SIZE_MULT+2]<<16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    MSG(INF, "[EXT_CSD] General purpose 3 size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_GP3_SIZE_MULT+0] | ext_csd[EXT_CSD_GP3_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_GP3_SIZE_MULT+2]<<16),
        (ext_csd[EXT_CSD_GP3_SIZE_MULT+0] | ext_csd[EXT_CSD_GP3_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_GP3_SIZE_MULT+2]<<16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    MSG(INF, "[EXT_CSD] General purpose 4 size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_GP4_SIZE_MULT+0] | ext_csd[EXT_CSD_GP4_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_GP4_SIZE_MULT+2]<<16),
        (ext_csd[EXT_CSD_GP4_SIZE_MULT+0] | ext_csd[EXT_CSD_GP4_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_GP4_SIZE_MULT+2]<<16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    MSG(INF, "[EXT_CSD] Enh. user area size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_ENH_SIZE_MULT+0] | ext_csd[EXT_CSD_ENH_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_ENH_SIZE_MULT+2]<<16),
        (ext_csd[EXT_CSD_ENH_SIZE_MULT+0] | ext_csd[EXT_CSD_ENH_SIZE_MULT+1]<<8 | ext_csd[EXT_CSD_ENH_SIZE_MULT+2]<<16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    MSG(INF, "[EXT_CSD] Enh. user area start: %xh\n",
        (ext_csd[EXT_CSD_ENH_START_ADDR+0] | ext_csd[EXT_CSD_ENH_START_ADDR+1]<<8 |
         ext_csd[EXT_CSD_ENH_START_ADDR+2]<<16 | ext_csd[EXT_CSD_ENH_START_ADDR+3]) << 24);
    MSG(INF, "[EXT_CSD] Bad block mgmt mode: %xh\n", ext_csd[EXT_CSD_BADBLK_MGMT]);
    MSG(INF, "===========================================================\n");
}
#endif /* MMC_DEBUG */

/* ======================================================================
 * CARD DETECT / PROTECT (platform-conditional)
 * ====================================================================== */

#if defined(FEATURE_MMC_CARD_DETECT)
int mmc_card_avail(struct mmc_host *host)
{
    return msdc_card_avail(host);
}
#endif

#if defined(MMC_MSDC_DRV_CTP)
int mmc_card_protected(struct mmc_host *host)
{
    return msdc_card_protected(host);
}
#endif

/* ======================================================================
 * HOST / CARD ACCESSORS
 * ====================================================================== */

struct mmc_host *mmc_get_host(int id)
{
    return &sd_host[id];
}

struct mmc_card *mmc_get_card(int id)
{
    return &sd_card[id];
}

/* ======================================================================
 * COMMAND PRIMITIVES
 * ====================================================================== */

int mmc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
    int err;
    int retry = cmd->retries;

    do {
        err = msdc_cmd(host, cmd);
        if (err == MMC_ERR_NONE)
            break;
        if (err == MMC_ERR_ERASE_SEQ)
            break;
    } while (retry--);

    return err;
}

static int mmc_app_cmd(struct mmc_host *host, struct mmc_command *cmd,
                       u32 rca, int retries)
{
    int err = MMC_ERR_FAILED;
    struct mmc_command appcmd;

    appcmd.opcode  = MMC_CMD_APP_CMD;
    appcmd.arg     = rca << 16;
    appcmd.rsptyp  = RESP_R1;
    appcmd.retries = CMD_RETRIES;
    appcmd.timeout = CMD_TIMEOUT;

    do {
        err = mmc_cmd(host, &appcmd);
        if (err == MMC_ERR_NONE)
            err = mmc_cmd(host, cmd);
        if (err == MMC_ERR_NONE)
            break;
    } while (retries--);

    return err;
}

u32 mmc_select_voltage(struct mmc_host *host, u32 ocr)
{
    int bit;

    ocr &= host->ocr_avail;
    bit  = uffs(ocr);
    if (bit) {
        bit -= 1;
        ocr &= 3 << bit;
    } else {
        ocr = 0;
    }
    return ocr;
}

int mmc_go_idle(struct mmc_host *host)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_IDLE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg     = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_go_irq_state(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_IO_MODE)) {
        MSG(ERR, "[SD%d] Card doesn't support I/O mode for IRQ state\n", host->id);
        return MMC_ERR_FAILED;
    }

    cmd.opcode  = MMC_CMD_GO_IRQ_STATE;
    cmd.rsptyp  = RESP_R5;
    cmd.arg     = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

static int mmc_go_inactive(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_INACTIVE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg     = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

static int mmc_go_pre_idle(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_IDLE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg     = 0xF0F0F0F0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_sleep_awake(struct mmc_host *host, struct mmc_card *card, int sleep)
{
    struct mmc_command cmd;
    u32 timeout;

    if (card->raw_ext_csd[EXT_CSD_S_A_TIMEOUT]) {
        timeout = ((1 << card->raw_ext_csd[EXT_CSD_S_A_TIMEOUT]) * 100) / 1000000;
    } else {
        timeout = CMD_TIMEOUT;
    }

    cmd.opcode  = MMC_CMD_SLEEP_AWAKE;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg     = (card->rca << 16) | (sleep << 15);
    cmd.retries = CMD_RETRIES;
    cmd.timeout = timeout;

    return mmc_cmd(host, &cmd);
}

int mmc_send_status(struct mmc_host *host, struct mmc_card *card, u32 *status)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SEND_STATUS;
    cmd.arg     = card->rca << 16;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);
    if (err == MMC_ERR_NONE) {
        *status = cmd.resp[0];
#if MMC_DEBUG
        mmc_dump_card_status(*status);
#endif
    }
    return err;
}

static int mmc_send_if_cond(struct mmc_host *host, u32 ocr)
{
    struct mmc_command cmd;
    int err;
    static const u8 test_pattern = 0xAA;
    u8 result_pattern;

    cmd.opcode  = SD_CMD_SEND_IF_COND;
    cmd.arg     = ((ocr & 0xFF8000) != 0) << 8 | test_pattern;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE)
        return err;

    result_pattern = cmd.resp[0] & 0xFF;
    if (result_pattern != test_pattern)
        return MMC_ERR_INVALID;

    return MMC_ERR_NONE;
}

static int mmc_sd_get_write_blocks(struct mmc_host *host, struct mmc_card *card, u32 *num)
{
    struct mmc_command cmd;
    int err;
    int result = MMC_ERR_NONE;
    u8  buf[4];

    cmd.opcode  = SD_ACMD_SEND_NR_WR_BLOCKS;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    msdc_reset_tune_counter(host);
    do {
        msdc_set_blknum(host, 1);
        msdc_set_blklen(host, 4);
        msdc_set_timeout(host, 100000000, 0);

        MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_PIO);

        err = mmc_app_cmd(host, &cmd, card->rca, CMD_RETRIES);
        if (err != MMC_ERR_NONE)
            return err;

        err = msdc_pio_read(host, (u32 *)buf, 4);
        if (err != MMC_ERR_NONE) {
            msdc_abort_handler(host, 1);
            result = msdc_tune_read(host);
        }
    } while (err && (result != MMC_ERR_READTUNEFAIL));
    msdc_reset_tune_counter(host);

    msdc_set_blklen(host, 512);
    if (err != MMC_ERR_NONE)
        return err;

    *num = buf[3] | buf[2] << 8 | buf[1] << 16 | buf[0] << 24;
    return MMC_ERR_NONE;
}

static int mmc_send_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
    struct mmc_command cmd;
    int i, err = 0;

    cmd.opcode  = MMC_CMD_SEND_OP_COND;
    cmd.arg     = ocr;
    cmd.rsptyp  = RESP_R3;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    for (i = 100; i; i--) {
        err = mmc_cmd(host, &cmd);
        if (err) break;
        if (ocr == 0) break;
        if (cmd.resp[0] & MMC_CARD_BUSY) break;
        err = MMC_ERR_TIMEOUT;
        mdelay(10);
    }

    if (!err && rocr)
        *rocr = cmd.resp[0];

    return err;
}

static int mmc_send_app_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
    struct mmc_command cmd;
    int i, err = 0;

    cmd.opcode  = SD_ACMD_SEND_OP_COND;
    cmd.arg     = ocr;
    cmd.rsptyp  = RESP_R3;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    for (i = 100; i; i--) {
        err = mmc_app_cmd(host, &cmd, 0, CMD_RETRIES);
        if (err != MMC_ERR_NONE) break;
        if (cmd.resp[0] & MMC_CARD_BUSY || ocr == 0) break;
        err = MMC_ERR_TIMEOUT;
        mdelay(10);
    }

    if (rocr)
        *rocr = cmd.resp[0];

    return err;
}

static int mmc_all_send_cid(struct mmc_host *host, u32 *cid)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_ALL_SEND_CID;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R2;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE)
        return err;

    memcpy(cid, cmd.resp, sizeof(u32) * 4);
    return MMC_ERR_NONE;
}

/* ======================================================================
 * CID / CSD / EXT_CSD DECODE
 * ====================================================================== */

static void mmc_decode_cid(struct mmc_card *card)
{
    u32 *resp = card->raw_cid;

    memset(&card->cid, 0, sizeof(struct mmc_cid));

    card->cid.prod_name[4] = unstuff_bits(resp, 64,  8);
    card->cid.prod_name[3] = unstuff_bits(resp, 72,  8);
    card->cid.prod_name[2] = unstuff_bits(resp, 80,  8);
    card->cid.prod_name[1] = unstuff_bits(resp, 88,  8);
    card->cid.prod_name[0] = unstuff_bits(resp, 96,  8);

    if (mmc_card_sd(card)) {
        card->cid.month   = unstuff_bits(resp,   8,  4);
        card->cid.year    = unstuff_bits(resp,  12,  8);
        card->cid.serial  = unstuff_bits(resp,  24, 32);
        card->cid.fwrev   = unstuff_bits(resp,  56,  4);
        card->cid.hwrev   = unstuff_bits(resp,  60,  4);
        card->cid.oemid   = unstuff_bits(resp, 104, 16);
        card->cid.manfid  = unstuff_bits(resp, 120,  8);
        card->cid.year   += 2000;
    } else {
        card->cid.year           = unstuff_bits(resp,   8, 4) + 1997;
        card->cid.month          = unstuff_bits(resp,  12, 4);
        card->cid.prod_name[5]   = unstuff_bits(resp,  56, 8);

        switch (card->csd.mmca_vsn) {
        case 0:
        case 1:
            card->cid.serial       = unstuff_bits(resp, 16, 24);
            card->cid.fwrev        = unstuff_bits(resp, 40,  4);
            card->cid.hwrev        = unstuff_bits(resp, 44,  4);
            card->cid.prod_name[6] = unstuff_bits(resp, 48,  8);
            card->cid.manfid       = unstuff_bits(resp, 104, 24);
            break;
        case 2:
        case 3:
        case 4:
            card->cid.serial  = unstuff_bits(resp,  16, 32);
            card->cid.oemid   = unstuff_bits(resp, 104, 16);
            card->cid.manfid  = unstuff_bits(resp, 120,  8);
            break;
        default:
            MSG(ERR, "[SD%d] Unknown MMCA version %d\n",
                mmc_card_id(card), card->csd.mmca_vsn);
            break;
        }
    }
}

static int mmc_decode_csd(struct mmc_card *card)
{
    struct mmc_csd *csd = &card->csd;
    unsigned int e, m, csd_struct;
    u32 *resp = card->raw_csd;

    csd_struct      = unstuff_bits(resp, 126, 2);
    csd->csd_struct = csd_struct;

    if ((mmc_card_mmc(card) &&
         (csd_struct != CSD_STRUCT_VER_1_0 && csd_struct != CSD_STRUCT_VER_1_1
          && csd_struct != CSD_STRUCT_VER_1_2 && csd_struct != CSD_STRUCT_EXT_CSD)) ||
        (mmc_card_sd(card) && (csd_struct != 0 && csd_struct != 1))) {
        MSG(ERR, "[SD%d] Unknown CSD ver %d\n", mmc_card_id(card), csd_struct);
        return MMC_ERR_INVALID;
    }

    m = unstuff_bits(resp, 99, 4);
    e = unstuff_bits(resp, 96, 3);
    csd->max_dtr      = tran_exp[e] * tran_mant[m];
    csd->read_blkbits = unstuff_bits(resp, 80, 4);

#if !defined(FEATURE_MMC_SLIM)
    csd->cmdclass         = unstuff_bits(resp,  84, 12);
    csd->read_partial     = unstuff_bits(resp,  79,  1);
    csd->write_misalign   = unstuff_bits(resp,  78,  1);
    csd->read_misalign    = unstuff_bits(resp,  77,  1);
    csd->dsr              = unstuff_bits(resp,  76,  1);
    csd->write_prot_grpsz = unstuff_bits(resp,  32,  7);
    csd->write_prot_grp   = unstuff_bits(resp,  31,  1);
    csd->r2w_factor       = unstuff_bits(resp,  26,  3);
    csd->write_blkbits    = unstuff_bits(resp,  22,  4);
    csd->write_partial    = unstuff_bits(resp,  21,  1);
    csd->copy             = unstuff_bits(resp,  14,  1);
    csd->perm_wr_prot     = unstuff_bits(resp,  13,  1);
    csd->tmp_wr_prot      = unstuff_bits(resp,  12,  1);

    m = unstuff_bits(resp, 115, 4);
    e = unstuff_bits(resp, 112, 3);
    csd->tacc_ns   = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
    csd->tacc_clks = unstuff_bits(resp, 104, 8) * 100;
#endif

    e = unstuff_bits(resp, 47, 3);
    m = unstuff_bits(resp, 62, 12);
    csd->capacity = (1 + m) << (e + 2);

    if (mmc_card_sd(card)) {
#if !defined(FEATURE_MMC_SLIM)
        csd->erase_sctsz = unstuff_bits(resp, 39, 7) + 1;
        /*
         * erase_blk_en: bit 46. The struct member may not exist in the
         * mt6589 mmc_csd struct — we guard with a compile-time check.
         * If it does exist, populate it; otherwise silently skip it.
         */
#ifdef MMC_CSD_HAS_ERASE_BLK_EN
        csd->erase_blk_en = unstuff_bits(resp, 46, 1);
#endif
#endif
        switch (csd_struct) {
        case 0:
            break;
        case 1:
            mmc_card_set_blockaddr(card);
            m = unstuff_bits(resp, 48, 22);
            csd->capacity     = (1 + m) << 10;
            csd->read_blkbits = 9;
#if !defined(FEATURE_MMC_SLIM)
            csd->tacc_ns      = 0;
            csd->tacc_clks    = 0;
            csd->read_partial = 0;
            csd->write_misalign = 0;
            csd->read_misalign  = 0;
            csd->r2w_factor   = 4;
            csd->write_blkbits = 9;
            csd->write_partial = 0;
#endif
            break;
        }
    } else {
        csd->mmca_vsn = unstuff_bits(resp, 122, 4);
#if !defined(FEATURE_MMC_SLIM)
        csd->write_prot_grpsz = unstuff_bits(resp, 32, 5);
        csd->erase_sctsz = (unstuff_bits(resp, 42, 5) + 1) *
                           (unstuff_bits(resp, 37, 5) + 1);
#endif
    }

#if MMC_DEBUG
    mmc_dump_csd(card);
#endif
    return 0;
}

static void mmc_decode_ext_csd(struct mmc_card *card)
{
    u8 *ext_csd = &card->raw_ext_csd[0];

    card->ext_csd.sectors =
        ext_csd[EXT_CSD_SEC_CNT + 0] <<  0 |
        ext_csd[EXT_CSD_SEC_CNT + 1] <<  8 |
        ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
        ext_csd[EXT_CSD_SEC_CNT + 3] << 24;

#if !defined(FEATURE_MMC_SLIM)
    card->ext_csd.rev            = ext_csd[EXT_CSD_REV];
    card->ext_csd.hc_erase_grp_sz = ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
    card->ext_csd.hc_wp_grp_sz   = ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
                                    ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
    card->ext_csd.trim_tmo_ms    = ext_csd[EXT_CSD_TRIM_MULT] * 300;
    card->ext_csd.boot_info      = ext_csd[EXT_CSD_BOOT_INFO];
    card->ext_csd.boot_part_sz   = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
    card->ext_csd.access_sz      = (ext_csd[EXT_CSD_ACC_SIZE] & 0xf) * 512;
    card->ext_csd.rpmb_sz        = ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;
    card->ext_csd.erased_mem_cont = ext_csd[EXT_CSD_ERASED_MEM_CONT];
    card->ext_csd.part_en        = ext_csd[EXT_CSD_PART_SUPPORT] &
                                   EXT_CSD_PART_SUPPORT_PART_EN ? 1 : 0;
    card->ext_csd.enh_attr_en    = ext_csd[EXT_CSD_PART_SUPPORT] &
                                   EXT_CSD_PART_SUPPORT_ENH_ATTR_EN ? 1 : 0;
    card->ext_csd.enh_start_addr =
        ext_csd[EXT_CSD_ENH_START_ADDR + 0] <<  0 |
        ext_csd[EXT_CSD_ENH_START_ADDR + 1] <<  8 |
        ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16 |
        ext_csd[EXT_CSD_ENH_START_ADDR + 3] << 24;
    card->ext_csd.enh_sz =
        (ext_csd[EXT_CSD_ENH_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) * 512 * 1024 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
#endif

    if (card->ext_csd.sectors)
        mmc_card_set_blockaddr(card);

    card->ext_csd.hs_max_dtr = 0;

    if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS400_1_2V) ||
        (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS400_1_8V)) {
        card->ext_csd.hs_max_dtr = 200000000;
        card->ext_csd.ddr_support = 1;
#ifdef MMC_CARD_HAS_VERSION
        card->version = EMMC_VER_50;
#endif
    } else if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS200_1_2V) ||
               (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS200_1_8V)) {
        card->ext_csd.hs_max_dtr = 200000000;
        if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52_1_2V) ||
            (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52))
            card->ext_csd.ddr_support = 1;
#ifdef MMC_CARD_HAS_VERSION
        card->version = EMMC_VER_45;
#endif
    } else if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52_1_2V) ||
               (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52)) {
        card->ext_csd.ddr_support = 1;
        card->ext_csd.hs_max_dtr  = 52000000;
#ifdef MMC_CARD_HAS_VERSION
        card->version = EMMC_VER_44;
#endif
    } else if (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_52) {
        card->ext_csd.hs_max_dtr = 52000000;
#ifdef MMC_CARD_HAS_VERSION
        card->version = EMMC_VER_43;
#endif
    } else if (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_26) {
        card->ext_csd.hs_max_dtr = 26000000;
#ifdef MMC_CARD_HAS_VERSION
        card->version = EMMC_VER_42;
#endif
    } else {
        MSG(ERR, "[SD%d] MMCv4 but HS unsupported\n", card->host->id);
    }

#if MMC_DEBUG
    mmc_dump_ext_csd(card);
#endif
}

/* ======================================================================
 * CARD SELECTION / RCA
 * ====================================================================== */

#if defined(MMC_MSDC_DRV_CTP)
int mmc_deselect_all_card(struct mmc_host *host)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SELECT_CARD;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_NONE;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}
#endif

int mmc_select_card(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SELECT_CARD;
    cmd.arg     = card->rca << 16;
    cmd.rsptyp  = RESP_R1B;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_send_relative_addr(struct mmc_host *host, struct mmc_card *card,
                           unsigned int *rca)
{
    int err;
    struct mmc_command cmd;

    memset(&cmd, 0, sizeof(struct mmc_command));

    if (mmc_card_mmc(card)) {
        cmd.opcode  = MMC_CMD_SET_RELATIVE_ADDR;
        cmd.arg     = *rca << 16;
        cmd.rsptyp  = RESP_R1;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
    } else {
        cmd.opcode  = SD_CMD_SEND_RELATIVE_ADDR;
        cmd.arg     = 0;
        cmd.rsptyp  = RESP_R6;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
    }

    err = mmc_cmd(host, &cmd);
    if ((err == MMC_ERR_NONE) && !mmc_card_mmc(card))
        *rca = cmd.resp[0] >> 16;

    return err;
}

int mmc_send_tuning_blk(struct mmc_host *host, struct mmc_card *card, u32 *buf)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_SEND_TUNING_BLOCK;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 64);
    msdc_set_timeout(host, 100000000, 0);

    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE) goto out;

    err = msdc_pio_read(host, buf, 64);
    if (err != MMC_ERR_NONE) goto out;

#if MMC_DEBUG
    mmc_dump_tuning_blk((u8 *)buf);
#endif
out:
    return err;
}

/* ======================================================================
 * SWITCH COMMANDS
 * ====================================================================== */

int mmc_switch(struct mmc_host *host, struct mmc_card *card,
               u8 set, u8 index, u8 value)
{
    int err;
    u32 status = 0;
    uint count = 0;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SWITCH;
    cmd.arg     = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
                  (index << 16) | (value << 8) | set;
    cmd.rsptyp  = RESP_R1B;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE)
        return err;

    do {
        err = mmc_send_status(host, card, &status);
        if (err) {
            MSG(ERR, "[SD%d] Fail to send status %d\n", host->id, err);
            break;
        }
        if (status & R1_SWITCH_ERROR) {
            MSG(ERR, "[SD%d] switch error. arg(0x%x)\n", host->id, cmd.arg);
            return MMC_ERR_FAILED;
        }
        if (count++ >= 600000) {
            MSG(ERR, "[%s]: timeout happend, count=%d, status=0x%x\n",
                __func__, count, status);
            break;
        }
    } while (!(status & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(status) == 7));

    return err;
}

static int mmc_sd_switch(struct mmc_host *host, struct mmc_card *card,
                         int mode, int group, u8 value, mmc_switch_t *resp)
{
    int err = MMC_ERR_FAILED;
    struct mmc_command cmd;
    u32 *sts = (u32 *)resp;

    mode   = !!mode;
    value &= 0xF;

    cmd.opcode  = SD_CMD_SWITCH;
    cmd.arg     = mode << 31 | 0x00FFFFFF;
    cmd.arg    &= ~(0xF << (group * 4));
    cmd.arg    |=  value << (group * 4);
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = 100;

#if defined(FEATURE_MMC_RD_TUNING)
    msdc_reset_tune_counter(host);
    do {
#endif
        msdc_set_blknum(host, 1);
        msdc_set_blklen(host, 64);
        msdc_set_timeout(host, 100000000, 0);

        err = mmc_cmd(host, &cmd);
        if (err != MMC_ERR_NONE) goto out;

        err = msdc_pio_read(host, sts, 64);
        if (err != MMC_ERR_NONE) {
            msdc_abort_handler(host, 1);
#if defined(FEATURE_MMC_RD_TUNING)
            msdc_tune_read(host);
#else
            goto out;
#endif
        }
#if defined(FEATURE_MMC_RD_TUNING)
    } while (err);
    msdc_reset_tune_counter(host);
#endif
out:
    return err;
}

#if defined(FEATURE_MMC_UHS1)
int mmc_ctrl_speed_class(struct mmc_host *host, u32 scc)
{
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_SPEED_CLASS_CTRL;
    cmd.arg     = scc << 28;
    cmd.rsptyp  = RESP_R1B;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_switch_volt(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_VOL_SWITCH;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);
    if (err == MMC_ERR_NONE)
        err = msdc_switch_volt(host, MMC_VDD_18_19);

    return err;
}
#endif /* FEATURE_MMC_UHS1 */

int mmc_switch_hs(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    u8  status[64];

    err = mmc_sd_switch(host, card, 1, 0, MMC_SWITCH_MODE_SDR25,
                        (mmc_switch_t *)&status[0]);
    if (err != MMC_ERR_NONE) goto out;

    if ((status[16] & 0xF) != 1) {
        MSG(ERR, "[SD%d] HS mode not supported!\n", host->id);
        err = MMC_ERR_FAILED;
    } else {
        printf("[SD%d] Switch to HS mode!\n", host->id);
        mmc_card_set_highspeed(card);
    }
out:
    return err;
}

#if defined(FEATURE_MMC_UHS1)
int mmc_switch_uhs1(struct mmc_host *host, struct mmc_card *card, unsigned int mode)
{
    int err;
    u8  status[64];
    const char *smode[] = {"SDR12","SDR25","SDR50","SDR104","DDR50"};

    err = mmc_sd_switch(host, card, 1, 0, mode, (mmc_switch_t *)&status[0]);
    if (err != MMC_ERR_NONE) goto out;

    if ((status[16] & 0xF) != mode) {
        MSG(ERR, "[SD%d] UHS-1 %s mode not supported!\n", host->id, smode[mode]);
        err = MMC_ERR_FAILED;
    } else {
        card->uhs_mode = mode;
        mmc_card_set_uhs1(card);
        printf("[SD%d] Switch to UHS-1 %s mode!\n", host->id, smode[mode]);
        if (mode == MMC_SWITCH_MODE_DDR50)
            mmc_card_set_ddr(card);
    }
out:
    return err;
}

int mmc_switch_drv_type(struct mmc_host *host, struct mmc_card *card, int val)
{
    int err;
    u8  status[64];
    const char *type[] = {"TYPE-B","TYPE-A","TYPE-C","TYPE-D"};

    err = mmc_sd_switch(host, card, 1, 2, val, (mmc_switch_t *)&status[0]);
    if (err != MMC_ERR_NONE) goto out;

    if ((status[15] & 0xF) != val) {
        MSG(ERR, "[SD%d] UHS-1 %s drv not supported!\n", host->id, type[val]);
        err = MMC_ERR_FAILED;
    } else {
        printf("[SD%d] Switch to UHS-1 %s drv!\n", host->id, type[val]);
    }
out:
    return err;
}

int mmc_switch_max_cur(struct mmc_host *host, struct mmc_card *card, int val)
{
    int err;
    u8  status[64];
    const char *curr[] = {"200mA","400mA","600mA","800mA"};

    err = mmc_sd_switch(host, card, 1, 3, val, (mmc_switch_t *)&status[0]);
    if (err != MMC_ERR_NONE) goto out;

    if (((status[15] >> 4) & 0xF) != val) {
        MSG(ERR, "[SD%d] UHS-1 %s max. current not supported!\n", host->id, curr[val]);
        err = MMC_ERR_FAILED;
    } else {
        printf("[SD%d] Switch to UHS-1 %s max. current!\n", host->id, curr[val]);
    }
out:
    return err;
}
#endif /* FEATURE_MMC_UHS1 */

/* ======================================================================
 * READ REGISTER HELPERS
 * ====================================================================== */

static int mmc_read_csds(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SEND_CSD;
    cmd.arg     = card->rca << 16;
    cmd.rsptyp  = RESP_R2;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT * 100;

    err = mmc_cmd(host, &cmd);
    if (err == MMC_ERR_NONE)
        memcpy(&card->raw_csd, &cmd.resp[0], sizeof(u32) * 4);
    return err;
}

#if !defined(FEATURE_MMC_SLIM)
static int mmc_read_scrs(struct mmc_host *host, struct mmc_card *card)
{
    int err    = MMC_ERR_NONE;
    int result = MMC_ERR_NONE;
    struct mmc_command cmd;
    struct sd_scr *scr = &card->scr;
    u32 resp[4];
    u32 tmp;
    u8  buf[8];

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 8);
    msdc_set_timeout(host, 100000000, 0);

    memset(buf, 0, 8);
    cmd.opcode  = SD_ACMD_SEND_SCR;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

#if defined(FEATURE_MMC_RD_TUNING)
    msdc_reset_tune_counter(host);
    do {
#endif
        mmc_app_cmd(host, &cmd, card->rca, CMD_RETRIES);
        if ((err != MMC_ERR_NONE) || !(cmd.resp[0] & R1_APP_CMD))
            return MMC_ERR_FAILED;

        err = msdc_pio_read(host, (u32 *)buf, 8);
        if (err != MMC_ERR_NONE) {
            msdc_abort_handler(host, 1);
#if defined(FEATURE_MMC_RD_TUNING)
            result = msdc_tune_read(host);
#else
            return err;
#endif
        }
#if defined(FEATURE_MMC_RD_TUNING)
    } while (err && result != MMC_ERR_READTUNEFAIL);
    msdc_reset_tune_counter(host);
#endif

    if ((err == MMC_ERR_NONE) && (result != MMC_ERR_READTUNEFAIL))
        memcpy(card->raw_scr, buf, 8);

    MSG(INF, "[SD%d] SCR: %x %x (raw)\n", host->id,
        card->raw_scr[0], card->raw_scr[1]);

    tmp             = ntohl(card->raw_scr[0]);
    card->raw_scr[0] = ntohl(card->raw_scr[1]);
    card->raw_scr[1] = tmp;

    MSG(INF, "[SD%d] SCR: %x %x (ntohl)\n", host->id,
        card->raw_scr[0], card->raw_scr[1]);

    resp[2] = card->raw_scr[1];
    resp[3] = card->raw_scr[0];

    if (unstuff_bits(resp, 60, 4) != 0) {
        MSG(ERR, "[SD%d] Unknown SCR ver %d\n",
            mmc_card_id(card), unstuff_bits(resp, 60, 4));
        return MMC_ERR_INVALID;
    }

    scr->scr_struct           = unstuff_bits(resp, 60, 4);
    scr->sda_vsn              = unstuff_bits(resp, 56, 4);
    scr->data_bit_after_erase = unstuff_bits(resp, 55, 1);
    scr->security             = unstuff_bits(resp, 52, 3);
    scr->bus_widths           = unstuff_bits(resp, 48, 4);
    scr->sda_vsn3             = unstuff_bits(resp, 47, 1);
    scr->ex_security          = unstuff_bits(resp, 43, 4);
    scr->cmd_support          = unstuff_bits(resp, 32, 2);

    MSG(INF, "[SD%d] SD_SPEC(%d) SD_SPEC3(%d) SD_BUS_WIDTH=%d\n",
        mmc_card_id(card), scr->sda_vsn, scr->sda_vsn3, scr->bus_widths);
    MSG(INF, "[SD%d] SD_SECU(%d) EX_SECU(%d), CMD_SUPP(%d): CMD23(%d), CMD20(%d)\n",
        mmc_card_id(card), scr->security, scr->ex_security, scr->cmd_support,
        (scr->cmd_support >> 1) & 0x1, scr->cmd_support & 0x1);

    return err;
}
#endif /* !FEATURE_MMC_SLIM */

int mmc_read_ext_csd(struct mmc_host *host, struct mmc_card *card)
{
    int err    = MMC_ERR_NONE;
    int result = MMC_ERR_NONE;
    struct mmc_command cmd;
    u32 *ptr;
    u8   buf[512];

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) {
        MSG(INF, "[SD%d] MMCA_VSN: %d. Skip EXT_CSD\n",
            host->id, card->csd.mmca_vsn);
        return MMC_ERR_NONE;
    }

    memset(buf, 0, 512);
    ptr = (u32 *)buf;

    cmd.opcode  = MMC_CMD_SEND_EXT_CSD;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

#if defined(FEATURE_MMC_RD_TUNING)
    msdc_reset_tune_counter(host);
    do {
#endif
        msdc_set_blknum(host, 1);
        msdc_set_blklen(host, 512);
        msdc_set_timeout(host, 100000000, 0);

        err = mmc_cmd(host, &cmd);
        if (err != MMC_ERR_NONE) goto out;

        err = msdc_pio_read(host, ptr, 512);
        if (err != MMC_ERR_NONE) {
            host->card = card;
            msdc_abort_handler(host, 1);
#if defined(FEATURE_MMC_RD_TUNING)
            result = msdc_tune_read(host);
#endif
        }
#if defined(FEATURE_MMC_RD_TUNING)
    } while (err && result != MMC_ERR_READTUNEFAIL);
    msdc_reset_tune_counter(host);
#endif

    if ((err == MMC_ERR_NONE) && (result != MMC_ERR_READTUNEFAIL)) {
        memcpy(card->raw_ext_csd, buf, 512);
        mmc_decode_ext_csd(card);
    }
out:
    return err;
}

static int mmc_read_switch(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    u8  status[64];

    err = mmc_sd_switch(host, card, 0, 0, 1, (mmc_switch_t *)&status[0]);
    if (err != MMC_ERR_NONE) {
        err = MMC_ERR_NONE;
        goto out;
    }

    if (status[13] & 0x01) {
        MSG(INF, "[SD%d] Support: Default/SDR12\n", host->id);
        card->sw_caps.hs_max_dtr = 25000000;
    }
    if (status[13] & 0x02) {
        MSG(INF, "[SD%d] Support: HS/SDR25\n", host->id);
        card->sw_caps.hs_max_dtr = 50000000;
    }
    if (status[13] & 0x10) {
        MSG(INF, "[SD%d] Support: DDR50\n", host->id);
        card->sw_caps.hs_max_dtr = 50000000;
        card->sw_caps.ddr = 1;
    }
#if defined(FEATURE_MMC_UHS1)
    if (status[13] & 0x04) {
        MSG(INF, "[SD%d] Support: SDR50\n", host->id);
        card->sw_caps.hs_max_dtr = 100000000;
    }
    if (status[13] & 0x08) {
        MSG(INF, "[SD%d] Support: SDR104\n", host->id);
        card->sw_caps.hs_max_dtr = 208000000;
    }
    if (status[9] & 0x01) MSG(INF, "[SD%d] Support: Type-B Drv\n", host->id);
    if (status[9] & 0x02) MSG(INF, "[SD%d] Support: Type-A Drv\n", host->id);
    if (status[9] & 0x04) MSG(INF, "[SD%d] Support: Type-C Drv\n", host->id);
    if (status[9] & 0x08) MSG(INF, "[SD%d] Support: Type-D Drv\n", host->id);
    if (status[7] & 0x01) MSG(INF, "[SD%d] Support: 200mA current limit\n", host->id);
    if (status[7] & 0x02) MSG(INF, "[SD%d] Support: 400mA current limit\n", host->id);
    if (status[7] & 0x04) MSG(INF, "[SD%d] Support: 600mA current limit\n", host->id);
    if (status[7] & 0x08) MSG(INF, "[SD%d] Support: 800mA current limit\n", host->id);
#endif
out:
    return err;
}

/* ======================================================================
 * WRITE PROTECTION
 * ====================================================================== */

int mmc_send_write_prot(struct mmc_card *card, u32 wp_addr, u32 *wp_status)
{
    int err;
    struct mmc_command cmd;
    struct mmc_host *host = card->host;
    u8 *buf = (u8 *)wp_status;

    if (!(card->csd.cmdclass & CCC_WRITE_PROT))
        return MMC_ERR_INVALID;

    cmd.opcode  = MMC_CMD_SEND_WRITE_PROT;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = wp_addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 4);
    msdc_set_timeout(host, 100000000, 0);

    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE) goto out;
    err = msdc_pio_read(host, (u32 *)buf, 4);
out:
    return err;
}

/* ======================================================================
 * ERASE
 * ====================================================================== */

int mmc_erase_start(struct mmc_card *card, u64 addr)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_ERASE)) {
        MSG(ERR, "[SD%d] Card doesn't support Erase commands\n", card->host->id);
        return MMC_ERR_INVALID;
    }

    if (mmc_card_highcaps(card))
        addr /= MMC_BLOCK_SIZE;

    cmd.opcode  = mmc_card_mmc(card) ?
                  MMC_CMD_ERASE_GROUP_START : MMC_CMD_ERASE_WR_BLK_START;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = (u32)addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(card->host, &cmd);
}

int mmc_erase_end(struct mmc_card *card, u64 addr)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_ERASE)) {
        MSG(ERR, "[SD%d] Erase isn't supported\n", card->host->id);
        return MMC_ERR_INVALID;
    }

    if (mmc_card_highcaps(card))
        addr /= MMC_BLOCK_SIZE;

    cmd.opcode  = mmc_card_mmc(card) ?
                  MMC_CMD_ERASE_GROUP_END : MMC_CMD_ERASE_WR_BLK_END;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = (u32)addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(card->host, &cmd);
}

int mmc_erase(struct mmc_card *card, u32 arg)
{
    int err;
    u32 status;
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_ERASE)) {
        MSG(ERR, "[SD%d] Erase isn't supported\n", card->host->id);
        return MMC_ERR_INVALID;
    }

    if (arg & MMC_ERASE_SECURE_REQ) {
        if (!(card->raw_ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT] &
              EXT_CSD_SEC_FEATURE_ER_EN))
            return MMC_ERR_INVALID;
    }
    if ((arg & MMC_ERASE_GC_REQ) || (arg & MMC_ERASE_TRIM)) {
        if (!(card->raw_ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT] &
              EXT_CSD_SEC_FEATURE_GB_CL_EN))
            return MMC_ERR_INVALID;
    }

    cmd.opcode  = MMC_CMD_ERASE;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg     = arg;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(card->host, &cmd);
    if (!err) {
        do {
            err = mmc_send_status(card->host, card, &status);
            if (err) break;
#if MMC_DEBUG
            mmc_dump_card_status(status);
#endif
            if (R1_STATUS(status) != 0) break;
        } while (R1_CURRENT_STATE(status) == 7);
    }
    return err;
}

/* ======================================================================
 * TUNING
 * ====================================================================== */

#if defined(FEATURE_MMC_UHS1)
int mmc_tune_timing(struct mmc_host *host, struct mmc_card *card)
{
    int err = MMC_ERR_NONE;

    if (mmc_card_sd(card) && mmc_card_uhs1(card) && !mmc_card_ddr(card))
        err = msdc_tune_uhs1(host, card);
    else if (mmc_card_mmc(card) && mmc_card_hs200(card))
        err = msdc_tune_hs200(host, card);
    else if (mmc_card_mmc(card) && mmc_card_hs400(card))
        err = msdc_tune_hs400(host, card);

    return err;
}
#endif

/* ======================================================================
 * WRITE PROTECT GROUP SIZE
 * ====================================================================== */

u32 mmc_get_wpg_size(struct mmc_card *card)
{
    u32 size;
    u8 *ext_csd;

    if (mmc_card_mmc(card)) {
        ext_csd = &card->raw_ext_csd[0];
        if ((ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN) &&
            (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] > 0)) {
            size = 512 * 1024 * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] *
                   ext_csd[EXT_CSD_HC_WP_GPR_SIZE];
        } else {
            size = card->csd.write_prot_grpsz;
        }
    } else {
        if (card->csd.write_prot_grp)
            size = (card->csd.write_prot_grpsz + 1) * (1 << card->csd.write_blkbits);
        else
            size = 0;
    }
    return size;
}

/* ======================================================================
 * CLOCK CONFIGURATION
 *
 * mt6589 struct mmc_host does NOT have a cur_bus_clk member.
 * We track it locally and expose it via a helper so callers that
 * previously referenced host->cur_bus_clk still work.
 * ====================================================================== */

/*
 * Per-host shadow of the current bus clock, indexed by host->id.
 * NR_MMC is bounded by MSDC_MAX_NUM which is small (typically 2-4).
 */
static u32 g_cur_bus_clk[MSDC_MAX_NUM];

static inline u32 mmc_host_get_cur_clk(struct mmc_host *host)
{
    if (host->id < MSDC_MAX_NUM)
        return g_cur_bus_clk[host->id];
    return 0;
}

static inline void mmc_host_set_cur_clk(struct mmc_host *host, u32 hz)
{
    if (host->id < MSDC_MAX_NUM)
        g_cur_bus_clk[host->id] = hz;
}

void mmc_set_clock(struct mmc_host *host, int ddr, u32 hz)
{
    unsigned int hs_timing = 0;

    if (hz >= host->f_max)
        hz = host->f_max;
    else if (hz < host->f_min)
        hz = host->f_min;

    if (host->card && mmc_card_hs400(host->card))
        hs_timing |= EXT_CSD_HS_TIMEING_HS400;

    if (host->card)
        msdc_config_clock(host,
            (mmc_card_ddr(host->card) > 0) ? 1 : 0, hz, hs_timing);
    else
        msdc_config_clock(host, ddr ? 1 : 0, hz, hs_timing);

    mmc_host_set_cur_clk(host, hz);
}

/* ======================================================================
 * EXT_CSD SETTERS
 * ====================================================================== */

int mmc_set_ext_csd(struct mmc_card *card, uint8 addr, uint8 value)
{
    int err;
    u8 *ext_csd;

    if (192 <= addr || !card || !mmc_card_mmc(card))
        return MMC_ERR_INVALID;

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, addr, value);
    if (err == MMC_ERR_NONE) {
        err = mmc_read_ext_csd(card->host, card);
        if (err == MMC_ERR_NONE) {
            ext_csd = &card->raw_ext_csd[0];
            if (ext_csd[addr] != value)
                err = MMC_ERR_FAILED;
        }
    }
    return err;
}

int mmc_set_card_detect(struct mmc_host *host, struct mmc_card *card, int connect)
{
    struct mmc_command cmd;

    cmd.opcode  = SD_ACMD_SET_CLR_CD;
    cmd.arg     = connect;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_app_cmd(host, &cmd, card->rca, CMD_RETRIES);
}

int mmc_set_blk_length(struct mmc_host *host, u32 blklen)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SET_BLOCKLEN;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = blklen;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);
    if (err == MMC_ERR_NONE)
        msdc_set_blklen(host, blklen);

    return err;
}

#if defined(MMC_MSDC_DRV_CTP)
int mmc_set_blk_count(struct mmc_host *host, u32 blkcnt)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SET_BLOCK_COUNT;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = blkcnt;
#if MSDC_USE_DATA_TAG
    cmd.arg |= (1 << 29);
    cmd.arg &= ~(1 << 30);
#endif
#if MSDC_USE_RELIABLE_WRITE
    cmd.arg |= (1 << 31);
    cmd.arg &= ~(1 << 30);
#endif
#if MSDC_USE_FORCE_FLUSH
    cmd.arg |= (1 << 24);
    cmd.arg &= ~(1 << 30);
#endif
#if MSDC_USE_PACKED_CMD
    cmd.arg &= ~0xffff;
    cmd.arg |= (1 << 30);
#endif
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}
#endif /* MMC_MSDC_DRV_CTP */

int mmc_set_bus_width(struct mmc_host *host, struct mmc_card *card, int width)
{
    int err = MMC_ERR_NONE;
    u32 arg = 0;
    struct mmc_command cmd;

    if (mmc_card_sd(card)) {
        if (width == HOST_BUS_WIDTH_8) {
            WARN_ON(width == HOST_BUS_WIDTH_8);
            arg   = SD_BUS_WIDTH_4;
            width = HOST_BUS_WIDTH_4;
        }
        if ((width == HOST_BUS_WIDTH_4) && (host->caps & MMC_CAP_4_BIT_DATA))
            arg = SD_BUS_WIDTH_4;
        else {
            arg   = SD_BUS_WIDTH_1;
            width = HOST_BUS_WIDTH_1;
        }

        cmd.opcode  = SD_ACMD_SET_BUSWIDTH;
        cmd.arg     = arg;
        cmd.rsptyp  = RESP_R1;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;

        err = mmc_app_cmd(host, &cmd, card->rca, 0);
        if (err != MMC_ERR_NONE) goto out;

        msdc_config_bus(host, width);

    } else if (mmc_card_mmc(card)) {
        if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
            goto out;

        if (width == HOST_BUS_WIDTH_8) {
            if (host->caps & MMC_CAP_8_BIT_DATA) {
#if defined(MSDC0_EMMC50_SUPPORT)
                if (mmc_card_hs400(card)) {
                    arg = (host->caps & MMC_CAP_EMMC_HS400) ?
                          EXT_CSD_BUS_WIDTH_8_DDR : EXT_CSD_BUS_WIDTH_8;
                } else
#endif
                if (mmc_card_highspeed(card)) {
                    arg = ((host->caps & MMC_CAP_DDR) && card->ext_csd.ddr_support) ?
                          EXT_CSD_BUS_WIDTH_8_DDR : EXT_CSD_BUS_WIDTH_8;
                } else if (mmc_card_hs200(card) || mmc_card_backyard(card)) {
                    arg = EXT_CSD_BUS_WIDTH_8;
                } else {
                    width = HOST_BUS_WIDTH_4;
                }
            } else {
                width = HOST_BUS_WIDTH_4;
            }
        }

        if (width == HOST_BUS_WIDTH_4) {
            if (host->caps & MMC_CAP_4_BIT_DATA) {
#if defined(MSDC0_EMMC50_SUPPORT)
                if (mmc_card_hs400(card)) {
                    arg = (host->caps & MMC_CAP_EMMC_HS400) ?
                          EXT_CSD_BUS_WIDTH_8_DDR : EXT_CSD_BUS_WIDTH_8;
                } else
#endif
                if (mmc_card_highspeed(card)) {
                    arg = ((host->caps & MMC_CAP_DDR) && card->ext_csd.ddr_support) ?
                          EXT_CSD_BUS_WIDTH_4_DDR : EXT_CSD_BUS_WIDTH_4;
                } else if (mmc_card_hs200(card) || mmc_card_backyard(card)) {
                    arg = EXT_CSD_BUS_WIDTH_4;
                } else {
                    width = HOST_BUS_WIDTH_1;
                }
            } else {
                width = HOST_BUS_WIDTH_1;
            }
        }

        if (width == HOST_BUS_WIDTH_1)
            arg = EXT_CSD_BUS_WIDTH_1;

        err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH, arg);
        if (err != MMC_ERR_NONE) {
            MSG(ERR, "[SD%d] Switch to bus width(%d) failed\n", host->id, arg);
            goto out;
        }

        if (arg == EXT_CSD_BUS_WIDTH_8_DDR || arg == EXT_CSD_BUS_WIDTH_4_DDR)
            mmc_card_set_ddr(card);
        else
            mmc_card_clr_ddr(card);

        mmc_set_clock(host, mmc_card_ddr(card), mmc_host_get_cur_clk(host));
        msdc_config_bus(host, width);
    } else {
        BUG_ON(1);
    }
out:
    return err;
}

int mmc_set_erase_grp_def(struct mmc_card *card, int enable)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card) || !mmc_card_highcaps(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    err = mmc_set_ext_csd(card, EXT_CSD_ERASE_GRP_DEF,
                          EXT_CSD_ERASE_GRP_DEF_EN & enable);
out:
    return err;
}

int mmc_set_gp_size(struct mmc_card *card, u8 id, u32 size)
{
    int i;
    int err = MMC_ERR_FAILED;
    u8  gp[] = { EXT_CSD_GP1_SIZE_MULT, EXT_CSD_GP2_SIZE_MULT,
                 EXT_CSD_GP3_SIZE_MULT, EXT_CSD_GP4_SIZE_MULT };
    u8  arg;
    u8 *ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_sd(card) || !mmc_card_highcaps(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    id--;
    size /= 512 * 1024;
    size /= (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);

    for (i = 0; i < 3; i++) {
        arg  = (u8)(size & 0xFF);
        size = size >> 8;
        err  = mmc_set_ext_csd(card, gp[id] + i, arg);
        if (err) goto out;
    }
out:
    return err;
}

int mmc_set_enh_size(struct mmc_card *card, u32 size)
{
    int i;
    int err = MMC_ERR_FAILED;
    u8  arg;
    u8 *ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_sd(card) || !mmc_card_highcaps(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;
    if (0 == (card->raw_ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN))
        goto out;

    size /= (512 * 1024);
    size /= (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);

    for (i = 0; i < 3; i++) {
        arg  = (u8)(size & 0xFF);
        size = size >> 8;
        err  = mmc_set_ext_csd(card, EXT_CSD_ENH_SIZE_MULT + i, arg);
        if (err) goto out;
    }
out:
    return err;
}

int mmc_set_enh_start_addr(struct mmc_card *card, u32 addr)
{
    int i;
    int err = MMC_ERR_FAILED;
    u8  arg;

    if (mmc_card_sd(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;
    if (0 == (card->raw_ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN))
        goto out;

    if (mmc_card_highcaps(card))
        addr = addr / 512;

    for (i = 0; i < 4; i++) {
        arg  = (u8)(addr & 0xFF);
        addr = addr >> 8;
        err  = mmc_set_ext_csd(card, EXT_CSD_ENH_START_ADDR + i, arg);
        if (err) goto out;
    }
out:
    return err;
}

int mmc_set_boot_bus(struct mmc_card *card, u8 rst_bwidth, u8 mode, u8 bwidth)
{
    int err = MMC_ERR_FAILED;
    u8  arg;

    if (mmc_card_sd(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    arg = mode | rst_bwidth | bwidth;
    err = mmc_set_ext_csd(card, EXT_CSD_BOOT_BUS_WIDTH, arg);
out:
    return err;
}

int mmc_set_part_config(struct mmc_card *card, u8 cfg)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    err = mmc_set_ext_csd(card, EXT_CSD_PART_CFG, cfg);
out:
    return err;
}

int mmc_set_part_attr(struct mmc_card *card, u8 attr)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;
    if (!card->ext_csd.enh_attr_en) { err = MMC_ERR_INVALID; goto out; }

    attr &= 0x1F;
    attr |= (card->raw_ext_csd[EXT_CSD_PART_ATTR] & 0x1F);
    err   = mmc_set_ext_csd(card, EXT_CSD_PART_ATTR, attr);
out:
    return err;
}

int mmc_set_part_compl(struct mmc_card *card)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    err = mmc_set_ext_csd(card, EXT_CSD_PART_SET_COMPL, EXT_CSD_PART_SET_COMPL_BIT);
out:
    return err;
}

#if defined(MMC_MSDC_DRV_CTP)
int mmc_set_reset_func(struct mmc_card *card, u8 enable)
{
    int err    = MMC_ERR_FAILED;
    u8 *ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_sd(card)) goto out;
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    if (ext_csd[EXT_CSD_RST_N_FUNC] == 0)
        err = mmc_set_ext_csd(card, EXT_CSD_RST_N_FUNC, enable);
    else
        return MMC_ERR_NONE;
out:
    return err;
}

int mmc_boot_config(struct mmc_card *card, u8 acken, u8 enpart,
                    u8 buswidth, u8 busmode)
{
    int err = MMC_ERR_FAILED;
    u8  val;
    u8  rst_bwidth = 0;
    u8 *ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_sd(card) || card->csd.mmca_vsn < CSD_SPEC_VER_4 ||
        !card->ext_csd.boot_info || card->ext_csd.rev < 3)
        goto out;

    if (card->ext_csd.rev > 3 && !card->ext_csd.part_en)
        goto out;

    val = acken | enpart | (ext_csd[EXT_CSD_PART_CFG] & 0x7);
    err = mmc_set_part_config(card, val);
    if (err != MMC_ERR_NONE) goto out;

    ext_csd[EXT_CSD_PART_CFG] = val;
    rst_bwidth = (buswidth != EXT_CSD_BOOT_BUS_WIDTH_1 ? 1 : 0) << 2;
    MSG(INF, "=====Set boot Bus Width<%d>=======\n", buswidth);
    MSG(INF, "=====Set boot Bus mode<%d>=======\n", busmode);
    err = mmc_set_boot_bus(card, rst_bwidth, busmode, buswidth);
out:
    return err;
}

#if defined(FEATURE_MMC_BOOT_MODE)
int mmc_part_read(struct mmc_card *card, u8 partno, unsigned long blknr,
                  u32 blkcnt, unsigned long *dst)
{
    int err = MMC_ERR_FAILED;
    u8  val;
    u8 *ext_csd = &card->raw_ext_csd[0];
    struct mmc_host *host = card->host;

    if (mmc_card_sd(card) || card->csd.mmca_vsn < CSD_SPEC_VER_4 ||
        !card->ext_csd.boot_info || card->ext_csd.rev < 3)
        goto out;

    if (card->ext_csd.rev > 3 && !card->ext_csd.part_en)
        goto out;

    val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | (partno & 0x7);
    err = mmc_set_part_config(card, val);
    if (err != MMC_ERR_NONE) goto out;

    err = mmc_block_read(host->id, blknr, blkcnt, dst);
out:
    val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_DEFT_PART;
    mmc_set_part_config(card, val);
    return err;
}

int mmc_part_write(struct mmc_card *card, u8 partno, unsigned long blknr,
                   u32 blkcnt, unsigned long *src)
{
    int err = MMC_ERR_FAILED;
    u8  val;
    u8 *ext_csd = &card->raw_ext_csd[0];
    struct mmc_host *host = card->host;

    if (mmc_card_sd(card) || card->csd.mmca_vsn < CSD_SPEC_VER_4 ||
        !card->ext_csd.boot_info || card->ext_csd.rev < 3)
        goto out;

    if (card->ext_csd.rev > 3 && !card->ext_csd.part_en)
        goto out;

    val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | (partno & 0x7);
    err = mmc_set_part_config(card, val);
    if (err != MMC_ERR_NONE) goto out;

    err = mmc_block_write(host->id, blknr, blkcnt, src);
out:
    val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_DEFT_PART;
    mmc_set_part_config(card, val);
    return err;
}
#endif /* FEATURE_MMC_BOOT_MODE */
#endif /* MMC_MSDC_DRV_CTP */

/* ======================================================================
 * BLOCK READ / WRITE (high-level, with tuning and retry)
 * ====================================================================== */

int mmc_dev_bread(struct mmc_card *card, unsigned long blknr,
                  u32 blkcnt, u8 *dst)
{
    struct mmc_host *host  = card->host;
    u32 blksz  = host->blklen;
    int tune   = 0;
#if defined(FEATURE_MMC_RD_TUNING)
    int retry  = 1;
#else
    int retry  = 3;
#endif
    int err;
    unsigned long src;
    u8 *oridst = dst;

    src = mmc_card_highcaps(card) ? blknr : blknr * blksz;

    do {
        mmc_prof_start();
        if (!tune) {
            err = host->blk_read(host, (uchar *)dst, src, blkcnt);
        } else {
#ifdef FEATURE_MMC_RD_TUNING
#if defined(MSDC0_EMMC50_SUPPORT)
            if (mmc_card_mmc(card) && mmc_card_hs400(card)) {
                MSG(INF, "[tune][%s:%d] start hs400 read tune\n",
                    __func__, __LINE__);
                err = msdc_tune_rw_hs400(host, (uchar *)oridst, src, blkcnt, 0);
            } else
#endif
            {
                err = msdc_tune_bread(host, (uchar *)oridst, src, blkcnt);
            }
#endif
            if (err && (mmc_host_get_cur_clk(host) > (host->f_max >> 4))) {
                mmc_set_clock(host, mmc_card_ddr(card),
                              mmc_host_get_cur_clk(host) >> 1);
                err = host->blk_read(host, (uchar *)oridst, src, blkcnt);
            }
        }
        mmc_prof_stop();

        if (err == MMC_ERR_NONE) {
            mmc_prof_update(mmc_prof_read, blkcnt, mmc_prof_handle(host->id));
            break;
        }

#if defined(FEATURE_MMC_CM_TUNING) || defined(FEATURE_MMC_RD_TUNING)
        if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC ||
            err == MMC_ERR_CMD_RSPCRC) {
            if (tune) break;
            tune = 1;
        } else if (err == MMC_ERR_READTUNEFAIL || err == MMC_ERR_CMDTUNEFAIL) {
            printf("[SD%d] Fail to tuning,%s", host->id,
                   (err == MMC_ERR_CMDTUNEFAIL) ?
                   "cmd tune failed!\n" : "read tune failed!\n");
            break;
        }
#endif
        if (err == MMC_ERR_TIMEOUT) {
            printf("[SD%d] mmc_dev_bread TIMEOUT\n", host->id);
            break;
        }
    } while (retry--);

    return err;
}

static int mmc_dev_bwrite(struct mmc_card *card, unsigned long blknr,
                          u32 blkcnt, u8 *src)
{
    struct mmc_host *host  = card->host;
    u32 blksz  = host->blklen;
    u32 status;
    int tune   = 0;
#if defined(FEATURE_MMC_WR_TUNING)
    int retry  = 1;
#else
    int retry  = 3;
#endif
    int err;
    unsigned long dst;
    u8 *orisrc = src;

    dst = mmc_card_highcaps(card) ? blknr : blknr * blksz;

    do {
        mmc_prof_start();
        if (!tune) {
            err = host->blk_write(host, dst, (uchar *)src, blkcnt);
        } else {
#if defined(FEATURE_MMC_WR_TUNING)
#if defined(MSDC0_EMMC50_SUPPORT)
            if (mmc_card_mmc(card) && mmc_card_hs400(card))
                err = msdc_tune_rw_hs400(host, (uchar *)dst,
                                         (ulong)orisrc, blkcnt, 1);
            else
#endif
                err = msdc_tune_bwrite(host, dst, (uchar *)orisrc, blkcnt);
#endif
            if (err && (mmc_host_get_cur_clk(host) > (host->f_max >> 4))) {
                mmc_set_clock(host, mmc_card_ddr(card),
                              mmc_host_get_cur_clk(host) >> 1);
                err = host->blk_write(host, dst, (uchar *)orisrc, blkcnt);
            }
        }

        if (err == MMC_ERR_NONE) {
            do {
                err = mmc_send_status(host, card, &status);
                if (err) {
                    MSG(ERR, "[SD%d] Fail to send status %d\n", host->id, err);
                    break;
                }
            } while (!(status & R1_READY_FOR_DATA) ||
                     (R1_CURRENT_STATE(status) == 7));

            mmc_prof_stop();
            mmc_prof_update(mmc_prof_write, blkcnt, mmc_prof_handle(host->id));
            MSG(OPS, "[SD%d] Write %d bytes (DONE)\n", host->id, blkcnt * blksz);
            break;
        }

#if defined(FEATURE_MMC_WR_TUNING)
        if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC ||
            err == MMC_ERR_CMD_RSPCRC) {
            if (tune) break;
            tune = 1;
        }
#endif
        if (err == MMC_ERR_TIMEOUT) {
            printf("[SD%d] mmc_dev_bwrite TIMEOUT\n", host->id);
            break;
        }
    } while (retry--);

    return err;
}

int mmc_block_read(int dev_num, unsigned long blknr, u32 blkcnt,
                   unsigned long *dst)
{
    struct mmc_host *host = mmc_get_host(dev_num);
    struct mmc_card *card = mmc_get_card(dev_num);
    u32 blksz   = host->blklen;
    u32 maxblks = host->max_phys_segs;
    u32 leftblks;
    u8 *buf = (u8 *)dst;
    int ret;

    if (!blkcnt) return MMC_ERR_NONE;

    if (blknr * (blksz / MMC_BLOCK_SIZE) > card->nblks) {
        MSG(ERR, "[SD%d] Out of block range: blknr(%ld) > sd_blknr(%d)\n",
            host->id, blknr, card->nblks);
        return MMC_ERR_INVALID;
    }

    do {
        leftblks = (blkcnt > maxblks) ? maxblks : blkcnt;
        ret = mmc_dev_bread(card, (unsigned long)blknr, leftblks, buf);
        if (ret) return ret;
        blknr  += leftblks;
        buf    += maxblks * blksz;
        blkcnt -= leftblks;
    } while (blkcnt);

    return ret;
}

int mmc_block_write(int dev_num, unsigned long blknr, u32 blkcnt,
                    unsigned long *src)
{
    struct mmc_host *host = mmc_get_host(dev_num);
    struct mmc_card *card = mmc_get_card(dev_num);
    u32 blksz   = host->blklen;
    u32 maxblks = host->max_phys_segs;
    u32 leftblks;
    u8 *buf = (u8 *)src;
    int ret;

    if (!blkcnt) return MMC_ERR_NONE;

    if (blknr * (blksz / MMC_BLOCK_SIZE) > card->nblks) {
        MSG(ERR, "[SD%d] Out of block range: blknr(%ld) > sd_blknr(%d)\n",
            host->id, blknr, card->nblks);
        return MMC_ERR_INVALID;
    }

    do {
        leftblks = (blkcnt > maxblks) ? maxblks : blkcnt;
        ret = mmc_dev_bwrite(card, (unsigned long)blknr, leftblks, buf);
        if (ret) return ret;
        blknr  += leftblks;
        buf    += maxblks * blksz;
        blkcnt -= leftblks;
    } while (blkcnt);

    return ret;
}

/* ======================================================================
 * SANDISK FWID (preloader only)
 * ====================================================================== */

#if defined(MMC_MSDC_DRV_PRELOADER)
void mmc_stuff_buff(u8 *buf)
{
    memset(buf, 0, 512);
    buf[0]  = 0x10; buf[1]  = 0x06; buf[2]  = 0x01; buf[3]  = 0xF0;
    buf[11] = 0xAA; buf[12] = 0xA9; buf[13] = 0x87; buf[14] = 0x74;
    buf[15] = 0x3C; buf[16] = 0x71; buf[17] = 0xFB; buf[18] = 0xD4;
}

int mmc_get_sandisk_fwid(int id, u8 *buf)
{
    struct mmc_host *host = &sd_host[id];
    struct mmc_card *card = &sd_card[id];
    struct mmc_command stop;
    int err = MMC_ERR_NONE;
    u32 status;
    u32 state = 0;

    while (state != 4) {
        err = mmc_send_status(host, card, &status);
        if (err) {
            MSG(ERR, "[SD%d] Fail to send status %d\n", host->id, err);
            return err;
        }
        state = R1_CURRENT_STATE(status);
        MSG(INF, "check card state<%d>\n", state);
        if (state == 5 || state == 6) {
            MSG(INF, "state<%d> need cmd12 to stop\n", state);
            stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
            stop.rsptyp  = RESP_R1B;
            stop.arg     = 0;
            stop.retries = CMD_RETRIES;
            stop.timeout = CMD_TIMEOUT;
            msdc_send_cmd(host, &stop);
            msdc_wait_rsp(host, &stop);
        } else if (state == 7) {
            MSG(INF, "state<%d> card is busy\n", state);
            mdelay(100);
        } else if (state != 4) {
            MSG(ERR, "state<%d> ??? \n", state);
            return MMC_ERR_INVALID;
        }
    }

    mmc_stuff_buff(buf);

#if defined(MSDC_ENABLE_DMA_MODE)
    err = msdc_dma_send_sandisk_fwid(host, buf, MMC_CMD50, 1);
    if (err) {
        MSG(ERR, "[SD%d] Fail to send(CMD50) sandisk fwid %d\n", host->id, err);
        return err;
    }
    err = msdc_dma_send_sandisk_fwid(host, buf, MMC_CMD21, 1);
    if (err) {
        MSG(ERR, "[SD%d] Fail to get(CMD21) sandisk fwid %d\n", host->id, err);
        return err;
    }
#else
    err = msdc_pio_send_sandisk_fwid(host, buf);
    if (err) {
        MSG(ERR, "[SD%d] Fail to send(CMD50) sandisk fwid %d\n", host->id, err);
        return err;
    }
    err = msdc_pio_get_sandisk_fwid(host, buf);
    if (err) {
        MSG(ERR, "[SD%d] Fail to get(CMD21) sandisk fwid %d\n", host->id, err);
        return err;
    }
#endif
    return err;
}
#endif /* MMC_MSDC_DRV_PRELOADER */

/* ======================================================================
 * BOOT-MODE HELPERS
 * ====================================================================== */

#ifdef FEATURE_MMC_BOOT_MODE
void mmc_boot_reset(struct mmc_host *host, int reset)
{
    msdc_emmc_boot_reset(host, reset);
}

int mmc_boot_up(struct mmc_host *host, int ddr, int mode,
                u8 hostbuswidth, int ackdis, u32 *to, u64 size, int read_mode)
{
    int err;

    ERR_EXIT(msdc_emmc_boot_start(host, mmc_host_get_cur_clk(host),
                                  ddr, mode, ackdis, hostbuswidth, size),
             err, MMC_ERR_NONE);
    ERR_EXIT(msdc_emmc_boot_read(host, size, to, read_mode), err, MMC_ERR_NONE);
exit:
    msdc_emmc_boot_stop(host);
    return err;
}
#endif /* FEATURE_MMC_BOOT_MODE */

/* ======================================================================
 * CARD INITIALISATION (mem card / SD / eMMC)
 * ====================================================================== */

int mmc_init_mem_card(struct mmc_host *host, struct mmc_card *card, u32 ocr)
{
    int err, id = host->id;
#if defined(FEATURE_MMC_UHS1)
    int s18a = 0;
#endif

    if (ocr & 0x7F) {
        MSG(INF, "card claims to support voltages "
            "below the defined range. These will be ignored.\n");
        ocr &= ~0x7F;
    }

    ocr = host->ocr = mmc_select_voltage(host, ocr);
    if (!host->ocr) { err = MMC_ERR_FAILED; goto out; }

    mmc_go_idle(host);

    if (mmc_card_sd(card))
        err = mmc_send_if_cond(host, ocr);

    ocr |= (1 << 30);

#if defined(FEATURE_MMC_UHS1)
    if (!err) {
        if (host->caps & MMC_CAP_SD_UHS1)
            ocr |= ((1 << 28) | (1 << 24));
#ifdef MMC_CARD_HAS_VERSION
        card->version = SD_VER_20;
#endif
    } else {
#ifdef MMC_CARD_HAS_VERSION
        card->version = SD_VER_10;
#endif
    }
#else
    /* suppress unused-variable warning for err from mmc_send_if_cond */
    (void)err;
#ifdef MMC_CARD_HAS_VERSION
    card->version = SD_VER_10;
#endif
#endif

    if (mmc_card_sd(card))
        err = mmc_send_app_op_cond(host, ocr, &card->ocr);
    else
        err = mmc_send_op_cond(host, ocr, &card->ocr);

    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Fail in SEND_OP_COND cmd\n", id);
        goto out;
    }

    card->state |= ((card->ocr >> 30) & 0x1) ? MMC_STATE_HIGHCAPS : 0;

#if defined(FEATURE_MMC_UHS1)
    s18a = (card->ocr >> 24) & 0x1;
    printf("[SD%d] ocr = 0x%X, card->ocr=0x%X, s18a = %d\n",
           id, ocr, card->ocr, s18a);
    if (s18a) {
#ifdef MMC_CARD_HAS_VERSION
        card->version = SD_VER_30;
#endif
        err = mmc_switch_volt(host, card);
        if (err != MMC_ERR_NONE) {
            MSG(ERR, "[SD%d] Fail in SWITCH_VOLT cmd\n", id);
            goto out;
        }
    }
#endif

    err = mmc_all_send_cid(host, card->raw_cid);
    if (err != MMC_ERR_NONE) {
        MSG(ERR, "[SD%d] Fail in SEND_CID cmd\n", id);
        goto out;
    }
    mmc_decode_cid(card);

    if (mmc_card_mmc(card))
        card->rca = 0x1;

    err = mmc_send_relative_addr(host, card, &card->rca);
    if (err != MMC_ERR_NONE) {
        MSG(ERR, "[SD%d] Fail in SEND_RCA cmd\n", id);
        goto out;
    }

    err = mmc_read_csds(host, card);
    if (err != MMC_ERR_NONE) {
        MSG(ERR, "[SD%d] Fail in SEND_CSD cmd\n", id);
        goto out;
    }

    err = mmc_decode_csd(card);
    if (err != MMC_ERR_NONE) {
        MSG(ERR, "[SD%d] Fail in decode csd\n", id);
        goto out;
    }

    err = mmc_select_card(host, card);
    if (err != MMC_ERR_NONE) {
        MSG(ERR, "[SD%d] Fail in select card cmd\n", id);
        goto out;
    }

    if (mmc_card_sd(card)) {
#if !defined(FEATURE_MMC_SLIM)
        err = mmc_read_scrs(host, card);
        if (err != MMC_ERR_NONE) {
            MSG(ERR, "[SD%d] Fail in SEND_SCR cmd\n", id);
            goto out;
        }
#endif
        if ((card->csd.cmdclass & CCC_SWITCH) &&
            (mmc_read_switch(host, card) == MMC_ERR_NONE)) {
            do {
#if defined(FEATURE_MMC_UHS1)
                if (s18a && (host->caps & MMC_CAP_SD_UHS1)) {
                    unsigned int freq, uhs_mode, drv_type, max_curr;
                    freq = (host->f_max < card->sw_caps.hs_max_dtr) ?
                           host->f_max : card->sw_caps.hs_max_dtr;

                    if (freq > 100000000)
                        uhs_mode = MMC_SWITCH_MODE_SDR104;
                    else if (freq > 50000000)
                        uhs_mode = (card->sw_caps.ddr && (host->caps & MMC_CAP_DDR)) ?
                                   MMC_SWITCH_MODE_DDR50 : MMC_SWITCH_MODE_SDR50;
                    else if (freq > 25000000)
                        uhs_mode = MMC_SWITCH_MODE_SDR25;
                    else
                        uhs_mode = MMC_SWITCH_MODE_SDR12;

                    drv_type = MMC_SWITCH_MODE_DRV_TYPE_B;
                    max_curr = MMC_SWITCH_MODE_CL_200MA;

                    if (mmc_switch_drv_type(host, card, drv_type) == MMC_ERR_NONE &&
                        mmc_switch_max_cur(host, card, max_curr) == MMC_ERR_NONE &&
                        mmc_switch_uhs1(host, card, uhs_mode) == MMC_ERR_NONE) {
                        break;
                    } else {
                        mmc_switch_drv_type(host, card, MMC_SWITCH_MODE_DRV_TYPE_B);
                        mmc_switch_max_cur(host, card, MMC_SWITCH_MODE_CL_200MA);
                    }
                }
#endif
                if (host->caps & MMC_CAP_SD_HIGHSPEED) {
                    mmc_switch_hs(host, card);
                    break;
                }
            } while (0);
        }

        mmc_set_bus_width(host, card, HOST_BUS_WIDTH_4);

        card->maxhz = (unsigned int)-1;
        if (mmc_card_highspeed(card) || mmc_card_uhs1(card)) {
            if (card->maxhz > card->sw_caps.hs_max_dtr)
                card->maxhz = card->sw_caps.hs_max_dtr;
        } else if (card->maxhz > card->csd.max_dtr) {
            card->maxhz = card->csd.max_dtr;
        }

    } else {
        /* eMMC path */
        mmc_card_set_backyard(card);
        mmc_set_bus_width(host, card, HOST_BUS_WIDTH_8);

        if ((host->caps & MMC_CAP_EMMC_HS200) && !(host->ocr_avail & 0x80)) {
            host->caps &= ~MMC_CAP_EMMC_HS200;
            MSG(WRN, "[SD%d] can not switch to HS200:"
                "Host voltage not support!\n", id);
        }

        err = mmc_read_ext_csd(host, card);
        if (err != MMC_ERR_NONE) {
            MSG(ERR, "[SD%d] Fail in SEND_EXT_CSD cmd\n", id);
            goto out;
        }

#if defined(MSDC0_EMMC50_SUPPORT)
        if ((card->ext_csd.hs_max_dtr > MSDC_52M_SCLK) &&
            (host->caps & MMC_CAP_EMMC_HS400)) {
            err = mmc_set_blk_length(host, MMC_BLOCK_SIZE);

            if (err == MMC_ERR_NONE)
                err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
                                 EXT_CSD_HS_TIMING, 2);
            else
                MSG(ERR, "[SD%d] Fail in set blklen cmd, card state=0x%x\n",
                    id, card->state);

            if (err == MMC_ERR_NONE)
                err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
                                 EXT_CSD_HS_TIMING, 1);
            else
                MSG(ERR, "[SD%d] Switch to HS200 mode failed!\n", host->id);

            if (err == MMC_ERR_NONE)
                err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
                                 EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_8_DDR);
            else
                MSG(ERR, "[SD%d] Switch to High-Speed mode failed!\n", host->id);

            if (err == MMC_ERR_NONE)
                err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
                                 EXT_CSD_HS_TIMING, 3);
            else
                MSG(ERR, "[SD%d] Switch to DDR mode failed!\n", id);

            if (err == MMC_ERR_NONE) {
                printf("[SD%d] Switch to HS400 mode!\n", host->id);
                mmc_card_set_hs400(card);
            } else {
                MSG(ERR, "[SD%d] Switch to HS400 mode failed!\n", host->id);
            }
        } else
#endif
        if ((card->ext_csd.hs_max_dtr > 52000000) &&
            (host->caps & MMC_CAP_EMMC_HS200)) {
            err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
                             EXT_CSD_HS_TIMING, 2);
            if (err == MMC_ERR_NONE) {
                printf("[SD%d] Switch to HS200 mode!\n", host->id);
                mmc_card_set_hs200(card);
            }
        } else if ((card->ext_csd.hs_max_dtr != 0) &&
                   (host->caps & MMC_CAP_MMC_HIGHSPEED)) {
            err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
                             EXT_CSD_HS_TIMING, 1);
            if (err == MMC_ERR_NONE) {
                printf("[SD%d] Switch to High-Speed mode!\n", host->id);
                mmc_card_set_highspeed(card);
            }
            if ((host->caps & MMC_CAP_DDR) && card->ext_csd.ddr_support)
                mmc_set_bus_width(host, card, HOST_BUS_WIDTH_8);
        }

        card->maxhz = (unsigned int)-1;
        if (mmc_card_highspeed(card)) {
            card->maxhz = 52000000;
        } else if (mmc_card_hs200(card) || mmc_card_hs400(card)) {
            if (card->maxhz > card->ext_csd.hs_max_dtr)
                card->maxhz = card->ext_csd.hs_max_dtr;
        } else if (card->maxhz > card->csd.max_dtr) {
            card->maxhz = card->csd.max_dtr;
        }
    }

    if (!(mmc_card_mmc(card) && (mmc_card_ddr(card) || mmc_card_hs400(card)))) {
        err = mmc_set_blk_length(host, MMC_BLOCK_SIZE);
        if (err != MMC_ERR_NONE) {
            MSG(ERR, "[SD%d] Fail in set blklen cmd, card state=0x%x\n",
                id, card->state);
            goto out;
        }
    }

    if (mmc_card_sd(card))
        mmc_set_card_detect(host, card, 0);

    if (!mmc_card_sd(card) && mmc_card_blockaddr(card)) {
        card->blklen = MMC_BLOCK_SIZE;
        card->nblks  = card->ext_csd.sectors;
    } else {
        card->blklen = MMC_BLOCK_SIZE;
        card->nblks  = card->csd.capacity << (card->csd.read_blkbits - 9);
    }

    printf("[SD%d] Size: %d MB, Max.Speed: %d kHz, blklen(%d), nblks(%d), ro(%d)\n",
           id,
           ((card->nblks / 1024) * card->blklen) / 1024,
           card->maxhz / 1000,
           card->blklen, card->nblks,
           mmc_card_readonly(card));

    card->ready = 1;

    printf("[%s %d][SD%d] Initialized, %s\n",
           __func__, __LINE__, id,
           mmc_card_sd(card) ? "SD" : "eMMC");

out:
    return err;
}

/* ======================================================================
 * mmc_init_card
 * ====================================================================== */

int mmc_init_card(struct mmc_host *host, struct mmc_card *card)
{
    int err, id = host->id;
    u32 ocr;

    MSG(INF, "[%s]: start\n", __func__);
    memset(card, 0, sizeof(struct mmc_card));
    mmc_prof_init(id, host, card);
    mmc_prof_start();

#ifdef FEATURE_MMC_CARD_DETECT
    if (!msdc_card_avail(host)) {
        err = MMC_ERR_INVALID;
        goto out;
    }
#endif

    mmc_card_set_present(card);
    mmc_card_set_host(card, host);
    mmc_card_set_unknown(card);

    mmc_go_idle(host);
    mmc_send_if_cond(host, host->ocr_avail);

    err = mmc_send_app_op_cond(host, 0, &ocr);
    if (err != MMC_ERR_NONE) {
        err = mmc_send_op_cond(host, 0, &ocr);
        if (err != MMC_ERR_NONE) {
            MSG(ERR, "[SD%d] Fail in MMC_CMD_SEND_OP_COND/"
                "SD_ACMD_SEND_OP_COND cmd\n", id);
            goto out;
        }
        mmc_card_set_mmc(card);
    } else {
        mmc_card_set_sd(card);
    }

    err = mmc_init_mem_card(host, card, ocr);
    if (err) goto out;

    printf("before cur_bus_clk(%d)\n", mmc_host_get_cur_clk(host));
    mmc_set_clock(host, mmc_card_ddr(card), card->maxhz);
    printf("cur_bus_clk(%d)\n", mmc_host_get_cur_clk(host));

#if defined(FEATURE_MMC_UHS1)
    mmc_tune_timing(host, card);
#endif

out:
    mmc_prof_stop();
    mmc_prof_update(mmc_prof_card_init, (ulong)id, (void *)err);

    if (err) {
        MSG(ERR, "[%s]: failed, err=%d\n", __func__, err);
        return err;
    }

    host->card = card;

    if (mmc_card_hs400(card))
        mmc_set_clock(host, mmc_card_ddr(card), card->maxhz);

    printf("[%s]: finish successfully\n", __func__);
    return 0;
}

/* ======================================================================
 * mmc_init_host  —  matches the mt6589 header: (host, id) only
 * ====================================================================== */

int mmc_init_host(struct mmc_host *host, int id)
{
    memset(host, 0, sizeof(struct mmc_host));
    /* mt6589 msdc_init() takes no arguments */
    return msdc_init();
}

/* ======================================================================
 * SELF-TEST (preloader test build only)
 * ====================================================================== */

#ifdef MTK_MSDC_PL_TEST
#define PL_MMC_TEST_SIZE  (8 * 512)
unsigned char g_mmc_buf[PL_MMC_TEST_SIZE + 1];

static void emmc_r_w_compare_test(void)
{
    unsigned int i;

    MSG(INF, "[PL][%s:%d]eMMC simp test of user partition start\n",
        __func__, __LINE__);

    for (i = 0; i < PL_MMC_TEST_SIZE; i++) g_mmc_buf[i] = 0x5a;

    mmc_block_write(0, 0x40000000 / 512, PL_MMC_TEST_SIZE / 512,
                    (unsigned long *)g_mmc_buf);
    MSG(INF, "[PL][%s:%d] finish write user partition\n", __func__, __LINE__);

    for (i = 0; i < PL_MMC_TEST_SIZE; i++) g_mmc_buf[i] = 0x0;

    mmc_block_read(0, 0x40000000 / 512, PL_MMC_TEST_SIZE / 512,
                   (unsigned long *)g_mmc_buf);
    MSG(INF, "[PL][%s:%d] finish read user partition\n", __func__, __LINE__);

    for (i = 0; i < PL_MMC_TEST_SIZE; i++) {
        if (g_mmc_buf[i] != 0x5a) {
            MSG(ERR, "[PL][%s:%d]mmc simple r/w compare failed"
                "(%d is %d, not 0x5a)\n",
                __func__, __LINE__, i, g_mmc_buf[i]);
            break;
        }
    }

    for (i = 0; i < PL_MMC_TEST_SIZE; i++) g_mmc_buf[i] = 0x0;

    mmc_bread_boot(NULL, 0x0 / 512, PL_MMC_TEST_SIZE / 512, g_mmc_buf);
    MSG(INF, "[PL][%s:%d] finish read boot partition\n", __func__, __LINE__);
    MSG(INF, "[PL][%s:%d]eMMC simp test end\n", __func__, __LINE__);
}
#endif /* MTK_MSDC_PL_TEST */

/* ======================================================================
 * mmc_init  —  matches the mt6589 header: (id) only
 * ====================================================================== */

int mmc_init(int id)
{
    int err = MMC_ERR_NONE;
    struct mmc_host *host;
    struct mmc_card *card;

    BUG_ON(id >= NR_MMC);

    host = &sd_host[id];
    card = &sd_card[id];

    err = mmc_init_host(host, id);
    if (err == MMC_ERR_NONE) {
        MSG(INF, "[%s]: msdc%d start mmc_init_card()\n", __func__, id);
        err = mmc_init_card(host, card);
    }

#ifdef MTK_EMMC_SUPPORT_OTP
    MSG(INF, "[%s]: msdc%d, use hc erase size\n", __func__, id);
    mmc_set_erase_grp_def(card, 1);
#endif

#ifdef MTK_MSDC_PL_TEST
    MSG(INF, "[%s]: start r/w compare test\n", __func__);
    emmc_r_w_compare_test();
#endif

    return err;
}

/* ======================================================================
 * POWER-ON WRITE PROTECTION (optional feature)
 * ====================================================================== */

#ifdef MTK_EMMC_POWER_ON_WP
int mmc_set_write_prot(struct mmc_host *host, u32 addr)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SET_WRITE_PROT;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg     = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_clr_write_prot(struct mmc_host *host, u32 addr)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_CLR_WRITE_PROT;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg     = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_set_boot_prot(struct mmc_card *card, u8 prot)
{
    int err = MMC_ERR_FAILED;

    if (!mmc_card_mmc(card)) goto out;
    WARN_ON(card->csd.mmca_vsn < CSD_SPEC_VER_4);
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL,
                     EXT_CSD_BOOT_CONFIG_PROT, prot);
out:
    return err;
}

int mmc_send_write_prot_type(struct mmc_card *card, u32 wp_addr, u32 *wp_type)
{
    int err;
    int result = MMC_ERR_NONE;
    struct mmc_command cmd;
    struct mmc_host *host = card->host;
    u8 *buf = (u8 *)wp_type;

    cmd.opcode  = MMC_CMD_SEND_WRITE_PROT_TYPE;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = wp_addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    msdc_reset_tune_counter(host);
    do {
        msdc_set_blklen(host, 8);
        msdc_set_timeout(host, 100000000, 0);

        err = mmc_cmd(host, &cmd);
        if (err != MMC_ERR_NONE) goto out;

        err = msdc_pio_read(host, (u32 *)buf, 8);
        if (err != MMC_ERR_NONE) {
            msdc_abort_handler(host, 1);
            result = msdc_tune_read(host);
        }
    } while (err && result != MMC_ERR_READTUNEFAIL);
    msdc_reset_tune_counter(host);
out:
    return err;
}

int mmc_verify_write_prot_type_by_group(struct mmc_card *card,
    unsigned long blknr, u32 blkcnt, EMMC_WP_TYPE type)
{
    int err = MMC_ERR_FAILED;
    u32 count_wp       = blkcnt / card->wp_size;
    u32 count_wp_group = count_wp / 32;
    u32 count_wp_rest  = count_wp % 32;
    u32 i, j, z;
    u8  status_bit;
    u8  status[8] = {0xFF};

    if      (type == WP_TEMPORARY) status_bit = 0x55;
    else if (type == WP_POWER_ON)  status_bit = 0xAA;
    else if (type == WP_PERMANENT) status_bit = 0xFF;
    else                           status_bit = 0;

    for (i = 0; i < count_wp_group; i++) {
        err = mmc_send_write_prot_type(card,
                  blknr + i * 32 * card->wp_size, (u32 *)status);
        if (err) {
            MSG(ERR, "[%s]: mmc_send_write_prot_type err %d\n", __func__, err);
            goto out;
        }
        for (j = 0; j < 8; j++) {
            if (status[j] != status_bit) { err = MMC_ERR_FAILED; goto out; }
        }
    }

    err = mmc_send_write_prot_type(card,
              blknr + count_wp_group * 32 * card->wp_size, (u32 *)status);
    if (err) {
        MSG(ERR, "[%s]: mmc_send_write_prot_type err %d\n", __func__, err);
        goto out;
    }

    for (z = 0; z < 8; z++) {
        if (count_wp_rest >= 4) {
            if (status[7 - z] != status_bit) { err = MMC_ERR_FAILED; goto out; }
            count_wp_rest -= 4;
        } else if (count_wp_rest >= 1) {
            u8 sel    = status[7 - z] & (0xFF >> (8 - count_wp_rest * 2));
            u8 should = status_bit    >> (8 - count_wp_rest * 2);
            if (sel != should) { err = MMC_ERR_FAILED; goto out; }
            break;
        } else {
            break;
        }
    }
    err = 0;
out:
    return err;
}

int mmc_set_write_protect_by_group(struct mmc_card *card,
    Region partition, unsigned long blknr, u32 blkcnt, EMMC_WP_TYPE type)
{
    int err = MMC_ERR_FAILED;
    u32 count_wp, index;

    if (type == WP_PERMANENT || type == WP_DISABLE || type == WP_TEMPORARY) {
        MSG(ERR, "[%s]: Type Not Support\n", __func__);
        return 0;
    }
    if (type > WP_DISABLE) {
        MSG(ERR, "[%s]:%d type=%d\n", __func__, __LINE__, type);
        return err;
    }
    if (partition != EMMC_PART_USER) {
        MSG(ERR, "[%s]:partition=%d Not Support\n", __func__, partition);
        return err;
    }

    err = mmc_switch_part(EMMC_PART_USER);
    if (err) {
        MSG(ERR, "[%s]: mmc_switch_part err %d\n", __func__, err);
        return err;
    }

    count_wp = blkcnt / card->wp_size;
    MSG(INF, "[%s]: count_wp:%d\n", __func__, count_wp);

    for (index = 0; index < count_wp; index++) {
        err = mmc_set_write_prot(card->host,
                  blknr + (index * card->wp_size));
        if (err) {
            MSG(ERR, "[%s]: mmc_set_write_prot err %d\n", __func__, err);
            return err;
        }
    }

    err = mmc_verify_write_prot_type_by_group(card, blknr, blkcnt, type);
    return err;
}

int mmc_set_boot_wp(struct mmc_card *card, EMMC_WP_TYPE type)
{
    int err = MMC_ERR_FAILED;
    u8  value;

    if (type == WP_DISABLE || type == WP_PERMANENT || type == WP_TEMPORARY)
        return 0;
    if (type > WP_DISABLE) goto out;
    if (!mmc_card_mmc(card)) goto out;

    WARN_ON(card->csd.mmca_vsn < CSD_SPEC_VER_4);
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    if (card->ext_csd.boot_wp & EXT_CSD_BOOT_WP_DIS_PWR_WP) {
        MSG(INF, "[%s]: EXT_CSD_BOOT_WP_DIS_PWR_WP is set\n", __func__);
        goto out;
    }

    value  = card->ext_csd.boot_wp | EXT_CSD_BOOT_WP_EN_PWR_WP;
    value &= ~EXT_CSD_BOOT_WP_WP_SEC_SEL;

    if (card->ext_csd.boot_wp == value) {
        MSG(INF, "[%s]: EXT_CSD_BOOT_WP already set\n", __func__);
    } else {
        err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL,
                         EXT_CSD_BOOT_WP, value);
        if (err) { MSG(INF, "[%s]: mmc_switch err %d\n", __func__, err); goto out; }
    }

    err = mmc_read_ext_csd(card->host, card);
    if (err) { MSG(INF, "[%s]: read ext_csd err %d\n", __func__, err); goto out; }

    MSG(INF, "[%s]: card->ext_csd.boot_wp:%d\n", __func__, card->ext_csd.boot_wp);
out:
    return err;
}

int mmc_set_user_wp(struct mmc_card *card, EMMC_WP_TYPE type,
                    unsigned long blknr, u32 blkcnt, Region partition)
{
    int err = MMC_ERR_FAILED;
    u8  value;

    if (type == WP_DISABLE || type == WP_PERMANENT || type == WP_TEMPORARY)
        return 0;
    if (type > WP_DISABLE) goto out;
    if (!mmc_card_mmc(card)) goto out;

    WARN_ON(card->csd.mmca_vsn < CSD_SPEC_VER_4);
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) goto out;

    if (card->ext_csd.usr_wp & US_PWR_WP_DIS) {
        MSG(ERR, "[%s]: US_PWR_WP_DIS is set\n", __func__);
        goto out;
    }

    value  = card->ext_csd.usr_wp | US_PWR_WP_EN;
    value &= ~US_PERM_WP_EN;

    if (card->ext_csd.usr_wp == value) {
        MSG(ERR, "[%s]: EXT_CSD_USR_WP already set\n", __func__);
    } else {
        err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL,
                         EXT_CSD_USR_WP, value);
        if (err) { MSG(ERR, "[%s]: mmc_switch err %d\n", __func__, err); goto out; }
    }

    err = mmc_read_ext_csd(card->host, card);
    if (err) { MSG(ERR, "[%s]: read ext_csd err %d\n", __func__, err); goto out; }

    MSG(INF, "[%s]: card->ext_csd.usr_wp:%d\n", __func__, card->ext_csd.usr_wp);

    err = mmc_set_write_protect_by_group(card, partition, blknr, blkcnt, type);
    if (err) {
        MSG(ERR, "[%s]: mmc_set_write_protect_by_group err%d\n", __func__, err);
        goto out;
    }

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL,
                     EXT_CSD_USR_WP,
                     card->ext_csd.usr_wp & ~US_PWR_WP_EN);
    if (err) {
        MSG(ERR, "[%s]: restore US_PWR_WP_EN err %d\n", __func__, err);
        goto out;
    }

    err = mmc_read_ext_csd(card->host, card);
    if (err) { MSG(ERR, "[%s]: read ext_csd err %d\n", __func__, err); goto out; }

    MSG(INF, "[%s]: card->ext_csd.usr_wp:%d\n", __func__, card->ext_csd.usr_wp);
out:
    return err;
}

unsigned int mmc_set_write_protect(int dev_num, Region partition,
                                   unsigned long blknr, u32 blkcnt,
                                   EMMC_WP_TYPE type)
{
    int err = MMC_ERR_FAILED;
    struct mmc_card *card = mmc_get_card(dev_num);

    if (type == WP_PERMANENT || type == WP_DISABLE || type == WP_TEMPORARY) {
        MSG(ERR, "[%s]: Type Not Support\n", __func__);
        goto done;
    }
    if (partition != EMMC_PART_USER && partition != EMMC_PART_BOOT1) {
        MSG(ERR, "[%s]:partition=%d Not Support\n", __func__, partition);
        goto done;
    }

    if (type == WP_POWER_ON) {
        if (partition == EMMC_PART_USER) {
            MSG(INF, "wp_size: %d, blknr: %d, blkcnt: %d, nblks: %d\n",
                card->wp_size, blknr, blkcnt, card->nblks);
            if (blknr % card->wp_size || blkcnt % card->wp_size ||
                (blknr + blkcnt > card->nblks)) {
                MSG(ERR, "[%s]: alignment or capacity error\n", __func__);
                goto done;
            }
            err = mmc_set_user_wp(card, WP_POWER_ON, blknr, blkcnt, partition);
            if (err) {
                MSG(ERR, "[%s]: mmc_set_user_wp err%d\n", __func__, err);
                goto done;
            }
        } else if (partition == EMMC_PART_BOOT1) {
            err = mmc_set_boot_wp(card, WP_POWER_ON);
            if (err) {
                MSG(ERR, "[%s]: mmc_set_boot_wp err%d\n", __func__, err);
                goto done;
            }
        }
    }
done:
    printf("[%s]: mmc_set_write_protect result:%d\n", __func__, err);
    return err;
}
#endif /* MTK_EMMC_POWER_ON_WP */

/* ======================================================================
 * CD INTERRUPT POLLING (CTP only)
 * ====================================================================== */

#if defined(MMC_MSDC_DRV_CTP)
int mmc_polling_CD_INT(struct mmc_host *host)
{
    return msdc_polling_CD_interrupt(host);
}
#endif
