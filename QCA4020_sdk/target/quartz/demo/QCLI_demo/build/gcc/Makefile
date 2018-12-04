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

CFG_FEATURE_HMI ?= true
CFG_FEATURE_BLE ?= true
CFG_FEATURE_WLAN ?= true
CFG_FEATURE_FLASHLOG ?= true
CFG_FEATURE_LP ?= true
CFG_FEATURE_ZIGBEE ?= true
CFG_FEATURE_COEX ?= true
CFG_FEATURE_FWUP ?= true
CFG_FEATURE_FS ?= true
CFG_FEATURE_SECUREFS ?= true
CFG_FEATURE_THREAD ?= true
CFG_FEATURE_I2S ?= false
CFG_FEATURE_PERIPHERALS ?= true
CFG_FEATURE_PLATFORM ?= true
CFG_FEATURE_ECOSYSTEM ?= true
CFG_FEATURE_JSON ?= true
CFG_FEATURE_KPI_DEMO ?=false
CFG_FEATURE_NET ?= true
CFG_FEATURE_NET_PING ?= true
CFG_FEATURE_NET_ROUTE ?= true
CFG_FEATURE_NET_TXRX ?= true
CFG_FEATURE_NET_SSL ?= true
CFG_FEATURE_NET_DHCPV4C ?= true
CFG_FEATURE_NET_DHCPV6C ?= true
CFG_FEATURE_NET_AUTOIP ?= true
CFG_FEATURE_NET_SNTPC ?= true
CFG_FEATURE_NET_HTTPS ?= true
CFG_FEATURE_NET_HTTPC ?= true
CFG_FEATURE_NET_DNSC ?= true
CFG_FEATURE_NET_DNSS ?= true
CFG_FEATURE_NET_MDNSS ?= true
CFG_FEATURE_NET_DNSSD ?= true
CFG_FEATURE_NET_MQTTC ?= true
CFG_FEATURE_NET_USER_ACCOUNT ?= true
CFG_FEATURE_NET_RAW_SOCKET ?= true
CFG_FEATURE_NET_WLAN_BRIDGE ?= true
CFG_FEATURE_NET_TCP_KEEPALIVE ?= true
CFG_FEATURE_CRYPTO ?= true
CFG_FEATURE_CRYPTO_UNIT_TEST ?= true
CFG_FEATURE_CRYPTO_BASE64 ?= true
CFG_FEATURE_CRYPTO_PERSISTENT_OBJECT ?= true
CFG_FEATURE_CRYPTO_ATTESTATION ?= true
CFG_FEATURE_NET_WEBSOCKETC ?= true

ifeq ($(CFG_FEATURE_CHK_POINT_GPIO_PIN),)
    CFG_FEATURE_CHK_POINT_GPIO_PIN=56
endif
 

# Allowed CHIPSET_VARIANT - qca4020, qca4024, qca4025
ifeq ($(CHIPSET_VARIANT),)
   CHIPSET_VARIANT = qca4020
endif

