#ifndef __PMT_H__
#define __PMT_H__

#include <platform/mt_typedefs.h>

/* PMT (Partition Management Table) for MT6589 */

#define PMT_VERSION         0x00000001
#define PMT_MAGIC           0x4D544450  /* "PMTD" */

/* PMT Entry */
typedef struct {
    u8   name[32];          /* partition name */
    u64  offset;            /* partition offset in bytes */
    u64  size;              /* partition size in bytes */
    u32  flags;             /* partition flags */
    u32  reserved;
} pmt_entry_t;

/* PMT Header */
typedef struct {
    u32  magic;             /* PMT_MAGIC */
    u32  version;           /* PMT_VERSION */
    u32  entry_count;       /* number of entries */
    u32  checksum;          /* checksum */
} pmt_header_t;

/* Function prototypes used in partition_mt.c */
extern void part_init_pmt(unsigned long totalblks, void *dev);

#endif /* __PMT_H__ */
