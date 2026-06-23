#ifndef CUST_MSDC_H
#define CUST_MSDC_H

#define MSDC_CD_PIN_EN      (1 << 0)  /* card detection pin is wired   */
#define MSDC_WP_PIN_EN      (1 << 1)  /* write protection pin is wired */
#define MSDC_RST_PIN_EN     (1 << 2)  /* emmc reset pin is wired       */
#define MSDC_SDIO_IRQ       (1 << 3)  /* use internal sdio irq (bus)   */
#define MSDC_EXT_SDIO_IRQ   (1 << 4)  /* use external sdio irq         */
#define MSDC_REMOVABLE      (1 << 5)  /* removable slot                */
#define MSDC_SYS_SUSPEND    (1 << 6)  /* suspended by system           */
#define MSDC_HIGHSPEED      (1 << 7)  /* high-speed mode support       */
#define MSDC_UHS1           (1 << 8)  /* uhs-1 mode support            */
#define MSDC_DDR            (1 << 9)  /* ddr mode support              */

#define MSDC_SMPL_RISING        (0)
#define MSDC_SMPL_FALLING       (1)

/* Add missing definitions */
#define MSDC0_CLKSRC_DEFAULT    0
#define MSDC_DRVN_GEAR1         1
#define MSDC_DRVN_DONT_CARE     0
#define MSDC_MAX_NUM            1

typedef enum {
    MSDC_CLKSRC_200MHZ = 0
} clk_source_t;

struct msdc_cust {
    unsigned char  clk_src;           /* host clock source             */
    unsigned char  cmd_edge;          /* command latch edge            */
    unsigned char  data_edge;         /* data latch edge               */
    unsigned char  clk_drv;           /* clock pad driving             */
    unsigned char  cmd_drv;           /* command pad driving           */
    unsigned char  dat_drv;           /* data pad driving              */
    unsigned char  data_pins;         /* data pins                     */
    unsigned int   data_offset;       /* data address offset           */    
    unsigned int   flags;             /* hardware capability flags     */
};

/* Define msdc_cap directly in the header */
struct msdc_cust msdc_cap = {
    .clk_src      = MSDC0_CLKSRC_DEFAULT,
    .cmd_edge     = MSDC_SMPL_RISING,
    .data_edge    = MSDC_SMPL_RISING,
    .clk_drv      = MSDC_DRVN_GEAR1,
    .cmd_drv      = MSDC_DRVN_GEAR1,
    .dat_drv      = MSDC_DRVN_GEAR1,
    .data_pins    = 8,
    .data_offset  = 0,
    .flags        = MSDC_CD_PIN_EN | MSDC_REMOVABLE,
};

#endif /* end of _MSDC_CUST_H_ */