ifneq ($(CHIPSET_VARIANT),qca4020)
   CFG_FEATURE_WLAN = false
   CFG_FEATURE_KPI_DEMO = false
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
         spple/spple_demo.c \
         spple/ota/ble_ota_service.c \
         hmi/hmi_demo.c \
         coex/coex_demo.c \
         net/netcmd.c \
         net/netutils.c \
         net/bench_udp.c   \
         net/bench_tcp.c   \
         net/bench_raw.c   \
         net/bench_ssl.c   \
         net/bench_uapsd.c   \
         net/bench.c \
         net/ssl_demo.c \
         net/cert_demo.c \
         net/httpc_demo.c \
		 net/websocketc_demo.c \
         net/mqttc_demo.c \
         net/iperf.c \
         net/eth_raw.c \
         net/httpsvr/cgi/htmldata.c \
         net/httpsvr/cgi/cgi_showintf.c \
         net/httpsvr/cgi/cgi_demo.c \
         ota/ota_demo.c \
         ota/plugins/ftp/ota_ftp.c \
         ota/plugins/http/ota_http.c \
         ota/plugins/zigbee/ota_zigbee.c \
         ota/plugins/ble/ota_ble.c \
         adss/adss_demo.c \
         adss/adss.c \
         adss/adss_audio_data.c \
         adss/adss_ftp.c \
         adss/adss_ftp_rec.c \
         adss/adss_pcm.c \
         adss/adss_mem.c  \
         master_sdcc/master_sdcc_demo.c \
         master_sdcc/master_sdcc.c \
         htc_slave/htc_slave_demo.c \
         targetif/htc/src/htc.c \
         targetif/htc/src/htc_events.c \
         targetif/htc/src/htc_recv.c \
         targetif/htc/src/htc_send.c \
         targetif/htc/src/htc_utils.c \
         targetif/hif/hif.c \
         targetif/transport/qurt/transport.c \
         targetif/transport/qurt/sdio/sdio.c \
         targetif/transport/qurt/spi/spi_hal.c \
         targetif/app/htc_demo.c \
         lp/lp_demo.c \
         lp/fom_lp_test.c \
         lp/som_lp_test.c \
         lp/mom_lp_test.c \
         fs/fs_demo.c \
         securefs/securefs_demo.c \
         crypto/crypto_demo.c \
         crypto/crypto_helper.c \
         crypto/persistent_obj_demo.c \
         platform/platform_demo.c \
         sensors/sensors_demo.c \
         sensors/sensors.c \
         adc/adc_demo.c \
         adc/adc.c \
         pwm/pwm_demo.c \
         pwm/pwm.c \
         keypad/keypad.c \
         keypad/keypad_demo.c \
         peripherals/peripherals_demo.c \
         ecosystem/ecosystem_demo.c \
         enc/json_demo.c \
         thread/thread_demo.c

ifeq ($(CFG_FEATURE_KPI_DEMO),true)
CSRCS += kpi/kpi_demo.c
endif

ifeq ($(CFG_FEATURE_ZIGBEE),true)
CSRCS += zigbee/zigbee_demo.c \
         zigbee/zdp_demo.c \
         zigbee/zcl_demo.c \
         zigbee/clusters/zcl_alarms_demo.c \
         zigbee/clusters/zcl_basic_demo.c \
         zigbee/clusters/zcl_colorcontrol_demo.c \
         zigbee/clusters/zcl_custom_demo.c \
         zigbee/clusters/zcl_devicetemp_demo.c \
         zigbee/clusters/zcl_groups_demo.c \
         zigbee/clusters/zcl_identify_demo.c \
         zigbee/clusters/zcl_levelcontrol_demo.c \
         zigbee/clusters/zcl_onoff_demo.c \
         zigbee/clusters/zcl_ota_demo.c \
         zigbee/clusters/zcl_powerconfig_demo.c \
         zigbee/clusters/zcl_scenes_demo.c \
         zigbee/clusters/zcl_time_demo.c \
         zigbee/clusters/zcl_touchlink_demo.c \
         zigbee/clusters/zcl_doorlock_demo.c \
         zigbee/clusters/zcl_wincover_demo.c \
         zigbee/clusters/zcl_thermostat_demo.c \
         zigbee/clusters/zcl_fancontrol_demo.c \
         zigbee/clusters/zcl_tempmeasure_demo.c \
         zigbee/clusters/zcl_occupancy_demo.c \
         zigbee/clusters/zcl_iaszone_demo.c \
         zigbee/clusters/zcl_iasace_demo.c \
         zigbee/clusters/zcl_iaswd_demo.c \
         zigbee/clusters/zcl_ballast_demo.c \
         zigbee/clusters/zcl_illuminance_demo.c \
         zigbee/clusters/zcl_relhumid_demo.c
endif

ifeq ($(CFG_FEATURE_WLAN),true)
CSRCS += wifi/util.c \
         wifi/wifi_cmd_handler.c \
         wifi/wifi_demo.c
endif

