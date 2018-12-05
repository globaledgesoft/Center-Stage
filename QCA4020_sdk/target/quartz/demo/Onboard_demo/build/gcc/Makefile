# Copyright (c) 2018 Qualcomm Technologies, Inc.
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

# Onboard settings WIFI for via WIFI and BLE for via BLE
DEVICE_BATTERY_OPERATED=0

# Allowed CHIPSET_VERSION - v1, v2
ifeq ($(CHIPSET_VERSION),)
   CHIPSET_VERSION = v2
endif

# Allowed CHIPSET_VARIANT - qca4020, qca4024, qca4025
ifeq ($(CHIPSET_VARIANT),)
   CHIPSET_VARIANT = qca4020
endif

ifeq ($(CHIPSET_VARIANT), qca4020)
   BOARD_SUPPORTS_WIFI = 1
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
ThirdpartyDir    = ../../../..
EcosystemRoot    = ../../../ecosystem
MeshClientModels = ../../../../qmesh/models/client
MeshServerModels = ../../../../qmesh/models/server
MeshModelsCommonCode = ../../../../qmesh/models/common

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
         lp/lp_demo.c \
         lp/fom_lp_test.c \
         lp/som_lp_test.c \
         lp/mom_lp_test.c \
         onboard.c\
         onboard_via_wifi.c\
         onboard_via_ble.c\
         utils/net_util.c\
         utils/wifi_util.c\
         utils/led_utils.c\
         utils/ble_util.c\
         utils/util.c\
         sensors/sensor_json.c \
         sensors/sensors.c\
         sensors/pir_int.c \
         ble/ble_wifi_service.c \
         ble/ble_zigbee_service.c \
         ble/ble_thread_service.c \
         zigbee/zigbee_util.c \
         zigbee/zigbee_coordinator.c \
         zigbee/zigbee_router.c \
         zigbee/zigbee_enddevice.c \
    	 zigbee/zcl_util.c \
         zigbee/clusters/zcl_alarms_demo.c \
         zigbee/clusters/zcl_basic_demo.c \
         zigbee/clusters/zcl_colorcontrol_demo.c \
         zigbee/clusters/zcl_custom_demo.c \
         zigbee/clusters/zcl_groups_demo.c \
         zigbee/clusters/zcl_identify_demo.c \
         zigbee/clusters/zcl_levelcontrol_demo.c \
         zigbee/clusters/zcl_onoff_demo.c \
         zigbee/clusters/zcl_ota_demo.c \
         zigbee/clusters/zcl_scenes_demo.c \
         zigbee/clusters/zcl_time_demo.c \
         zigbee/clusters/zcl_touchlink_demo.c \
         zigbee/clusters/zcl_powerconfig_demo.c \
         zigbee/clusters/zcl_devicetemp_demo.c \
         thread/thread_util.c \
         thread/thread_joiner.c \
         thread/thread_udp_joiner.c \
         thread/thread_border_router.c \
         thread/thread_udp_border_router.c \
         thread/thread_router.c\

ifeq ($(ECOSYSTEM), offline)
CSRCS += ble/ble_offline_service.c \
         ecosystem/offline/offline.c\
         ../../../../thirdparty/jsmn/src/jsmn.c
endif

ifeq ($(ECOSYSTEM), awsiot)

ifeq ($(CHIPSET_VARIANT), qca4020)
CSRCS += ../../../ecosystem/aws/port/network_qca4020_wrapper.c\
         ../../../ecosystem/aws/port/timer.c\
         ../../../../thirdparty/aws/awsiot/aws_iot_shadow.c\
         ../../../../thirdparty/aws/awsiot/aws_iot_json_utils.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_common_internal.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_connect.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_publish.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_subscribe.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_unsubscribe.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_yield.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_shadow_actions.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_shadow_json.c \
         ../../../../thirdparty/aws/awsiot/jsmn.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_shadow_records.c\
         ecosystem/aws/aws_run.c\

 endif

ifeq ($(CHIPSET_VARIANT),qca4024)
CSRCS += ../../../../thirdparty/jsmn/src/jsmn.c
endif

endif

CSRCS += export/platform_oem.c \
         export/DALConfig_devcfg.c \
         export/platform_oem_som.c \
         export/platform_oem_mom.c \
         export/DALConfig_fom.c \
         export/devcfg_devcfg_data.c \
         export/devcfg_fom_data.c \
         export/UsrEDL.c

