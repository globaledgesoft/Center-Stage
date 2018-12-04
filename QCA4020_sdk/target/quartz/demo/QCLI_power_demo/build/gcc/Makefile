# Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
# 2016 Qualcomm Atheros, Inc.
# All Rights Reserved.
# Copyright (c) 2018 Qualcomm Technologies, Inc.
# All rights reserved.
# Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below)
# provided that the following conditions are met:
# Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Allowed CHIPSET_VERSION - v1, v2
ifeq ($(CHIPSET_VERSION),)
   CHIPSET_VERSION = v2
endif

# Allowed CHIPSET_VARIANT - qca4020, qca4024, qca4025
ifeq ($(CHIPSET_VARIANT),)
   CHIPSET_VARIANT = qca4020
endif

# Allowed CFG_FEATURE_WLAN - true, false
ifeq ($(CHIPSET_VARIANT),qca4020)
   CFG_FEATURE_WLAN = true
else
   CFG_FEATURE_WLAN = false
endif

# Allowed RTOS - threadx, freertos
ifeq ($(RTOS),)
   RTOS = threadx
endif

# Allowed BOARD_VARIANT - carrier, dut, cdb
ifeq ($(BOARD_VARIANT),)
   BOARD_VARIANT = carrier
endif

# Allowed CHIPSET_REVISION - 1p2, 2p0
ifeq ($(CHIPSET_REVISION),)
    ifeq ($(CHIPSET_VERSION),v1)
        CHIPSET_REVISION = 1p2
    else
        CHIPSET_REVISION = 2p0
    endif
endif

# Setup the paths for the build
PROJECT          = Quartz
ROOTDIR          = ../../../../..
SRCDIR           = ../../src
NVMDIR           = $(ROOTDIR)/quartz/nvm
OUTDIR           = output
OBJDIR           = $(OUTDIR)/objs
SECTOOLSDIR      = $(ROOTDIR)/sectools
SECTOOLSQDNDIR   = $(ROOTDIR)/sectools/qdn
SECTOOLSCERTSDIR = $(SECTOOLSQDNDIR)/resources/data_prov_assets/Signing/Local/qc_presigned_certs-key2048_exp257
SECBOOT          = false
LIBDIR           = $(ROOTDIR)/lib/cortex-m4IPT/$(RTOS)
SYMFILE          = $(ROOTDIR)/bin/cortex-m4/IOE_ROM_IPT_IMG_ARNNRI_gcc.sym
SYMFILEUNPATCHED = $(ROOTDIR)/bin/cortex-m4/IOE_ROM_IPT_IMG_ARNNRI_orig_fcns_gcc.sym
SCRIPTDIR        = $(ROOTDIR)/build/scripts
LINKERSCRIPTDIR  = $(ROOTDIR)/build/scripts/linkerScripts
LINKFILE         = $(OUTDIR)/$(PROJECT).ld
LIBSFILE         = $(OUTDIR)/LinkerLibs.txt

ifeq ($(RTOS),threadx)
   OSLIB = threadx.lib
   LIBS  = "$(LIBDIR)/$(OSLIB)" \
           "$(LIBDIR)/mom_patch_table_ARNTRI_qcm.o" \
           "$(LIBDIR)/fom_patch_table_ARNTRI_qcm.o" \
           "$(LIBDIR)/som_patch_table_ARNTRI_qcm.o"
else
   OSLIB = free_rtos.lib
   LIBS  = "$(LIBDIR)/$(OSLIB)" \
           "$(LIBDIR)/mom_patch_table_ARNFRI_qcm.o" \
           "$(LIBDIR)/fom_patch_table_ARNFRI_qcm.o" \
           "$(LIBDIR)/som_patch_table_ARNFRI_qcm.o"
endif

ASSEMBLY_SRCS :=