ifeq ($(QMESH),true)
MESHCSRCS += qmesh/models/client_handler/model_client_menu.c \
         qmesh/models/client_handler/generic_onoff_client_handler.c \
         qmesh/models/client_handler/generic_power_onoff_client_handler.c \
         qmesh/models/client_handler/generic_default_transition_time_client_handler.c \
         qmesh/models/client_handler/generic_level_client_handler.c \
         qmesh/models/client_handler/generic_power_level_client_handler.c \
         qmesh/models/client_handler/light_lightness_client_handler.c \
         qmesh/models/client_handler/light_hsl_client_handler.c \
         qmesh/models/client_handler/config_client_handler.c \
         qmesh/models/client_handler/model_client_event_handler.c \
         qmesh/models/client_handler/model_client_handler_utils.c \
         qmesh/gatt_bearer/src/qmesh_ble_gap.c \
         qmesh/gatt_bearer/src/qmesh_ble_gatt.c \
         qmesh/gatt_bearer/src/qmesh_ble_gatt_client.c \
         qmesh/gatt_bearer/src/qmesh_ble_gatt_server.c	\
         qmesh/gatt_bearer/src/qmesh_ble_coex.c  \
         qmesh/qmesh_demo_menu.c  \
         qmesh/qmesh_demo_composition.c  \
         qmesh/qmesh_demo_core.c  \
         qmesh/qmesh_demo_utilities.c  \
         qmesh/qmesh_demo_nvm_utilities.c \
         $(MeshClientModels)/generic_onoff_client.c \
         $(MeshClientModels)/generic_power_onoff_client.c \
         $(MeshClientModels)/generic_default_transition_time_client.c \
         $(MeshClientModels)/generic_level_client.c \
         $(MeshClientModels)/generic_power_level_client.c \
         $(MeshClientModels)/light_lightness_client.c \
         $(MeshClientModels)/light_hsl_client.c \
         $(MeshModelsCommonCode)/qmesh_cache_mgmt.c \
         $(MeshModelsCommonCode)/qmesh_delay_cache.c \
         $(MeshModelsCommonCode)/qmesh_model_common.c \
         $(MeshModelsCommonCode)/qmesh_model_debug.c \
         $(MeshModelsCommonCode)/qmesh_model_nvm.c \
		 $(MeshModelsCommonCode)/qmesh_light_utilities.c \
         $(MeshServerModels)/qmesh_generic_default_transition_time_handler.c \
         $(MeshServerModels)/qmesh_generic_poweronoff_handler.c \
		 $(MeshServerModels)/qmesh_generic_poweronoff_setup_handler.c \
         $(MeshServerModels)/qmesh_generic_level_handler.c \
         $(MeshServerModels)/qmesh_generic_powerlevel_handler.c \
         $(MeshServerModels)/qmesh_generic_onoff_handler.c \
         $(MeshServerModels)/qmesh_light_hsl_handler.c \
         $(MeshServerModels)/qmesh_light_hsl_hue_handler.c \
         $(MeshServerModels)/qmesh_light_hsl_saturation_handler.c \
         $(MeshServerModels)/qmesh_light_hsl_setup_handler.c \
         $(MeshServerModels)/qmesh_light_lightness_handler.c \
         $(MeshServerModels)/qmesh_light_lightness_setup_handler.c  \
         $(MeshServerModels)/qmesh_vendor_model_handler.c
endif


ifeq ($(ECOSYSTEM),awsiot)
CSRCS += ../../../ecosystem/aws/port/timer.c \
         ../../../ecosystem/aws/port/network_qca4020_wrapper.c \
         ../../../../thirdparty/aws/awsiot/jsmn.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_json_utils.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_common_internal.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_connect.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_publish.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_subscribe.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_unsubscribe.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_mqtt_client_yield.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_shadow.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_shadow_actions.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_shadow_json.c \
         ../../../../thirdparty/aws/awsiot/aws_iot_shadow_records.c \
         ecosystem/aws/shadow_sample/shadow_sample.c
endif

