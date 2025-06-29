LOCAL_DIR := $(GET_LOCAL_DIR)
TARGET := k37mv1_bsp
MODULES += app/mt_boot \
           dev/lcm
MTK_EMMC_SUPPORT = yes
DEFINES += MTK_NEW_COMBO_EMMC_SUPPORT
MTK_KERNEL_POWER_OFF_CHARGING = yes
MTK_PUMP_EXPRESS_SUPPORT := no
MTK_LCM_PHYSICAL_ROTATION = 0
CUSTOM_LK_LCM="s6d78a0_qhd_dsi_vdo"
#hx8392a_dsi_cmd = yes
MTK_SECURITY_SW_SUPPORT = yes
MTK_VERIFIED_BOOT_SUPPORT = yes
MTK_SEC_FASTBOOT_UNLOCK_SUPPORT = yes
DEBUG := 0
BOOT_LOGO:=hd720
#DEFINES += WITH_DEBUG_DCC=1
#DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
#DEFINES += MACH_FPGA=y
#DEFINES += SB_LK_BRINGUP=y
DEFINES += MTK_GPT_SCHEME_SUPPORT
#DEFINES += MACH_FPGA_NO_DISPLAY=y
MTK_EFUSE_DOWNGRADE = no
MTK_GOOGLE_TRUSTY_SUPPORT = no
MTK_DM_VERITY_OFF = no
MTK_DYNAMIC_CCB_BUFFER_GEAR_ID =
SYSTEM_AS_ROOT = no
MTK_USERIMAGES_USE_F2FS = yes
MTK_MRDUMP_ENABLE := no
MTK_AVB20_SUPPORT := yes