# Include directories
INCLUDES := -I"$(ROOTDIR)/include" \
            -I"$(ROOTDIR)/thirdparty/aws/awsiot/include"\
            -I"$(ROOTDIR)/quartz/ecosystem/aws/port/include"\
            -I"$(ROOTDIR)/include/qapi" \
            -I"$(ROOTDIR)/include/bsp" \
            -I"$(SRCDIR)/qcli" \
            -I"$(SRCDIR)/include"\
            -I"$(SRCDIR)/ecosystem/aws"\
            -I"$(SRCDIR)/zigbee/clusters"\
            -I"$(SRCDIR)/sensors"\
            -I"$(ROOTDIR)/thirdparty/jsmn/include"\


# External objects and libraries
LIBS += "$(LIBDIR)/core.lib" \
        "$(LIBDIR)/qurt.lib" \
        "$(LIBDIR)/quartzplatform.lib" \
        "$(LIBDIR)/quartzplatform_xip.lib" \
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
        "$(LIBDIR)/vfs.lib" \
        "$(LIBDIR)/userpass.lib" \
        "$(LIBDIR)/i2s.lib" \
        "$(LIBDIR)/qapi_ed25519.lib" \
        "$(LIBDIR)/qapi_securefs.lib" \
        "$(LIBDIR)/pka_port.lib" \
        "$(LIBDIR)/fs_helper.lib" \
        "$(LIBDIR)/quartz_crypto_qapi.lib" \
        "$(LIBDIR)/zigbee.lib" \
        "$(LIBDIR)/quartz_zigbee.lib" \
        "$(LIBDIR)/mdns.lib" \
        "$(LIBDIR)/BLUETOPIA_SERVICES.lib" \
        "$(LIBDIR)/BLUETOPIA_QAPI_SERVICES.lib"   \
        "$(LIBDIR)/dnssd.lib" \
        "$(LIBDIR)/otp_tlv.lib" \
        "$(LIBDIR)/base64.lib" \
        "$(LIBDIR)/PERSIST_M4.lib" \
        "$(LIBDIR)/json.lib" \
        "$(LIBDIR)/json_qapi.lib"\
        "$(LIBDIR)/nichestack.lib"\
        "$(LIBDIR)/master_sdcc.lib"\
        "$(LIBDIR)/fwup.lib"\
        "$(LIBDIR)/fwup_engine.lib"\
        "$(LIBDIR)/csr.lib"\
        "$(LIBDIR)/thread.lib"\
        "$(LIBDIR)/qapi_thread.lib"\
		"$(LIBDIR)/tlv_transport.lib" \
        "$(LIBDIR)/crypto_port.lib" \
        "$(LIBDIR)/tee_master.lib" \
        "$(LIBDIR)/dnsclient.lib" \
        "$(LIBDIR)/securefs.lib" \
        "$(LIBDIR)/securefs_port.lib" \
        "$(LIBDIR)/v2core.lib"\
        "$(LIBDIR)/EDLManager.lib"\

ifeq ($(CHIPSET_VARIANT), qca4020)
    LIBS += "$(LIBDIR)/WLAN_QAPI.lib"\
            "$(LIBDIR)/CUST_IPSTACK_INICHE.lib" \
            "$(LIBDIR)/wlan_lib_common_xip.lib" \
            "$(LIBDIR)/cust_wlan_lib.lib"
endif

PATCHOBJS :=

   PATCHOBJS += "$(LIBDIR)/patch.lib"
   INCLUDES  += -I"$(ROOTDIR)/core/v2"

# Setup the build variables
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
LD = arm-none-eabi-ld

COPTS := -c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -O2 -Os -ffunction-sections -Wall
DEFINES :=-D WLAN_DEBUG -D ENABLE_P2P_MODE

ifeq ($(ONBOARD_VIA),WIFI)
   DEFINES += -D ONBOARD_VIA_WIFI
else ifeq ($(ONBOARD_VIA),BLE)
   DEFINES += -D ONBOARD_VIA_BLE
else
   DEFINES += -D ONBOARD_VIA_WIFI
endif

ifeq ($(RADIO), THREAD)
   DEFINES += -D THREAD
   DEFINES +=-D SUPPORTED_MODE=$(MODE)
else ifeq ($(RADIO), ZIGBEE)
   DEFINES += -D ZIGBEE
   DEFINES +=-D SUPPORTED_MODE=$(MODE)