ifeq ($(ECOSYSTEM),azure)
CThirdPartySrcs += $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/base64.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/buffer.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/connection_string_parser.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/constbuffer.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/consolelogger.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/crt_abstractions.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/constmap.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/doublylinkedlist.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/gballoc.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/gb_stdio.c \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/gb_time.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/gb_rand.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/hmac.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/hmacsha256.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/http_proxy_io.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/xio.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/singlylinkedlist.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/map.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/sastoken.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/sha1.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/sha224.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/sha384-512.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/strings.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/string_tokenizer.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/urlencode.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/usha.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/vector.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/xlogging.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/c-utility/src/optionhandler.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/src/mqtt_client.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/src/mqtt_codec.c  \
                   $(ThirdpartyDir)/thirdparty/azure/umqtt/src/mqtt_message.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/agenttypesystem.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/codefirst.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/commanddecoder.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/datamarshaller.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/datapublisher.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/dataserializer.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/iotdevice.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/jsondecoder.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/jsonencoder.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/multitree.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/schema.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/schemalib.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/schemaserializer.c  \
                   $(ThirdpartyDir)/thirdparty/azure/parson/parson.c  \
                   $(ThirdpartyDir)/thirdparty/azure/serializer/src/methodreturn.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/version.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/iothub_message.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/iothub_client_ll.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/iothub_client.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/iothubtransport.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/iothubtransport_mqtt_common.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/iothubtransportmqtt.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/iothub_client_authorization.c  \
                   $(ThirdpartyDir)/thirdparty/azure/iothub_client/src/iothub_client_retry_control.c  \
                   ecosystem/azure/devicetwin_simplesample.c  \
                   ecosystem/azure/simplesample_mqtt.c

CSRCS += $(EcosystemRoot)/azure/port/lock_qca402x.c  \
         $(EcosystemRoot)/azure/port/agenttime_qca402x.c  \
         $(EcosystemRoot)/azure/port/platform_qca402x.c  \
         $(EcosystemRoot)/azure/port/threadapi_qca402x.c  \
         $(EcosystemRoot)/azure/port/tickcounter_qca402x.c  \
         $(EcosystemRoot)/azure/port/tlsio_qca402x.c  \
         $(EcosystemRoot)/azure/port/certs.c
endif

ifeq ($(ENABLE_CPU_PROFILER),1)
   CSRCS += cpu_profiler/cpu_profiler_demo.c
   ASSEMBLY_SRCS += cpu_profiler/cpu_profiler_interrupt_asm.S
endif

ifeq ($(BOARD_VARIANT),cdb)
CSRCS += gpio/gpio.c \
         gpio/gpio_demo.c \
         spi/spi.c \
         spi/spi_demo.c
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
            -I"$(ROOTDIR)/include/qapi" \
            -I"$(ROOTDIR)/include/bsp" \
            -I"$(SRCDIR)/qcli" \
            -I"$(SRCDIR)/spple" \
            -I"$(SRCDIR)/spple/ota" \
            -I"$(SRCDIR)/hmi" \
            -I"$(SRCDIR)/coex" \
            -I"$(SRCDIR)/net" \
            -I"$(SRCDIR)/ota" \
            -I"$(SRCDIR)/lp" \
            -I"$(SRCDIR)/fs" \
            -I"$(SRCDIR)/wifi" \
            -I"$(SRCDIR)/adss" \
            -I"$(SRCDIR)/master_sdcc" \
            -I"$(SRCDIR)/htc_slave" \
            -I"$(SRCDIR)/targetif/include" \
            -I"$(SRCDIR)/targetif/htc/include" \
            -I"$(SRCDIR)/targetif/hif" \
            -I"$(SRCDIR)/targetif/osal" \
            -I"$(SRCDIR)/targetif/transport/qurt" \
            -I"$(SRCDIR)/targetif/transport/qurt/sdio" \
            -I"$(SRCDIR)/targetif/transport/qurt/spi" \
            -I"$(SRCDIR)/targetif/app" \
            -I"$(SRCDIR)/securefs" \
            -I"$(SRCDIR)/crypto" \
            -I"$(SRCDIR)/platform" \
            -I"$(SRCDIR)/zigbee" \
            -I"$(SRCDIR)/zigbee/clusters" \
            -I"$(SRCDIR)/thread" \
            -I"$(SRCDIR)/sensors" \
            -I"$(SRCDIR)/adc" \
            -I"$(SRCDIR)/pwm" \
            -I"$(SRCDIR)/keypad" \
            -I"$(SRCDIR)/peripherals" \
            -I"$(SRCDIR)/ecosystem" \
            -I"$(SRCDIR)/enc" \
            -I"$(ROOTDIR)/thirdparty/aws/awsiot/include" \
            -I"$(SRCDIR)/ecosystem/aws/shadow_sample" \
            -I"$(ROOTDIR)/quartz/ecosystem/aws/port/include" \
            -I"$(ROOTDIR)/qmesh/include"  \
            -I"$(ROOTDIR)/qmesh/platform/inc"  \
            -I"$(ROOTDIR)/qmesh/platform/qca402x/inc"  \
            -I"$(ROOTDIR)/qmesh/models/client" \
			-I"$(ROOTDIR)/qmesh/models/include"  \
            -I"$(SRCDIR)/qmesh"  \
            -I"$(SRCDIR)/qmesh/mesh"  \
            -I"$(SRCDIR)/qmesh/models/include"  \
            -I"$(SRCDIR)/qmesh/models/client_handler" \
            -I"$(SRCDIR)/qmesh/gatt_bearer/inc"  \
            -I"$(ROOTDIR)/qmesh/core/inc" \
            -I"$(SRCDIR)/gpio" \
            -I"$(SRCDIR)/spi"  \
			-I"$(SRCDIR)/kpi"


