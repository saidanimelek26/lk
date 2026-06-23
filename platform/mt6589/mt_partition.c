#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <printf.h>
#include <debug.h>
#include <err.h>
#include <target.h>

#include <platform/mt_typedefs.h>
#include <platform/errno.h>

// ============================================================
// DEFINITIONS MISSING IN NEW LK
// ============================================================

#define BLK_SIZE            512
#define BLK_BITS            9       // 2^9 = 512
#define GB                  (1024*1024*1024)
#define PART_MAX_COUNT      32
#define PART_FLAG_LEFT      0x01
#define PART_MISC           "misc"

// Convert bytes to block number
#define BLK_NUM(x)          ((x) / BLK_SIZE)

// ============================================================
// TYPE DEFINITIONS (matching old LK)
// ============================================================

typedef unsigned char uchar;
typedef unsigned long long u64;
typedef unsigned long ulong;

typedef struct {
    unsigned long startblk;
    unsigned long blknum;
    unsigned long flags;
    unsigned long type;
    char name[32];
} part_t;

typedef struct {
    int (*read)(void *dev, unsigned long start, unsigned char *buf, unsigned long size);
    int (*write)(void *dev, unsigned long start, unsigned char *buf, unsigned long size);
    int (*erase)(void *dev, unsigned long start, unsigned long size);
    int id;
    int init;
    int (*init_dev)(int id);
    void *blkdev;  // block_dev_desc_t *
} part_dev_t;

typedef struct {
    char *fb_name;
    char *r_name;
    char *partition_type;
    int is_support_dl;
    int is_support_erase;
    int partition_idx;
} pt_resident;

typedef struct {
    unsigned long lba;  // total blocks
    // other block device fields...
} block_dev_desc_t;

// ============================================================
// EXTERNAL DECLARATIONS (defined in other files)
// ============================================================

extern part_t partition_layout[];
extern pt_resident g_part_name_map[];
extern pt_resident lastest_part[];
extern int target_is_emmc_boot(void);

// ============================================================
// STATIC VARIABLES
// ============================================================

static part_dev_t *mt_part_dev;
static uchar *mt_part_buf;
part_t tempart;

// ============================================================
// DUMP FUNCTION
// ============================================================

void mt_part_dump(void)
{
    part_t *part = &partition_layout[0];
    
    printf("\nPart Info from compiler.(1blk=%dB):\n", BLK_SIZE);
    while (part->name[0]) {
        printf("[0x%016llx-0x%016llx] (%.8ld blocks): \"%s\"\n", 
               (u64)part->startblk * BLK_SIZE, 
               (u64)(part->startblk + part->blknum) * BLK_SIZE - 1, 
               part->blknum, part->name);
        part++;
    }
    printf("\n");
}

// ============================================================
// INIT FUNCTION
// ============================================================

void mt_part_init(unsigned long totalblks)
{
    part_t *part = &partition_layout[0];
    unsigned long lastblk;

    if (!totalblks) return;

    // Update the number of blks of first part
    if (totalblks <= part->blknum)
        part->blknum = totalblks;

    totalblks -= part->blknum;
    if (part->type == 1) { // TYPE_LOW
        lastblk = part->startblk + part->blknum * 2;
    } else {
        lastblk = part->startblk + part->blknum;
    }

    while (totalblks) {
        part++;
        if (!part->name[0])
            break;

        if (part->flags & PART_FLAG_LEFT || totalblks <= part->blknum)
            part->blknum = totalblks;

        part->startblk = lastblk;
        totalblks -= part->blknum;
        if (part->type == 1) { // TYPE_LOW
            lastblk = part->startblk + part->blknum * 2;
        } else {
            lastblk = part->startblk + part->blknum;
        }
        printf("[%s] part->startblk %x\n", __FUNCTION__, (unsigned int)part->startblk);
    }
}

// ============================================================
// GET PARTITION FUNCTION
// ============================================================

part_t* mt_part_get_partition(char *name)
{
    int index = 0;
    part_t *part = &partition_layout[0];
    
    printf("[%s] %s\n", __FUNCTION__, name);
    
    if (!strcmp(name, "para"))
        name = PART_MISC;
    
    printf("[pmt]%s\n", name);
    
    while (part->name[0]) {
        printf("[pmt]part name: %s\n", (char *)part->name);
        if (!strncmp(name, (char *)part->name, strlen(name))) {
            // Found match
            tempart.name[0] = '\0';
            strncpy((char *)tempart.name, (char *)part->name, sizeof(tempart.name) - 1);
            tempart.flags = part->flags;
            
            // Try to get from latest part table if available
            tempart.startblk = BLK_NUM(lastest_part[index].offset);
            tempart.blknum = BLK_NUM(lastest_part[index].size);
            
            printf("[%s] %lx\n", __FUNCTION__, tempart.startblk);
            return &tempart;
        }
        index++;
        part++;
    }
    return NULL;
}

