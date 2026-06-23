#ifndef MSDC_H
#define MSDC_H

#include <target/cust_msdc.h>
#include <platform/msdc_types.h>

/* MSDC register definitions */
#define MSDC_CFG            0x0000
#define MSDC_IOCON          0x0004
#define MSDC_PS             0x0008
#define MSDC_INT            0x000C
#define MSDC_INTEN          0x0010
#define MSDC_FIFOCS         0x0014
#define MSDC_TXDATA         0x0018
#define MSDC_RXDATA         0x001C

/* MSDC timing definitions */
#define MSDC_DEFAULT_TIMEOUT    1000000

/* REMOVE this line - it's already defined in cust_msdc.h */
/* #define MSDC_MAX_NUM            (1) */

/* MSDC function prototypes */
int msdc_init(void);
int msdc_read(unsigned int blk, unsigned int blkcnt, unsigned char *buf);
int msdc_write(unsigned int blk, unsigned int blkcnt, unsigned char *buf);
int msdc_get_capacity(unsigned int *total_blocks);

#endif /* MSDC_H */