ifeq ($(ECOSYSTEM),azure)
INCLUDES += -I"$(ROOTDIR)/quartz/ecosystem/azure/inc"  \
            -I"$(ROOTDIR)/thirdparty/azure/umqtt/inc"  \
            -I"$(ROOTDIR)/thirdparty/azure/umqtt/c-utility/inc"  \
            -I"$(ROOTDIR)/thirdparty/azure/serializer/inc"  \
            -I"$(ROOTDIR)/thirdparty/azure/parson" \
            -I"$(ROOTDIR)/thirdparty/azure/iothub_client/inc" \
            -I"$(ROOTDIR)/quartz/ecosystem/azure/port"
endif

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
		"$(LIBDIR)/csr.lib" \
        "$(LIBDIR)/cryptolib.lib" \
        "$(LIBDIR)/httpsvr.lib" \
        "$(LIBDIR)/httpc.lib" \
        "$(LIBDIR)/mqttc.lib" \
        "$(LIBDIR)/vfs.lib" \
        "$(LIBDIR)/userpass.lib" \
        "$(LIBDIR)/i2s.lib" \
        "$(LIBDIR)/master_sdcc.lib" \
        "$(LIBDIR)/fwup.lib" \
        "$(LIBDIR)/fwup_engine.lib" \
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
        "$(LIBDIR)/json_qapi.lib" \
        "$(LIBDIR)/nichestack.lib" \
		"$(LIBDIR)/tlv_transport.lib" \
	    "$(LIBDIR)/crypto_port.lib" \
	    "$(LIBDIR)/tee_master.lib" \
	    "$(LIBDIR)/dnsclient.lib" \
	    "$(LIBDIR)/securefs.lib" \
	    "$(LIBDIR)/securefs_port.lib" \
	    "$(LIBDIR)/v2core.lib" \
	    "$(LIBDIR)/EDLManager.lib" \
	    "$(LIBDIR)/thread.lib" \
	    "$(LIBDIR)/qapi_thread.lib" \
	    "$(LIBDIR)/websocket.lib" 

ifeq ($(CFG_FEATURE_WLAN),true)
    LIBS += "$(LIBDIR)/WLAN_QAPI.lib"\
            "$(LIBDIR)/CUST_IPSTACK_INICHE.lib"\
            "$(LIBDIR)/wlan_lib_common_xip.lib"\
            "$(LIBDIR)/cust_wlan_lib.lib"
endif

PATCHOBJS :=

PATCHOBJS += "$(LIBDIR)/patch.lib"
INCLUDES  += -I"$(ROOTDIR)/core/v2"


ifeq ($(QMESH),true)
   LIBS += "$(LIBDIR)/qmesh.lib"
endif

# Setup the build variables
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
LD = arm-none-eabi-ld

COPTS := -c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -O2 -Os -ffunction-sections -Wall
DEFINES :=-D WLAN_DEBUG -D ENABLE_P2P_MODE

DEFINES += "-DCFG_FEATURE_CHK_POINT_GPIO_PIN=$(CFG_FEATURE_CHK_POINT_GPIO_PIN)"

ifeq ($(CFG_FEATURE_BLE),true)
   DEFINES += "-D CONFIG_SPPLE_DEMO"