else
   DEFINES += -D ZIGBEE
   DEFINES +=-D SUPPORTED_MODE=1
endif

DEFINES  += "-D CHIPSET_VARIANT=\"$(CHIPSET_VARIANT)\""
DEFINES  += "-D RTOS=\"$(RTOS)\""

ifeq ($(BOARD_SUPPORTS_WIFI), 1)
DEFINES  += "-D BOARD_SUPPORTS_WIFI=1"
endif

ifeq ($(ENABLE_DBGCALL),1)
   INCLUDES += -I"$(ROOTDIR)/quartz/sys/dbgcall/include"
   DEFINES  += "-D ENABLE_DBGCALL"
   LIBS     += "$(LIBDIR)/dbgcall.lib" \
               "$(LIBDIR)/swintr.lib"
endif



   DEFINES += "-D V2" \
              "-D qurt_mutex_init(x)=qurt_mutex_create(x)" \
              "-D qurt_mutex_destroy(x)=qurt_mutex_delete(x)" \
              "-D qurt_signal_init(x)=qurt_signal_create(x)" \
              "-D qurt_signal_destroy(x)=qurt_signal_delete(x)" \
              "-D FEATURE_QUARTZ_V2"


ifeq ($(BOARD_VARIANT),cdb)
   DEFINES +=  "-D CONFIG_CDB_PLATFORM"
endif
# Add the correct NVM library and user NVM file.
LIBS += $(LIBDIR)/QCA$(subst qca,,$(CHIPSET_VARIANT))_$(CHIPSET_REVISION)_$(subst dut,D,$(subst carrier,C,$(subst cdb,C,$(BOARD_VARIANT))))_NVM.lib

ifndef NVM_FILE
   NVM_FILE = $(NVMDIR)/config/$(subst p,.,$(CHIPSET_REVISION))/$(subst qca,,$(CHIPSET_VARIANT))/$(subst dut,DUT,$(subst carrier,Carrier,$(subst cdb,CDB,$(BOARD_VARIANT))))/QCA$(subst qca,,$(CHIPSET_VARIANT))_$(CHIPSET_REVISION).nvm
endif

CFLAGS         = $(COPTS) $(DEFINES) $(INCLUDES) -D_WANT_IO_C99_FORMATS
LDOpts        := -eSBL_Entry -no-wchar-size-warning --no-warn-mismatch -R"$(SYMFILE)" -R"$(SYMFILEUNPATCHED)" -T"$(LINKFILE)" -Map="$(OUTDIR)/$(PROJECT).map" -n --gc-sections
LDFLAGS        = $(LDOpts) --start-group @$(LIBSFILE) --end-group
OBJS          := $(CSRCS:%.c=$(OBJDIR)/%.o)
THIRDPARTYOBJS := $(CThirdPartySrcs:%.c=$(OBJDIR)/%.o)
ASSEMBLY_OBJS := $(ASSEMBLY_SRCS:%.S=$(OBJDIR)/%.asm_o)
DEPS          := $(CSRCS:%.c=%.d)
ExtraLibs     :=

ifeq ($(ECOSYSTEM), offline)
LDFLAGS += -L "$(NEWLIBPATH)" -L "$(TOOLLIBPATH)" -utlsio_template_interface_description -utlsio_template_get_interface_description
LDFLAGS += -lc_nano -lnosys  -u _scanf_float -lgcc
DEFINES += "-D JSMN_PARENT_LINKS=1"
DEFINES += "-D OFFLINE"
DEFINES += "-D ENABLE_DIMMER=1"
endif

ifeq ($(ECOSYSTEM), awsiot)
    LDFLAGS += -L "$(NEWLIBPATH)" -L "$(TOOLLIBPATH)" -utlsio_template_interface_description -utlsio_template_get_interface_description
    LDFLAGS += -lc_nano -lnosys  -u _scanf_float -lgcc
    DEFINES += "-D AWS_IOT"
    DEFINES += "-D JSMN_PARENT_LINKS=1"
    DEFINES += "-D ENABLE_DIMMER=0"
    CThirdPartyOpts=-c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -fno-exceptions -ffunction-sections -w -O2
    CThirdPartyFlags=$(CThirdPartyOpts) $(DEFINES) $(INCLUDES) -D_WANT_IO_C99_FORMATS
endif

