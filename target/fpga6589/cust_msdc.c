#include <target/cust_msdc.h>

#ifndef MSDC_MAX_NUM
#define MSDC_MAX_NUM 1
#endif

#ifndef MSDC0_CLKSRC_DEFAULT
#define MSDC0_CLKSRC_DEFAULT 0
#endif

#ifndef MSDC_DRVN_GEAR1
#define MSDC_DRVN_GEAR1 1
#endif

struct msdc_cust msdc_cap[MSDC_MAX_NUM] = {
    {
        MSDC0_CLKSRC_DEFAULT,  /* clk_src */
        MSDC_SMPL_RISING,      /* cmd_edge */
        MSDC_SMPL_RISING,      /* data_edge */
        MSDC_DRVN_GEAR1,       /* clk_drv */
        MSDC_DRVN_GEAR1,       /* cmd_drv */
        MSDC_DRVN_GEAR1,       /* dat_drv */
        8,                     /* data_pins */
        0,                     /* data_offset */
        MSDC_HIGHSPEED | MSDC_DDR, /* flags */
    }
};