endif
ifeq ($(CFG_FEATURE_HMI),true)
   DEFINES += "-D CONFIG_HMI_DEMO"
endif
ifeq ($(CFG_FEATURE_WLAN),true)
   DEFINES += "-D CONFIG_WIFI_DEMO"
endif
ifeq ($(CFG_FEATURE_COEX),true)
   DEFINES += "-D CONFIG_COEX_DEMO"
endif
ifeq ($(CFG_FEATURE_FWUP),true)
   DEFINES += "-D CONFIG_FWUP_DEMO"
endif
ifeq ($(CFG_FEATURTE_ADSS),true)
   DEFINES += "-D CONFIG_ADSS_DEMO"
endif
ifeq ($(CFG_FEATURE_LP),true)
   DEFINES += "-D CONFIG_LP_DEMO"
endif
ifeq ($(CFG_FEATURE_FS),true)
   DEFINES += "-D CONFIG_FS_DEMO"
endif
ifeq ($(CFG_FEATURE_SECUREFS),true)
   DEFINES += "-D CONFIG_SECUREFS_DEMO"
endif
ifeq ($(CFG_FEATURE_THREAD),true)
   DEFINES += "-D CONFIG_THREAD_DEMO"
endif
ifeq ($(CFG_FEATURE_ZIGBEE),true)
   DEFINES += "-D CONFIG_ZIGBEE_DEMO"
endif
ifeq ($(CFG_FEATURE_PERIPHERALS),true)
   DEFINES += "-D CONFIG_PERIPHERALS_DEMO"
endif
ifeq ($(CFG_FEATURE_PLATFORM),true)
   DEFINES += "-D CONFIG_PLATFORM_DEMO"
endif
ifeq ($(CFG_FEATURE_ECOSYSTEM),true)
   DEFINES += "-D CONFIG_ECOSYSTEM_DEMO"
endif
ifeq ($(CFG_FEATURE_JSON),true)
   DEFINES += "-D CONFIG_JSON_DEMO"
endif
ifeq ($(CFG_FEATURE_KPI_DEMO),true)
   DEFINES += "-D CONFIG_KPI_DEMO"
endif

# ----- Network Services Demos -----
# Global switch:
ifeq ($(CFG_FEATURE_NET),true)
   DEFINES += "-D CONFIG_NET_DEMO"
endif
# Specific network services demos. Comment to disable a demo:
ifeq ($(CFG_FEATURE_NET_PING),true)
   DEFINES +=  "-D CONFIG_NET_PING_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_ROUTE),true)
   DEFINES +=  "-D CONFIG_NET_ROUTE_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_TXRX),true)
   DEFINES +=  "-D CONFIG_NET_TXRX_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_SSL),true)
   DEFINES +=  "-D CONFIG_NET_SSL_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_DHCPV4C),true)
   DEFINES +=  "-D CONFIG_NET_DHCPV4C_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_DHCPV6C),true)
   DEFINES +=  "-D CONFIG_NET_DHCPV6C_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_AUTOIP),true)
   DEFINES +=  "-D CONFIG_NET_AUTOIP_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_SNTPC),true)
   DEFINES +=  "-D CONFIG_NET_SNTPC_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_HTTPS),true)
   DEFINES +=  "-D CONFIG_NET_HTTPS_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_HTTPC),true)
   DEFINES +=  "-D CONFIG_NET_HTTPC_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_DNSC),true)
   DEFINES +=  "-D CONFIG_NET_DNSC_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_DNSS),true)
   DEFINES +=  "-D CONFIG_NET_DNSS_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_MDNSS),true)
   DEFINES +=  "-D CONFIG_NET_MDNSS_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_DNSSD),true)
   DEFINES +=  "-D CONFIG_NET_DNSSD_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_MQTTC),true)
   DEFINES +=  "-D CONFIG_NET_MQTTC_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_USER_ACCOUNT),true)
   DEFINES +=  "-D CONFIG_NET_USER_ACCOUNT_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_RAW_SOCKET),true)
   DEFINES +=  "-D CONFIG_NET_RAW_SOCKET_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_WLAN_BRIDGE),true)
   DEFINES +=  "-D CONFIG_NET_WLAN_BRIDGE_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_TCP_KEEPALIVE),true)
   DEFINES +=  "-D CONFIG_NET_TCP_KEEPALIVE_DEMO"