# Sources to compile
CSRCS := sbrk.c \
         qcli/qcli.c \
         qcli/qcli_util.c \
         qcli/pal.c \
         qcli/pal_som.c \
         qcli/pal_mom.c \
         hmi/hmi_demo.c \
         hmi/hmi_demo_som.c \
         ble/ble_demo.c \
         coex/coex_demo.c

CSRCS += export/DALConfig_devcfg.c \
         export/DALConfig_fom.c \
         export/devcfg_devcfg_data.c \
         export/devcfg_fom_data.c \
         export/UsrEDL.c

# Include directories
INCLUDES := -I"$(ROOTDIR)/include" \
            -I"$(ROOTDIR)/include/qapi" \
            -I"$(ROOTDIR)/include/bsp" \
            -I"$(SRCDIR)/qcli" \
            -I"$(SRCDIR)/hmi" \
            -I"$(SRCDIR)/ble" \
            -I"$(SRCDIR)/coex" \

# External objects and libraries
LIBS += "$(LIBDIR)/core.lib" \
        "$(LIBDIR)/qurt.lib" \
        "$(LIBDIR)/quartzplatform.lib" \
        "$(LIBDIR)/WLAN.lib" \
        "$(LIBDIR)/WLAN_PROFILER.lib" \
        "$(LIBDIR)/net.lib" \
        "$(LIBDIR)/net_ram.lib" \
        "$(LIBDIR)/dhcpv6c.lib" \
        "$(LIBDIR)/sntpc.lib" \
        "$(LIBDIR)/dnssrvr.lib" \
        "$(LIBDIR)/sharkssl.lib" \
        "$(LIBDIR)/cryptolib.lib" \
        "$(LIBDIR)/httpsvr.lib" \
        "$(LIBDIR)/httpc.lib" \
        "$(LIBDIR)/mqttc.lib" \
        "$(LIBDIR)/csr.lib" \
        "$(LIBDIR)/vfs.lib" \
        "$(LIBDIR)/userpass.lib" \
        "$(LIBDIR)/i2s.lib" \
        "$(LIBDIR)/fwup.lib" \
        "$(LIBDIR)/fwup_engine.lib" \
        "$(LIBDIR)/qapi_ed25519.lib" \
        "$(LIBDIR)/qapi_securefs.lib" \
        "$(LIBDIR)/pka_port.lib" \
        "$(LIBDIR)/fs_helper.lib" \
        "$(LIBDIR)/zigbee.lib" \
        "$(LIBDIR)/quartz_zigbee.lib" \
        "$(LIBDIR)/mdns.lib" \
        "$(LIBDIR)/BLUETOPIA_SERVICES.lib" \
        "$(LIBDIR)/BLUETOPIA_QAPI_SERVICES.lib"   \
        "$(LIBDIR)/dnssd.lib" \
        "$(LIBDIR)/otp_tlv.lib" \
        "$(LIBDIR)/base64.lib"	\
        "$(LIBDIR)/nichestack.lib" \
        "$(LIBDIR)/quartzplatform_xip.lib"

ifeq ($(CFG_FEATURE_WLAN),true)
LIBS += "$(LIBDIR)/WLAN_QAPI.lib" \
        "$(LIBDIR)/CUST_IPSTACK_INICHE.lib"\
        "$(LIBDIR)/wlan_lib_common_xip.lib"\
        "$(LIBDIR)/cust_wlan_lib.lib"
endif
		
PATCHOBJS :=

ifeq ($(CHIPSET_VERSION),v1)
   LIBS += "$(LIBDIR)/pka.lib" \
           "$(LIBDIR)/homekit.lib" \
           "$(LIBDIR)/v1core.lib" \
           "$(LIBDIR)/net_v1.lib" \
           "$(LIBDIR)/i15p4_hmi.lib" \
           "$(LIBDIR)/OSAL_M4.lib" \

   PATCHOBJS += "$(LIBDIR)/patch.lib"
   INCLUDES  += -I"$(ROOTDIR)/core/v1"
