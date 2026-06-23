#include <target/cust_msdc.h>

// ============================================================
// Definitions for MSDC capability
// ============================================================

#ifndef MSDC0_CLKSRC_DEFAULT
#define MSDC0_CLKSRC_DEFAULT 0
#endif

#ifndef MSDC_DRVN_GEAR1
#define MSDC_DRVN_GEAR1 1
#endif

#ifndef MSDC_DRVN_GEAR2
#define MSDC_DRVN_GEAR2 2
#endif

#ifndef MSDC_DRVN_GEAR3
#define MSDC_DRVN_GEAR3 3
#endif

// ============================================================
// MSDC capability structure (C89 style - no field names)
// ============================================================

struct msdc_cust msdc_cap = {
    MSDC0_CLKSRC_DEFAULT,  /* clk_src     - host clock source */
    MSDC_SMPL_RISING,      /* cmd_edge    - command latch edge */
    MSDC_SMPL_RISING,      /* data_edge   - data latch edge */
    MSDC_DRVN_GEAR1,       /* clk_drv     - clock pad driving */
    MSDC_DRVN_GEAR1,       /* cmd_drv     - command pad driving */
    MSDC_DRVN_GEAR1,       /* dat_drv     - data pad driving */
    8,                     /* data_pins   - data pins (1, 4, or 8) */
    0,                     /* data_offset - data address offset */
    MSDC_HIGHSPEED | MSDC_DDR, /* flags    - hardware capability flags */
};