// ============================================================
// GENERIC READ FUNCTIONS (EMMC)
// ============================================================

#ifdef MTK_EMMC_SUPPORT

int mt_part_generic_read(part_dev_t *dev, u64 src, uchar *dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = (block_dev_desc_t *)dev->blkdev;
    u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

    if (!blkdev) {
        printf("No block device registered\n");
        return -ENODEV;
    }

    if (size == 0) 
        return 0;

    end = src + size;
    
    part_start    = src & ((u64)BLK_SIZE - 1);
    part_end      = end & ((u64)BLK_SIZE - 1);
    aligned_start = src & ~((u64)BLK_SIZE - 1);
    aligned_end   = end & ~((u64)BLK_SIZE - 1);

    if (part_start) {
        blknr = aligned_start >> BLK_BITS;
        part_len = BLK_SIZE - part_start;
        if (part_len > (u64)size) {
            part_len = size;
        }
        // Read one block
        if (blkdev->block_read(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1)
            return -EIO;
        memcpy(dst, buf + part_start, part_len);
        dst += part_len;
        src += part_len;
    }

    aligned_start = src & ~((u64)BLK_SIZE - 1);
    blknr = aligned_start >> BLK_BITS;
    blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

    if (blkcnt != 0) {
        if (blkdev->block_read(dev_id, blknr, blkcnt, (unsigned long *)(dst), 0) != blkcnt)
            return -EIO;
    }
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

    if (part_end && src < end) {
        blknr = aligned_end >> BLK_BITS;
        if (blkdev->block_read(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1)
            return -EIO;
        memcpy(dst, buf, part_end);
    }
    return size;
}

static int mt_part_generic_write(part_dev_t *dev, uchar *src, u64 dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = (block_dev_desc_t *)dev->blkdev;
    u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

    if (!blkdev) {
        printf("No block device registered\n");
        return -ENODEV;
    }

    if (size == 0) 
        return 0;

    end = dst + size;
    
    part_start    = dst & ((u64)BLK_SIZE - 1);
    part_end      = end & ((u64)BLK_SIZE - 1);
    aligned_start = dst & ~((u64)BLK_SIZE - 1);
    aligned_end   = end & ~((u64)BLK_SIZE - 1);

    if (part_start) {
        blknr = aligned_start >> BLK_BITS;
        part_len = BLK_SIZE - part_start;
        if (blkdev->block_read(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1)
            return -EIO;
        memcpy(buf + part_start, src, part_len);
        if (blkdev->block_write(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1)
            return -EIO;
        dst += part_len;
        src += part_len;
    }

    aligned_start = dst & ~((u64)BLK_SIZE - 1);
    blknr = aligned_start >> BLK_BITS;
    blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

    if (blkcnt != 0) {
        if (blkdev->block_write(dev_id, blknr, blkcnt, (unsigned long *)(src), 0) != blkcnt)
            return -EIO;
    }
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

    if (part_end && dst < end) {
        blknr = aligned_end >> BLK_BITS;
        if (blkdev->block_read(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1) {
            return -EIO;
        }
        memcpy(buf, src, part_end);
        if (blkdev->block_write(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1) {
            return -EIO;
        }
    }
    return size;
}

#else // NAND version

int mt_part_generic_read(part_dev_t *dev, ulong src, uchar *dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = (block_dev_desc_t *)dev->blkdev;
    ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

    if (!blkdev) {
        printf("No block device registered\n");
        return -ENODEV;
    }

    if (size == 0) 
        return 0;

    end = src + size;
    
    part_start    = src & (BLK_SIZE - 1);
    part_end      = end & (BLK_SIZE - 1);
    aligned_start = src & ~(BLK_SIZE - 1);
    aligned_end   = end & ~(BLK_SIZE - 1);

    if (part_start) {
        blknr = aligned_start >> BLK_BITS;
        part_len = BLK_SIZE - part_start;
        if (part_len > (ulong)size) {
            part_len = size;
        }
        if (blkdev->block_read(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1)
            return -EIO;
        memcpy(dst, buf + part_start, part_len);
        dst += part_len;
        src += part_len;
    }

    aligned_start = src & ~(BLK_SIZE - 1);
    blknr = aligned_start >> BLK_BITS;
    blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

    if (blkcnt != 0) {
        if (blkdev->block_read(dev_id, blknr, blkcnt, (unsigned long *)(dst), 0) != blkcnt)
            return -EIO;
    }
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

    if (part_end && src < end) {
        blknr = aligned_end >> BLK_BITS;
        if (blkdev->block_read(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1)
            return -EIO;
        memcpy(dst, buf, part_end);
    }
    return size;
}

static int mt_part_generic_write(part_dev_t *dev, uchar *src, ulong dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = (block_dev_desc_t *)dev->blkdev;
    ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

    if (!blkdev) {
        printf("No block device registered\n");
        return -ENODEV;
    }

    if (size == 0) 
        return 0;

    end = dst + size;
    
    part_start    = dst & (BLK_SIZE - 1);
    part_end      = end & (BLK_SIZE - 1);
    aligned_start = dst & ~(BLK_SIZE - 1);
    aligned_end   = end & ~(BLK_SIZE - 1);

    if (part_start) {
        blknr = aligned_start >> BLK_BITS;
        part_len = BLK_SIZE - part_start;
        if (blkdev->block_read(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1)
            return -EIO;
        memcpy(buf + part_start, src, part_len);
        if (blkdev->block_write(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1)
            return -EIO;
        dst += part_len;
        src += part_len;
    }

    aligned_start = dst & ~(BLK_SIZE - 1);
    blknr = aligned_start >> BLK_BITS;
    blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

    if (blkcnt != 0) {
        if (blkdev->block_write(dev_id, blknr, blkcnt, (unsigned long *)(src), 0) != blkcnt)
            return -EIO;
    }
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

    if (part_end && dst < end) {
        blknr = aligned_end >> BLK_BITS;
        if (blkdev->block_read(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1) {
            return -EIO;
        }
        memcpy(buf, src, part_end);
        if (blkdev->block_write(dev_id, blknr, 1, (unsigned long *)buf, 0) != 1) {
            return -EIO;
        }
    }
    return size;
}

#endif

// ============================================================
// DEVICE REGISTRATION
// ============================================================

int mt_part_register_device(part_dev_t *dev)
{
    printf("[mt_part_register_device]\n");
    
    if (!mt_part_dev) {
        if (!dev->read)
            dev->read = mt_part_generic_read;
        if (!dev->write)
            dev->write = mt_part_generic_write;
        mt_part_dev = dev;

        // Allocate buffer
        mt_part_buf = (uchar*)malloc(BLK_SIZE * 2);
        printf("[mt_part_register_device]malloc %d : %x\n", (BLK_SIZE * 2), (unsigned int)mt_part_buf);

        // Initialize partition table
        part_init_pmt((unsigned long)((block_dev_desc_t *)dev->blkdev)->lba, dev);
    }
    return 0;
}

// ============================================================
// GET DEVICE
// ============================================================

part_dev_t *mt_part_get_device(void)
{
    if (mt_part_dev && !mt_part_dev->init && mt_part_dev->init_dev) {
        mt_part_dev->init_dev(mt_part_dev->id);
        mt_part_dev->init = 1;
    }
    return mt_part_dev;
}

// ============================================================
// FASTBOOT PARTITION FUNCTIONS
// ============================================================

unsigned int write_partition(unsigned size, unsigned char *partition)
{
    // Placeholder
    return 0;
}

int partition_get_index(const char *name)
{
    int index;

    for (index = 0; index < PART_MAX_COUNT; index++) {
        if (!strcmp(name, g_part_name_map[index].fb_name)) {
            printf("[%s]find %s %s index %d\n", __FUNCTION__, name, 
                   g_part_name_map[index].r_name, g_part_name_map[index].partition_idx);
            return g_part_name_map[index].partition_idx;
        }
    }
    return -1;
}

u64 partition_get_offset(int index)
{
    part_t *p = mt_part_get_partition(g_part_name_map[index].r_name);
    if (p == NULL)
        return -1;
    return (u64)p->startblk * BLK_SIZE;
}

u64 partition_get_size(int index)
{
    part_t *p = mt_part_get_partition(g_part_name_map[index].r_name);
    if (p == NULL)
        return -1;
    return (u64)p->blknum * BLK_SIZE;
}

int partition_get_type(int index, char **p_type)
{
    *p_type = g_part_name_map[index].partition_type;
    return 0;
}

int partition_get_name(int index, char **p_name)
{
    *p_name = g_part_name_map[index].fb_name;
    return 0;
}

int is_support_erase(int index)
{
    return g_part_name_map[index].is_support_erase;
}

int is_support_flash(int index)
{
    return g_part_name_map[index].is_support_dl;
}

unsigned long partition_reserve_size(void)
{
    unsigned long size = 0;
    
    if (target_is_emmc_boot()) {
#ifdef MTK_EMMC_SUPPORT_OTP
        size += PART_SIZE_OTP;
#endif
        size += PART_SIZE_BMTPOOL * (128 * 1024);
    }

    return size;
}