else
   # In v1 these are in ROM, moved to XIP for v2
   LIBS += "$(LIBDIR)/tlv_transport.lib" \
           "$(LIBDIR)/crypto_port.lib" \
           "$(LIBDIR)/tee_master.lib" \
           "$(LIBDIR)/dnsclient.lib" \
           "$(LIBDIR)/securefs.lib" \
           "$(LIBDIR)/securefs_port.lib" \
           "$(LIBDIR)/v2core.lib" \
           "$(LIBDIR)/EDLManager.lib" \

   PATCHOBJS += "$(LIBDIR)/patch.lib"
   INCLUDES  += -I"$(ROOTDIR)/core/v2"
endif

ifeq ($(ENABLE_CPU_PROFILER),1)
   INCLUDES += -I"$(SRCDIR)/cpu_profiler"
endif

# Setup the build variables
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
LD = arm-none-eabi-ld

COPTS := -c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -O2 -Wall
DEFINES :=-D WLAN_DEBUG -D ENABLE_P2P_MODE

ifeq ($(BOARD_VARIANT),cdb)
   DEFINES +=  "-D CONFIG_CDB_PLATFORM"
endif

ifeq ($(CHIPSET_VERSION),v1)
   DEFINES += "-D V1"
else
   DEFINES += "-D V2" \
              "-D qurt_mutex_init(x)=qurt_mutex_create(x)" \
              "-D qurt_mutex_destroy(x)=qurt_mutex_delete(x)" \
              "-D qurt_signal_init(x)=qurt_signal_create(x)" \
              "-D qurt_signal_destroy(x)=qurt_signal_delete(x)" \
              "-D FEATURE_QUARTZ_V2"
endif

ifeq ($(ENABLE_CPU_PROFILER),1)
   DEFINES += "-D ENABLE_CPU_PROFILER"
endif

# Add the correct NVM library and user NVM file.
LIBS += $(LIBDIR)/QCA$(subst qca,,$(CHIPSET_VARIANT))_$(CHIPSET_REVISION)_$(subst dut,D,$(subst carrier,C,$(subst cdb,CDB,$(BOARD_VARIANT))))_NVM.lib

ifndef NVM_FILE
   NVM_FILE = $(NVMDIR)/config/$(subst p,.,$(CHIPSET_REVISION))/$(subst qca,,$(CHIPSET_VARIANT))/$(subst dut,DUT,$(subst carrier,Carrier,$(subst cdb,CDB,$(BOARD_VARIANT))))/QCA$(subst qca,,$(CHIPSET_VARIANT))_$(CHIPSET_REVISION).nvm
endif

CFLAGS         = $(COPTS) $(DEFINES) $(INCLUDES)
LDOpts        := -eSBL_Entry -no-wchar-size-warning --no-warn-mismatch -R"$(SYMFILE)" -R"$(SYMFILEUNPATCHED)" -T"$(LINKFILE)" -Map="$(OUTDIR)/$(PROJECT).map" -uqca_init -ugTsensBsp_devcfg_xml -ugAdcBsp_devcfg_xml -uquartz_ram_function_table -uwlan_secboot_auth -umom_indirection_table -usom_indirection_table -ufom_indirection_table -n
LDFLAGS        = $(LDOpts) --start-group @$(LIBSFILE) --end-group
OBJS          := $(CSRCS:%.c=$(OBJDIR)/%.o)
ASSEMBLY_OBJS := $(ASSEMBLY_SRCS:%.S=$(OBJDIR)/%.asm_o)
DEPS          := $(CSRCS:%.c=%.d)

all: prepare propgen nvm $(OBJS) $(ASSEMBLY_OBJS)
	-rm -rf $(LIBSFILE)
	-rm -rf $(LINKFILE)

	$(foreach f,$(LIBS),echo $f>>$(LIBSFILE);)
	$(foreach f,$(PATCHOBJS),echo $f>>$(LIBSFILE);)
	$(foreach f,$(OBJS),echo $f>>$(LIBSFILE);)
	$(foreach f,$(ASSEMBLY_OBJS),echo $f>>$(LIBSFILE);)