endif
ifeq ($(CFG_FEATURE_NET_WEBSOCKETC),true)
   DEFINES +=  "-D CONFIG_NET_WEBSOCKETC_DEMO"
endif
ifeq ($(BOARD_VARIANT),cdb)
   DEFINES +=  "-D CONFIG_CDB_PLATFORM"
endif

# ----- Crypto Demos -----
# Global switch:
ifeq ($(CFG_FEATURE_CRYPTO),true)
   DEFINES += "-D CONFIG_CRYPTO_DEMO"
endif

# Specific crypto demos. Comment to disable a demo:
ifeq ($(CFG_FEATURE_CRYPTO_UNIT_TEST),true)
   DEFINES += "-D CONFIG_CRYPTO_UNIT_TEST_DEMO"
endif
ifeq ($(CFG_FEATURE_CRYPTO_BASE64),true)
   DEFINES += "-D CONFIG_CRYPTO_BASE64_DEMO"
endif
ifeq ($(CFG_FEATURE_CRYPTO_PERSISTENT_OBJECT),true)
   DEFINES += "-D CONFIG_CRYPTO_PERSISTENT_OBJECT_DEMO"
endif
ifeq ($(CFG_FEATURE_CRYPTO_ATTESTATION),true)
   DEFINES += "-D CONFIG_CRYPTO_ATTESTATION_DEMO"
endif


ifeq ($(ECOSYSTEM), awsiot)
   DEFINES += "-D AWS_IOT"
endif

ifeq ($(ECOSYSTEM), azure)
#   @echo Building Azure IOT SDK
   DEFINES += "-D AZURE_IOT"
   DEFINES += "-D DONT_USE_UPLOADTOBLOB"
   DEFINES += "-D NO_LOGGING"
endif

ifeq ($(QMESH),true)
   DEFINES  += "-D  CONFIG_QMESH_DEMO"
   DEFINES  += "-D  PLATFORM_QUARTZ" "-D PLATFORM_MULTITHREAD_SUPPORT"
   DEFINES  += "-D  ENABLE_PROVISIONING"
endif

ifeq ($(CFG_FEATURE_COEX),true)
  DEFINES  += "-D CONFIG_QMESH_COEX_DEMO"
  DEFINES  += "-D ENABLE_THROUGHPUT_TESTING"
endif

ifeq ($(ENABLE_CPU_PROFILER),1)
   INCLUDES += -I"$(SRCDIR)/cpu_profiler"
   DEFINES  += "-D ENABLE_CPU_PROFILER"
endif

ifeq ($(ENABLE_DBGCALL),1)
   INCLUDES += -I"$(ROOTDIR)/quartz/sys/dbgcall/include"
   DEFINES  += "-D ENABLE_DBGCALL"
   LIBS     += "$(LIBDIR)/dbgcall.lib" \
               "$(LIBDIR)/swintr.lib"
endif

ifeq ($(CFG_FEATURE_FLASHLOG),true)
   CSRCS += flashlog/flashlog_demo.c
   INCLUDES +=-I"$(SRCDIR)/flashlog"
   DEFINES += "-D CONFIG_FLASHLOG_DEMO"
endif

DEFINES  += "-D HTC_SYNC"
DEFINES  += "-D DEBUG"

DEFINES += "-D V2" \
		  "-D qurt_mutex_init(x)=qurt_mutex_create(x)" \
		  "-D qurt_mutex_destroy(x)=qurt_mutex_delete(x)" \
		  "-D qurt_signal_init(x)=qurt_signal_create(x)" \
		  "-D qurt_signal_destroy(x)=qurt_signal_delete(x)" \
		  "-D FEATURE_QUARTZ_V2"


# Add the correct NVM library and user NVM file.
LIBS += $(LIBDIR)/QCA$(subst qca,,$(CHIPSET_VARIANT))_$(CHIPSET_REVISION)_$(subst dut,D,$(subst carrier,C,$(subst cdb,CDB,$(BOARD_VARIANT))))_NVM.lib

