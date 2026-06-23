#ifndef __PMT_H__
#define __PMT_H__

#include <platform/mt_typedefs.h>

/* PMT (Partition Management Table) for MT6589 */

#define PMT_VERSION         0x00000001
#define PMT_MAGIC           0x4D544450  /* "PMTD" */

/* PMT Entry */
typedef struct {
    u8   name[32];
    u64  offset;
    u64  size;
    u32  flags;
    u32  reserved;
} pmt_entry_t;

/* PMT Header */
typedef struct {
    u32  magic;
    u32  version;
    u32  entry_count;
    u32  checksum;
} pmt_header_t;

/* Forward declaration of part_dev_t */
typedef struct part_dev part_dev_t;

/* ============================================================
   FUNCTION PROTOTYPES
   ============================================================ */

/* PMT initialization function - matches partition_mt.c */
extern void part_init_pmt(unsigned long totalblks, part_dev_t *dev);

#endif /* __PMT_H__ */
