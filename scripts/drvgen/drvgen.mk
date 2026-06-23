ifndef DRVGEN_OUT
DRVGEN_OUT := $(BUILDDIR)
endif

DRVGEN_OUT_PATH := $(DRVGEN_OUT)/inc

ALL_DRVGEN_FILE :=

ifeq ($(filter mt2601,$(PLATFORM)),)
  ALL_DRVGEN_FILE += inc/cust_kpd.h
  ALL_DRVGEN_FILE += inc/cust_eint.h
  ALL_DRVGEN_FILE += inc/cust_gpio_boot.h
  ALL_DRVGEN_FILE += inc/cust_gpio_usage.h
  ALL_DRVGEN_FILE += inc/cust_adc.h
  ALL_DRVGEN_FILE += inc/pmic_drv.h
  ALL_DRVGEN_FILE += inc/pmic_drv.c
endif

ifeq ($(filter mt2601 mt8127 mt8163,$(PLATFORM)),)
  ALL_DRVGEN_FILE += inc/cust_eint_md1.h
endif

ifeq ($(filter mt2601 mt6572 mt6582 mt6592 mt8127,$(PLATFORM)),)
  ALL_DRVGEN_FILE += inc/cust_eint.dtsi
endif

ifeq ($(filter mt2601 mt6580,$(PLATFORM)),)
  ALL_DRVGEN_FILE += inc/cust_power.h
endif

ifeq ($(filter mt2601 mt6572 mt6582 mt6592 mt8127 mt8163,$(PLATFORM)),)
  ALL_DRVGEN_FILE += inc/cust_clk_buf.h
endif

ifeq ($(filter mt2601 mt6572 mt6582 mt6592 mt8127 mt8163,$(PLATFORM)),)
  ALL_DRVGEN_FILE += inc/cust_i2c.h
endif

ifeq ($(PLATFORM),mt2601)
  ALL_DRVGEN_FILE += inc/cust_kpd.h
  ALL_DRVGEN_FILE += inc/cust_eint.h
  ALL_DRVGEN_FILE += inc/cust_gpio_usage.h
  ALL_DRVGEN_FILE += include/target/cust_gpio_boot.h
  ALL_DRVGEN_FILE += include/target/cust_power.h
endif

ifeq ($(PLATFORM),mt6752)
  ALL_DRVGEN_FILE += inc/cust_eint_md2.h
endif

ifeq ($(PLATFORM),mt6595)
  ALL_DRVGEN_FILE += inc/cust_gpio_suspend.h
endif

ifeq ($(PLATFORM),mt6580)
  ALL_DRVGEN_FILE += inc/cust_i2c.dtsi
endif

ifeq ($(PLATFORM),mt8127)
  ALL_DRVGEN_FILE += inc/cust_eint_ext.h
endif

ifeq ($(MTK_PLATFORM),mt6735)
  ALL_DRVGEN_FILE += inc/cust_adc.dtsi
  ALL_DRVGEN_FILE += inc/cust_i2c.dtsi
  ALL_DRVGEN_FILE += inc/cust_md1_eint.dtsi
  ALL_DRVGEN_FILE += inc/cust_kpd.dtsi
  ALL_DRVGEN_FILE += inc/cust_clk_buf.dtsi
  ALL_DRVGEN_FILE += inc/cust_gpio.dtsi
  ALL_DRVGEN_FILE += inc/cust_adc.dtsi
  ALL_DRVGEN_FILE += inc/cust_pmic.dtsi
  ALL_DRVGEN_FILE += inc/mt6735-pinfunc.h
  ALL_DRVGEN_FILE += inc/pinctrl-mtk-mt6735.h
endif

DRVGEN_FILE_LIST := $(addprefix $(DRVGEN_OUT)/,$(ALL_DRVGEN_FILE))
DRVGEN_TOOL := $(PWD)/scripts/dct/DrvGen
DWS_FILE := $(PWD)/target/$(TARGET)/dct/$(if $(CUSTOM_KERNEL_DCT),$(CUSTOM_KERNEL_DCT),dct)/codegen.dws
DRVGEN_PREBUILT_PATH := $(PWD)/target/$(TARGET)
DRVGEN_PREBUILT_CHECK := $(filter-out $(wildcard $(addprefix $(DRVGEN_PREBUILT_PATH)/,$(ALL_DRVGEN_FILE))),$(addprefix $(DRVGEN_PREBUILT_PATH)/,$(ALL_DRVGEN_FILE)))

.PHONY: drvgen
drvgen: $(DRVGEN_FILE_LIST)

$(DRVGEN_FILE_LIST): $(DRVGEN_OUT)/% : $(DRVGEN_PREBUILT_PATH)/%
	@mkdir -p $(dir $@)
	@if [ -f "$<" ]; then \
		cp -f $< $@; \
	else \
		echo "/* Auto-generated stub for $(TARGET) */" > $@; \
		echo "#ifndef __$$(echo $(notdir $@) | tr 'a-z.' 'A-Z_')__" >> $@; \
		echo "#define __$$(echo $(notdir $@) | tr 'a-z.' 'A-Z_')__" >> $@; \
		echo "#endif" >> $@; \
	fi
