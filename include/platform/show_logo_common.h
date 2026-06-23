#ifndef __SHOW_LOGO_COMMON_H__
#define __SHOW_LOGO_COMMON_H__

#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>

/*===========================================================================
 * Logo display for MT6589
 *===========================================================================*/

/* Logo display states */
#define LOGO_STATE_NONE         0
#define LOGO_STATE_NORMAL       1
#define LOGO_STATE_CHARGING     2
#define LOGO_STATE_LOW_BATTERY  3

/* Logo structure for MT6589 */
typedef struct {
    unsigned int width;          /* Logo width in pixels */
    unsigned int height;         /* Logo height in pixels */
    unsigned int bpp;            /* Bits per pixel */
    unsigned int size;           /* Total size in bytes */
    unsigned char *data;         /* Pointer to logo data */
} logo_info_t;

/*===========================================================================
 * Function prototypes for MT6589
 *===========================================================================*/

/* Initialize logo display */
extern void show_logo_common_init(void);

/* Show logo */
extern int show_logo_common(unsigned int logo_addr);

/* Show logo from specific address */
extern int show_logo_common_ex(unsigned int logo_addr, unsigned int size);

/* Show battery charging logo */
extern void show_logo_common_charging(void);

/* Show low battery logo */
extern void show_logo_common_low_battery(void);

/* Deinitialize logo display */
extern void show_logo_common_deinit(void);

/* Get logo information */
extern int get_logo_info(logo_info_t *info);

#endif /* __SHOW_LOGO_COMMON_H__ */