# Update application PlacementFile
	python $(LINKERSCRIPTDIR)/CreateAppPlacementFile.py $(ROOTDIR)/bin/cortex-m4/$(RTOS)/sys.placement $(ROOTDIR)/bin/cortex-m4/$(RTOS)/cust.placement app.config app.placement 2>dbg.CreateApp

# Create a Quartz.ld linker script
	python $(LINKERSCRIPTDIR)/MakeLinkerScript.py $(ROOTDIR)/bin/cortex-m4/$(RTOS)/DefaultTemplateLinkerScript.ld app.placement $(LIBSFILE) > $(LINKFILE) 2>dbg.Make

	mkdir -p $(OBJDIR)
	@echo Linking....
ifeq ($(CHIPSET_VERSION),v1)
	$(LD) $(LDFLAGS) -o"$(OUTDIR)/$(PROJECT).elf"
else
	$(LD) $(LDFLAGS) -o"$(OUTDIR)/$(PROJECT)_nocompact.elf"
	# Run the diag compaction script to generate the final ELF
	@echo DIAG Message Compaction...
	python $(SCRIPTDIR)/diagMsgCompact.py $(OUTDIR)/$(PROJECT).elf $(ROOTDIR)/bin/cortex-m4/diag_msg_QCLI_demo.strdb $(OUTDIR)/$(PROJECT)_nocompact.elf $(ROOTDIR)/bin/cortex-m4/diag_msg.pkl Final > dictLog
endif

	@echo Hashing...
	python $(SCRIPTDIR)/createxbl.py -f$(OUTDIR)/$(PROJECT).elf -a32 -o$(OUTDIR)/$(PROJECT)_HASHED.elf
ifeq ($(SECBOOT),true)
	@echo Signing Images...
ifeq ($(BOARD_VARIANT),cdb)
	python $(SECTOOLSQDNDIR)/sectools.py iot -p 4020 -g m4 -i $(OUTDIR)/$(PROJECT).elf -k $(SECTOOLSCERTSDIR)/qpsa_rootca.key -c $(SECTOOLSCERTSDIR)/qpsa_rootca.cer --cfg_oem_id=0xffff --cfg_model_id=0x0000 -o . -s
	python $(SECTOOLSQDNDIR)/sectools.py iot -p 4020 -g m0 -i $(ROOTDIR)/bin/cortex-m0/threadx/ioe_ram_m0_threadx_ipt.mbn -k $(SECTOOLSCERTSDIR)/qpsa_rootca.key -c $(SECTOOLSCERTSDIR)/qpsa_rootca.cer --cfg_oem_id=0xffff --cfg_model_id=0x0000 -o . -s
	python $(SECTOOLSQDNDIR)/sectools.py iot -p 4020 -g kf -i $(ROOTDIR)/bin/wlan/wlan_fw_img.bin -k $(SECTOOLSCERTSDIR)/qpsa_rootca.key -c $(SECTOOLSCERTSDIR)/qpsa_rootca.cer --cfg_oem_id=0xffff --cfg_model_id=0x0000 -o . -s
else
	python $(SECTOOLSDIR)/sectools.py secimage -i $(OUTDIR)/$(PROJECT).elf -c 4020_secimage.xml -sa -g m4 -o .
	python $(SECTOOLSDIR)/sectools.py secimage -i $(ROOTDIR)/bin/cortex-m0/threadx/ioe_ram_m0_threadx_ipt.mbn -c 4020_secimage.xml -sa -g m0 -o .
	python $(SECTOOLSDIR)/sectools.py secimage -i $(ROOTDIR)/bin/wlan/wlan_fw_img.bin -c 4020_secimage.xml -sa -g kf -o .
