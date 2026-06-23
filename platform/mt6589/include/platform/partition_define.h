#ifndef __PARTITION_DEFINE_H__
#define __PARTITION_DEFINE_H__

// ============================================================
// Partition Definitions for MT6589
// ============================================================

/* Partition Magic Number */
#define PART_MAGIC          0x5880

/* Partition flags */
#define PART_FLAG_LEFT      0x01
#define PART_FLAG_NONE      0x00

/* Partition types */
#define TYPE_NORMAL         0
#define TYPE_LOW            1

/* Block sizes */
#define BLK_SIZE            512
#define BLK_BITS            9

/* Partition Name Definitions */
#define PART_LOGO           "logo"
#define PART_KERNEL         "kernel"
#define PART_ROOTFS         "rootfs"
#define PART_RECOVERY       "recovery"
#define PART_MISC           "misc"
#define PART_PARA           "para"
#define PART_CUST           "cust"
#define PART_SYSTEM         "system"
#define PART_DATA           "data"
#define PART_CACHE          "cache"
#define PART_PROTECT_F      "protect_f"
#define PART_PROTECT_S      "protect_s"
#define PART_SEC_RO         "sec_ro"
#define PART_SEC_CFG        "sec_cfg"
#define PART_UBOOT          "uboot"
#define PART_BOOTIMG        "bootimg"
#define PART_RECOVERY_IMG   "recoveryimg"
#define PART_SECURE         "secure"
#define PART_LOGO_BIN       "logo.bin"
#define PART_UBOOT_BIN      "uboot.bin"
#define PART_MTDBLK         "mtdblk"

#endif /* __PARTITION_DEFINE_H__ */