ifeq ($(ECOSYSTEM), azure)
    ExtraLibs += -u _scanf_float -u _printf_float -lc -lnosys -lgcc -lm
    LDFLAGS += -L"$(NEWLIBPATH)" -L"$(TOOLLIBPATH)" -utlsio_template_interface_description -utlsio_template_get_interface_description $(ExtraLibs)
    CThirdPartyOpts=-c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -fno-exceptions -ffunction-sections -w -O1
    CThirdPartyFlags=$(CThirdPartyOpts) $(DEFINES) $(INCLUDES) -D_WANT_IO_C99_FORMATS
endif
$(THIRDPARTYOBJS): CFLAGS := $(CThirdPartyFlags)
all: prepare propgen nvm $(OBJS) $(THIRDPARTYOBJS) $(ASSEMBLY_OBJS)
	-rm -rf $(LIBSFILE)
	-rm -rf $(LINKFILE)


	$(foreach f,$(LIBS),echo $f>>$(LIBSFILE);)
	$(foreach f,$(PATCHOBJS),echo $f>>$(LIBSFILE);)
	$(foreach f,$(OBJS),echo $(addprefix $(OBJDIR)/, $(notdir $(f)))>>$(LIBSFILE);)
	$(foreach f,$(ASSEMBLY_OBJS),echo $(OBJDIR)/$(notdir $(f))>>$(LIBSFILE);)
	$(foreach f,$(THIRDPARTYOBJS),echo $(addprefix $(OBJDIR)/, $(notdir $(f)))>>$(LIBSFILE);)

# Update application PlacementFile
	python $(LINKERSCRIPTDIR)/CreateAppPlacementFile.py $(ROOTDIR)/bin/cortex-m4/$(RTOS)/sys.placement $(ROOTDIR)/bin/cortex-m4/$(RTOS)/cust.placement app.config app.placement 2>dbg.CreateApp

# Create a Quartz.ld linker script
	python $(LINKERSCRIPTDIR)/MakeLinkerScript.py $(ROOTDIR)/bin/cortex-m4/$(RTOS)/DefaultTemplateLinkerScript.ld app.placement $(LIBSFILE) > $(LINKFILE) 2>dbg.Make

	mkdir -p $(OBJDIR)
	@echo Linking....

	$(LD) $(LDFLAGS) -o"$(OUTDIR)/$(PROJECT)_nocompact.elf"
	# Run the diag compaction script to generate the final ELF
	@echo DIAG Message Compaction...
	python $(SCRIPTDIR)/diagMsgCompact.py $(OUTDIR)/$(PROJECT).elf $(ROOTDIR)/bin/cortex-m4/diag_msg_QCLI_demo.strdb $(OUTDIR)/$(PROJECT)_nocompact.elf $(ROOTDIR)/bin/cortex-m4/diag_msg.pkl Final > dictLog


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
	$(CC) $(CFLAGS) -D __FILENAME__=\"$<\" $< -o $(OBJDIR)/$(@F)



$(OBJDIR)/%.asm_o: $(SRCDIR)/%.S
	@mkdir -p $(dir $@)
	@echo Compiling $< $@
	$(CC) $(CFLAGS) -D __FILENAME__=\"$<\" $< -o $(OBJDIR)/$(@F)

clean:
	rm -rf $(OUTDIR)

clobber:
	rm -rf $(SRCDIR)/export
	rm -rf $(SRCDIR)/UsrEDL.c
	rm -rf $(OUTDIR)
prepare:
	@echo \##############################################################################
	@echo    Starting build for Quartz Onboard Application 
	@echo  $(NEWLIBPATH)
	@echo  $(TOOLLIBPATH)
	@echo \##############################################################################
	@echo Build Parameters:
	@echo RTOS      = $(RTOS)
	@echo Variant   = $(CHIPSET_VARIANT)
	@echo Board     = $(BOARD_VARIANT)
	@echo Revision  = $(CHIPSET_REVISION)
	@echo \##############################################################################

	@mkdir -p $(SRCDIR)/export
	cp -u -v $(ROOTDIR)/quartz/platform/export/platform_oem.h $(SRCDIR)/export
	cp -u -v $(ROOTDIR)/quartz/platform/export/platform_oem.c $(SRCDIR)/export
	cp -u -v $(ROOTDIR)/quartz/platform/export/platform_oem_mom.c $(SRCDIR)/export
	cp -u -v $(ROOTDIR)/quartz/platform/export/platform_oem_som.c $(SRCDIR)/export

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