ifndef NVM_FILE
   NVM_FILE = $(NVMDIR)/config/$(subst p,.,$(CHIPSET_REVISION))/$(subst qca,,$(CHIPSET_VARIANT))/$(subst dut,DUT,$(subst carrier,Carrier,$(subst cdb,CDB,$(BOARD_VARIANT))))/QCA$(subst qca,,$(CHIPSET_VARIANT))_$(CHIPSET_REVISION).nvm
endif

CFLAGS         = $(COPTS) $(DEFINES) $(INCLUDES) -D_WANT_IO_C99_FORMATS
LDOpts        := -eSBL_Entry -no-wchar-size-warning --no-warn-mismatch -R"$(SYMFILE)" -R"$(SYMFILEUNPATCHED)" -T"$(LINKFILE)" -Map="$(OUTDIR)/$(PROJECT).map" -n --gc-sections
LDFLAGS        = $(LDOpts) --start-group @$(LIBSFILE) --end-group
OBJS          := $(CSRCS:%.c=$(OBJDIR)/%.o)
MESHOBJS      := $(MESHCSRCS:%.c=$(OBJDIR)/%.o)
THIRDPARTYOBJS := $(CThirdPartySrcs:%.c=$(OBJDIR)/%.o)
ASSEMBLY_OBJS := $(ASSEMBLY_SRCS:%.S=$(OBJDIR)/%.asm_o)
DEPS          := $(CSRCS:%.c=%.d)
ExtraLibs     :=

ifeq ($(ECOSYSTEM), awsiot)
    ExtraLibs += -u _scanf_float -u _printf_float -lc -lnosys -lgcc -lm
    LDFLAGS += -L"$(NEWLIBPATH)" -L"$(TOOLLIBPATH)" -utlsio_template_interface_description -utlsio_template_get_interface_description $(ExtraLibs)
endif


ifeq ($(ECOSYSTEM), azure)
    ExtraLibs += -u _scanf_float -u _printf_float -lc -lnosys -lgcc -lm
    LDFLAGS += -L"$(NEWLIBPATH)" -L"$(TOOLLIBPATH)" -utlsio_template_interface_description -utlsio_template_get_interface_description $(ExtraLibs)
    CThirdPartyOpts=-c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -fno-exceptions -ffunction-sections -w -O1
    CThirdPartyFlags=$(CThirdPartyOpts) $(DEFINES) $(INCLUDES) -D_WANT_IO_C99_FORMATS
endif

$(THIRDPARTYOBJS): CFLAGS := $(CThirdPartyFlags)
all: prepare propgen nvm $(OBJS) $(MESHOBJS) $(THIRDPARTYOBJS) $(ASSEMBLY_OBJS)
	-rm -rf $(LIBSFILE)
	-rm -rf $(LINKFILE)

	$(foreach f,$(OBJS),echo $(addprefix $(OBJDIR)/, $(notdir $(f)))>>$(LIBSFILE);)
	$(foreach f,$(MESHOBJS),echo $(addprefix $(OBJDIR)/, $(notdir $(f)))>>$(LIBSFILE);)
	$(foreach f,$(THIRDPARTYOBJS),echo $(addprefix $(OBJDIR)/, $(notdir $(f)))>>$(LIBSFILE);)
	$(foreach f,$(LIBS),echo $f>>$(LIBSFILE);)
	$(foreach f,$(PATCHOBJS),echo $f>>$(LIBSFILE);)
	$(foreach f,$(ASSEMBLY_OBJS),echo $(OBJDIR)/$(notdir $(f))>>$(LIBSFILE);)

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
	rm -rf 4020

clobber:
	rm -rf $(SRCDIR)/export
	rm -rf $(SRCDIR)/UsrEDL.c
	rm -rf $(OUTDIR)
	rm -rf 4020

prepare:

	@echo \##############################################################################
	@echo    Starting build for QCA402X QCLI Application
	@echo \##############################################################################

	@echo Build Parameters:
	@echo RTOS		= $(RTOS)
	@echo Variant	= $(CHIPSET_VARIANT)
	@echo Board		= $(BOARD_VARIANT)
	@echo Revision	= $(CHIPSET_REVISION)
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