endif
endif

	@echo Build Completed...

nvm:
	-rm -rf $(SRCDIR)/export/UsrEDL.c
	python $(NVMDIR)/tool/NVM2C.py -o $(SRCDIR)/export/UsrEDL.c -i $(NVM_FILE)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@echo Compiling $< $@
	$(CC) $(CFLAGS) -D __FILENAME__=\"$<\" $< -o $@

$(OBJDIR)/%.asm_o: $(SRCDIR)/%.S
	@mkdir -p $(dir $@)
	@echo Compiling $< $@
	$(CC) $(CFLAGS) -D __FILENAME__=\"$<\" $< -o $@

clean:
	rm -rf $(OUTDIR)

clobber:
	rm -rf $(SRCDIR)/export
	rm -rf $(SRCDIR)/UsrEDL.c
	rm -rf $(OUTDIR)

prepare:
ifeq ($(CHIPSET_VERSION),v1)
	@echo \##############################################################################
	@echo    Starting build for Quartz QCLI Power Application for v1 Chipset
	@echo                  To debug v2, set CHIPSET_VERSION=v2
	@echo \##############################################################################
else
	@echo \##############################################################################
	@echo    Starting build for Quartz QCLI Power Application for v2 Chipset
	@echo                  To debug v1, set CHIPSET_VERSION=v1
	@echo \##############################################################################
endif

	@echo Build Parameters:
	@echo RTOS		= $(RTOS)
	@echo Variant	= $(CHIPSET_VARIANT)
	@echo Board		= $(BOARD_VARIANT)
	@echo Revision	= $(CHIPSET_REVISION)
	@echo \##############################################################################

	@mkdir -p $(SRCDIR)/export

	cp -u -v $(ROOTDIR)/build/tools/devcfg/$(RTOS)/DevCfg_master_devcfg_out.xml $(SRCDIR)/export
	cp -u -v $(ROOTDIR)/build/tools/devcfg/$(RTOS)/DevCfg_master_devcfg_out_cdb.xml $(SRCDIR)/export
	cp -u -v $(ROOTDIR)/build/tools/devcfg/$(RTOS)/DevCfg_master_fom_out.xml $(SRCDIR)/export
	cp -u -v $(ROOTDIR)/build/tools/devcfg/$(RTOS)/DevCfg_master_fom_out_cdb.xml $(SRCDIR)/export

propgen:
	@echo GENERATING DEVCFG....
	rm -f $(SRCDIR)/export/DALConfig_devcfg.c
	rm -f $(SRCDIR)/export/devcfg_devcfg_data.c
	rm -f $(SRCDIR)/export/DALConfig_fom.c
	rm -f $(SRCDIR)/export/devcfg_fom_data.c
	python $(ROOTDIR)/build/tools/devcfg/propgen.py --XmlFile=$(SRCDIR)/export/DevCfg_master_devcfg_out$(subst dut,,$(subst carrier,,$(subst cdb,_cdb,$(BOARD_VARIANT)))).xml --DirName=$(SRCDIR)/export --ConfigFile=$(SRCDIR)/export/DALConfig_devcfg.c --DevcfgDataFile=$(SRCDIR)/export/devcfg_devcfg_data.c --ConfigType=$(CHIPSET_VARIANT)_devcfg_xml
	python $(ROOTDIR)/build/tools/devcfg/propgen.py --XmlFile=$(SRCDIR)/export/DevCfg_master_fom_out$(subst dut,,$(subst carrier,,$(subst cdb,_cdb,$(BOARD_VARIANT)))).xml --DirName=$(SRCDIR)/export --ConfigFile=$(SRCDIR)/export/DALConfig_fom.c --DevcfgDataFile=$(SRCDIR)/export/devcfg_fom_data.c --ConfigType=$(CHIPSET_VARIANT)_fom_xml
