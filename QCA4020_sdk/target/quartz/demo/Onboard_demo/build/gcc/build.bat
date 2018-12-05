@ECHO OFF
REM Copyright (c) 2018 Qualcomm Technologies, Inc.
REM All Rights Reserved.
REM Copyright (c) 2018 Qualcomm Technologies, Inc.
REM All rights reserved.
REM Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below)
REM provided that the following conditions are met:
REM Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
REM Redistributions in binary form must reproduce the above copyright notice,
REM this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
REM Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived
REM from this software without specific prior written permission.
REM NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
REM BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
REM IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
REM OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
REM LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
REM WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
REM EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

SETLOCAL EnableDelayedExpansion
SET ONBOARD_VIA=%3
SET RADIO=%4

set count=0
for %%x in (%*) do Set /A count+=1

IF "%Ecosystem%" == "offline" (
    echo %Ecosystem%
    IF  NOT "%count%" GEQ "5" (
        goto Offline_demo
        rem goto end
        goto end
    )
) ELSE IF "%Ecosystem%" == "awsiot" (
    echo %Ecosystem%
	SET MODE=%5
    IF  NOT "%count%" GEQ "6" (
        goto online_demo
        goto end
    )
) ELSE (
    echo  Please Export the ECOSYSTEM before building the demo
    goto  EndOfFile
)

IF "%ONBOARD_VIA%" == "1" (
    set ONBOARD_VIA=BLE
) ELSE IF "%ONBOARD_VIA%" == "2" (
 set ONBOARD_VIA=WIFI
) ELSE (
    echo Incorrect Onboarding mode
    echo.
     IF "%Ecosystem%" == "offline" (
        goto Offline_demo
        goto end
    )
    ELSE (
        goto online_demo
        goto end
    )
)

IF "%RADIO%" == "1" (
    set RADIO=ZIGBEE
    echo.
    rem goto zigbee
) ELSE IF "%RADIO%" == "2" (
    set RADIO=THREAD
    echo.
    rem goto thread
) ELSE (
    echo Incorrect RADIO@echo.
    IF "%Ecosystem%" == "offline" (
        goto Offline_demo
        goto end
    )
    ELSE (
        rem SET MODE=%5
		goto online_demo
		goto end
    )
)

IF "%Ecosystem%" == "offline" (
    IF "%ONBOARD_VIA%" == "WIFI" (
        echo Offline demo supports BLE Onboarding mode Only
        echo.
        goto Offline_demo
        goto end
    )
    IF "%RADIO%" == "THREAD" (
        echo Offline demo supports ZIGBEE RADIO Only
        echo.
        goto Offline_demo
        goto end
    )
    set MODE=5
    goto end
) ELSE IF  "%Ecosystem%" == "awsiot" (
	IF "%RADIO%" == "ZIGBEE" (
        IF "%MODE%" == "1" (
            set MODE=4
            echo Selected Radio Is %RADIO%
            echo Selected Zigbee mode is COORDINATOR
            echo.
            goto end
        ) ELSE IF "%MODE%" == "2" (
            set MODE=1
            echo Selected Radio Is %RADIO%
            echo Selected Zigbee mode is ENDDEVICE
            echo.
            goto end
        ) ELSE (
           echo Incorrect Zigbee Mode
           goto Online_demo
           goto end
        )
    ) ELSE IF "%RADIO%" == "THREAD" (
        IF "%MODE%" == "1" (
            set MODE=4
            echo Selected Radio Is %RADIO%
            echo Selected Thread Mode is BORDER ROUTER
            echo.
            goto end
        ) ELSE IF "%MODE%" == "2" (
            set MODE=1
            echo Selected Radio Is %RADIO%
            echo Selected Thread Mode is JOINER
            echo.
            goto end
        ) ELSE (
            echo Incorrect Thread Mode
            echo.
            goto Online_demo
            goto end
        )
    )
)
:Online_demo
echo Online demo build script help:
echo build.bat [RTOS (FreeRTOS = f, ThreadX = t)] [chipset variant (4020, 4024)] [Onboarding mode] [Radio] [Supported mode] [board variant (CDB = cdb)]
echo  Select the options for Onboarding mode, Radio, Supported Mode as below
echo  Onboarding mode:   1.BLE 2.WIFI
echo  Radio:   1.ZIGBEE 2.THREAD
echo  Thread Supported mode:   1.JOINER ROUTER 2.JOINER
echo  Zigbee Supported mode:   1.COORDINATOR 2.ENDDEVICE
echo Example: build.bat t 4020 1 1 1 cdb
goto EndOfFile


:Offline_demo
echo Offline demo build script help:
echo build.bat [RTOS (FreeRTOS = f, ThreadX = t)] [chipset variant (4020, 4024)] [ONBOARD_VIA] [Radio] [board variant (CDB = cdb)]
echo  Select the options for Radio, Supported Mode as below
echo  Onboarding mode:   1.BLE
echo  Radio:   1.ZIGBEE
echo Example: build.bat t 4020 1 1 cdb
goto EndOfFile

:end

REM Onboard Settings WIFI for via WIFI and BLE for via BLE
REM SET ONBOARD_VIA=WIFI
SET DEVICE_BATTERY_OPERATED=0

REM Default CHIPSET_VERSION is v2
IF /I "%CHIPSET_VERSION%" == "" (SET CHIPSET_VERSION=v2)

REM Determine the RTOS to build. Default to freertos.
IF /I "%~1" == "" (
   SET RTOS=freertos
) ELSE IF /I "%~1" == "t" (
   SET RTOS=threadx
) ELSE IF /I "%~1" == "f" (
   SET RTOS=freertos
) ELSE IF /I "%~1" == "prepare" (
REM special command, will export devcfg files
   SET RTOS=freertos
) ELSE IF /I "%~1" == "clobber" (
REM special command, will delete devcfg files
   SET RTOS=freertos
) ELSE (
   ECHO Invalid RTOS: %1
   GOTO:Usage
)


REM Validate the chipset variant. Default: 4020
IF /I "%~2" == "" (
   SET CHIPSET_VARIANT=qca4020
   SET BOARD_SUPPORTS_WIFI=1
) ELSE IF /I "%2" == "4020" (
   SET CHIPSET_VARIANT=qca4020
   SET BOARD_SUPPORTS_WIFI=1
) ELSE IF /I "%2" == "4024" (
   SET CHIPSET_VARIANT=qca4024
   SET BOARD_SUPPORTS_WIFI=0
) ELSE IF /I "%2" == "4025" (
   SET CHIPSET_VARIANT=qca4025
   SET BOARD_SUPPORTS_WIFI=0
) ELSE (
   ECHO Invalid chipset variant: %2%
   GOTO:Usage
)

REM Validate the board variant  -- carrier, dut and CDB. Default: carrier
IF /I "%~6" == "" (
   SET BOARD_VARIANT=carrier
   SET NVM_LIB_VARIANT=C
) ELSE IF /I "%6" == "c" (
   SET BOARD_VARIANT=carrier
   SET NVM_LIB_VARIANT=C
) ELSE IF /I "%6" == "d" (
   SET BOARD_VARIANT=DUT
   SET NVM_LIB_VARIANT=D
) ELSE IF /I "%6" == "cdb" (
   SET BOARD_VARIANT=CDB
   SET NVM_LIB_VARIANT=CDB
) ELSE (
   ECHO Invalid board variant: %6%
   GOTO:Usage
)
IF "%Ecosystem%" == "offline" (
	SET BOARD_VARIANT=CDB
    SET NVM_LIB_VARIANT=CDB
)
ECHO %BOARD_VARIANT%
REM Validate the chipset revision. Default: 1.2 for v1, 2.0 for v2
IF /I "%~7" == "" (
   IF /I "%CHIPSET_VERSION%" == "v2" (
      SET CHIPSET_REVISION=2p0
   ) ELSE (
      SET CHIPSET_REVISION=1p2
   )
) ELSE IF /I "%7" == "1p2" (
   SET CHIPSET_REVISION=1p2
) ELSE IF /I "%7" == "2p0" (
   SET CHIPSET_REVISION=2p0
) ELSE (
   ECHO Invalid chipset revision: %7%
   GOTO:Usage
)

REM Get the path of the optional custom NVM file.
IF /I "%~8" == "" (
   SET NVM_FILE=
) ELSE (
   IF EXIST "%~8" (
      SET NVM_FILE=%8
   ) ELSE (
      ECHO Invalid NVM file path: %8%
      GOTO:Usage
   )
)

@ECHO ****************************************************************************
@ECHO                      Building Onboard AWS demo for %CHIPSET_VERSION% Chipset
@ECHO                      RTOS              %RTOS%
@ECHO                      CHIPSET VARIANT   %CHIPSET_VARIANT%
@ECHO                      Onboard Via       %ONBOARD_VIA%
@ECHO                      Battery Operated  %DEVICE_BATTERY_OPERATED%
@ECHO                      Ecosystem         %Ecosystem%
@ECHO *****************************************************************************

IF /I %BOARD_SUPPORTS_WIFI% == 0 (
   IF /I %ONBOARD_VIA% == WIFI (
       ECHO Invalid onboarding mode. Chipset does not provide Wi-Fi functionality
       ECHO Set right value to ONBOARD_VIA variable
       ECHO Example : build.bat t 4024 1 1 2 cdb
       ECHO  Select the options for  Onboarding mode, Radio, Supported Mode as below
       ECHO  Onboarding mode:   1.BLE 2.WIFI
       ECHO  Radio:   1.ZIGBEE 2.THREAD
       ECHO  Thread Supported mode:   1.JOINER ROUTER 2.JOINER
       ECHO  Zigbee Supported mode:   1.ROUTER 2.ENDDEVICE												
       GOTO:EndOfFile
   )
)

REM Setup the paths for the build
SET Project=Quartz
SET RootDir=..\..\..\..\..
SET SrcDir=..\..\src
SET NvmDir=%RootDir%\quartz\nvm
SET OutDir=output
SET ObjDir=%OutDir%\objs
SET SectoolsDir=%RootDir%\sectools
SET SectoolsQdnDir=%RootDir%\sectools\qdn
SET SectoolsCertsDir=%SectoolsQdnDir%\resources\data_prov_assets\Signing\Local\qc_presigned_certs-key2048_exp257
SET SECBOOT=false
SET LibDir=%RootDir%\lib\cortex-m4IPT\%RTOS%
SET SymFile="%RootDir%\bin\cortex-m4\IOE_ROM_IPT_IMG_ARNNRI_gcc.sym"
SET SymFileUnpatched="%RootDir%\bin\cortex-m4\IOE_ROM_IPT_IMG_ARNNRI_orig_fcns_gcc.sym"
SET ScriptDir=%RootDir%\build\scripts
SET LinkerScriptDir=%RootDir%\build\scripts\linkerScripts
SET LINKFILE="%OutDir%\%Project%.ld"
SET LIBSFILE="%OutDir%\LinkerLibs.txt"
SET ThirdpartyDir=..\..\..\..
SET EcosystemRoot=..\..\..\ecosystem
SET MeshModelsCommonCode=..\..\..\..\qmesh\models\common
SET MeshClientModels=..\..\..\..\qmesh\models\client
SET MeshServerModels=..\..\..\..\qmesh\models\server

REM Prepare Command, copy device config files to export directory
IF /I "%1" == "prepare" (
   SET exitflag=true
   GOTO Prepare
)

REM Clobber command, delete object files and export directory
IF /I "%1" == "clobber" (
   SET exitflag=true
   GOTO Clobber
)

IF /I "%RTOS%" == "threadx" (
   SET Libs="%LibDir%\threadx.lib"
   SET Libs=!Libs! "%LibDir%\mom_patch_table_ARNTRI_qcm.o"
   SET Libs=!Libs! "%LibDir%\fom_patch_table_ARNTRI_qcm.o"
   SET Libs=!Libs! "%LibDir%\som_patch_table_ARNTRI_qcm.o"
) ELSE (
   SET Libs="%LibDir%\free_rtos.lib"
   SET Libs=!Libs! "%LibDir%\mom_patch_table_ARNFRI_qcm.o"
   SET Libs=!Libs! "%LibDir%\fom_patch_table_ARNFRI_qcm.o"
   SET Libs=!Libs! "%LibDir%\som_patch_table_ARNFRI_qcm.o"
)

REM Sources to compile
SET CWallSrcs=
SET CSrcs=sbrk.c
SET CSrcs=%CSrcs% onboard.c
SET CSrcs=%CSrcs% utils\util.c
SET CSrcs=%CSrcs% utils\led_utils.c
SET CSrcs=%CSrcs% utils\wifi_util.c
SET CSrcs=%CSrcs% utils\net_util.c
SET CSrcs=%CSrcs% utils\ble_util.c
SET CSrcs=%CSrcs% ble\ble_zigbee_service.c
SET CSrcs=%CSrcs% ble\ble_thread_service.c
IF /I "%ONBOARD_VIA%" == "WIFI" (
   SET CSrcs=!CSrcs! onboard_via_wifi.c
)
IF /I "%ONBOARD_VIA%" == "BLE" (
   SET CSrcs=!CSrcs! onboard_via_ble.c
   SET CSrcs=!CSrcs! ble\ble_wifi_service.c
)
SET CSrcs=%CSrcs% qcli\qcli.c
SET CSrcs=%CSrcs% qcli\qcli_util.c
SET CSrcs=%CSrcs% qcli\pal.c
SET CSrcs=%CSrcs% lp\lp_demo.c
SET CSrcs=%CSrcs% lp\fom_lp_test.c
SET CSrcs=%CSrcs% lp\som_lp_test.c
SET CSrcs=%CSrcs% lp\mom_lp_test.c
SET CSrcs=%CSrcs% thread\thread_joiner.c
SET CSrcs=%CSrcs% thread\thread_util.c
SET CSrcs=%CSrcs% thread\thread_udp_joiner.c
SET CSrcs=%CSrcs% thread\thread_border_router.c
SET CSrcs=%CSrcs% thread\thread_udp_border_router.c
SET CSrcs=%CSrcs% thread\thread_router.c
SET CSrcs=%CSrcs% zigbee\zigbee_util.c
SET CSrcs=%CSrcs% zigbee\zigbee_coordinator.c
SET CSrcs=%CSrcs% zigbee\zigbee_router.c
SET CSrcs=%CSrcs% zigbee\zigbee_enddevice.c
SET CSrcs=%CSrcs% zigbee\zcl_util.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_alarms_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_basic_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_colorcontrol_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_custom_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_groups_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_identify_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_levelcontrol_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_onoff_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_ota_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_scenes_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_time_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_touchlink_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_powerconfig_demo.c
SET CSrcs=%CSrcs% zigbee\clusters\zcl_devicetemp_demo.c
SET CSrcs=%CSrcs% sensors\sensors.c
SET CSrcs=%CSrcs% sensors\pir_int.c
SET CSrcs=%CSrcs% export\platform_oem.c
SET CSrcs=%CSrcs% export\platform_oem_som.c
SET CSrcs=%CSrcs% export\platform_oem_mom.c
SET CSrcs=%CSrcs% export\DALConfig_devcfg.c
SET CSrcs=%CSrcs% export\DALConfig_fom.c
SET CSrcs=%CSrcs% export\devcfg_devcfg_data.c
SET CSrcs=%CSrcs% export\devcfg_fom_data.c
SET CSrcs=%CSrcs% export\UsrEDL.c

IF "%ENABLE_CPU_PROFILER%"=="1" (
    SET CSrcs=!CSrcs! cpu_profiler\cpu_profiler_demo.c
    SET CSrcs=!CSrcs! cpu_profiler\cpu_profiler_interrupt_asm.S
)

REM Include directories
SET Includes=-I"%RootDir%\include"
SET Includes=%Includes% -I"%RootDir%\include\qapi"
SET Includes=%Includes% -I"%RootDir%\include\bsp"

SET Includes=%Includes% -I"%SrcDir%\qcli"
SET Includes=%Includes% -I"%SrcDir%\net"
SET Includes=%Includes% -I"%SrcDir%\lp"
SET Includes=%Includes% -I"%SrcDir%\fs"
SET Includes=%Includes% -I"%SrcDir%\wifi"
SET Includes=%Includes% -I"%SrcDir%\platform"
SET Includes=%Includes% -I"%SrcDir%\ecosystem"
SET Includes=%Includes% -I"%SrcDir%\include"
SET Includes=%Includes% -I"%SrcDir%\ecosystem\aws"
SET Includes=%Includes% -I"%SrcDir%\zigbee\clusters"
SET Includes=%Includes% -I"%RootDir%\thirdparty\aws\awsiot\include"
SET Includes=%Includes% -I"%RootDir%\thirdparty\jsmn\include"
SET Includes=%Includes% -I"%RootDir%\quartz\ecosystem\aws\port\include"
SET Includes=%Includes% -I"%SrcDir%\sensors"

IF /I "%ENABLE_CPU_PROFILER%" == "1" (
   SET Includes=%Includes% -I"%SrcDir%\cpu_profiler"
)


IF /I "%Ecosystem%" == "awsiot" (
   GOTO:awsiot
) ELSE (
   GOTO:skip_awsiot
)
:awsiot

IF /I "%CHIPSET_VARIANT%" == "qca4020" (
	GOTO:qca4020
) ELSE (
	GOTO:skip_qca4020
)
:qca4020
   SET CSrcs=%CSrcs% ecosystem\aws\aws_run.c
   SET CSrcs=%CSrcs% sensors\sensor_json.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\quartz\ecosystem\aws\port\timer.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\quartz\ecosystem\\aws\port\network_qca4020_wrapper.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\jsmn.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_json_utils.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_mqtt_client.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_mqtt_client_common_internal.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_mqtt_client_connect.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_mqtt_client_publish.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_mqtt_client_subscribe.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_mqtt_client_unsubscribe.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_mqtt_client_yield.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_shadow.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_shadow_actions.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_shadow_records.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\aws\awsiot\aws_iot_shadow_json.c

   SET Includes=%Includes% -I"%RootDir%\thirdparty\aws\awsiot\include"
   SET Includes=%Includes% -I"%RootDir%\quartz\ecosystem\aws\port\include"
   SET Includes=%Includes% -I"%SrcDir%\ecosystem\aws\shadow_sample"

:skip_qca4020

IF /I "%CHIPSET_VARIANT%" == "qca4024" (
	GOTO:qca4024
) ELSE (
	GOTO:skip_qca4024
)
:qca4024
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\jsmn\src\jsmn.c
   SET CSrcs=%CSrcs% sensors\sensor_json.c

:skip_qca4024

:skip_awsiot

IF /I "%Ecosystem%" == "offline" (
   GOTO:offline
) ELSE (
   GOTO:skip_offline
)
:offline
   SET CSrcs=%CSrcs% ble\ble_offline_service.c
   SET CSrcs=%CSrcs% ecosystem\offline\offline.c
   SET CSrcs=%CSrcs% %ThirdpartyDir%\thirdparty\jsmn\src\jsmn.c
   SET CSrcs=%CSrcs% sensors\sensor_json.c

   SET Includes=%Includes% -I"%RootDir%\thirdparty\jsmn\include"

:skip_offline



REM External objects and libraries
SET Libs=%Libs% "%LibDir%\core.lib"
SET Libs=%Libs% "%LibDir%\qurt.lib"
SET Libs=%Libs% "%LibDir%\quartzplatform.lib"
SET Libs=%Libs% "%LibDir%\quartzplatform_xip.lib"
SET Libs=%Libs% "%LibDir%\WLAN.lib"
SET Libs=%Libs% "%LibDir%\WLAN_PROFILER.lib"
SET Libs=%Libs% "%LibDir%\net.lib"
SET Libs=%Libs% "%LibDir%\net_ram.lib"
SET Libs=%Libs% "%LibDir%\dhcpv6c.lib"
SET Libs=%Libs% "%LibDir%\sntpc.lib"
SET Libs=%Libs% "%LibDir%\dnssrvr.lib"
SET Libs=%Libs% "%LibDir%\sharkssl.lib"
SET Libs=%Libs% "%LibDir%\csr.lib"
SET Libs=%Libs% "%LibDir%\cryptolib.lib"
SET Libs=%Libs% "%LibDir%\httpsvr.lib"
SET Libs=%Libs% "%LibDir%\httpc.lib"
SET Libs=%Libs% "%LibDir%\mqttc.lib"
SET Libs=%Libs% "%LibDir%\vfs.lib"
SET Libs=%Libs% "%LibDir%\userpass.lib"
SET Libs=%Libs% "%LibDir%\i2s.lib"
SET Libs=%Libs% "%LibDir%\master_sdcc.lib"
SET Libs=%Libs% "%LibDir%\fwup.lib"
SET Libs=%Libs% "%LibDir%\fwup_engine.lib"
SET Libs=%Libs% "%LibDir%\qapi_ed25519.lib"
SET Libs=%Libs% "%LibDir%\qapi_securefs.lib"
SET Libs=%Libs% "%LibDir%\pka_port.lib"
SET Libs=%Libs% "%LibDir%\fs_helper.lib"
SET Libs=%Libs% "%LibDir%\quartz_crypto_qapi.lib"
SET Libs=%Libs% "%LibDir%\zigbee.lib"
SET Libs=%Libs% "%LibDir%\quartz_zigbee.lib"
SET Libs=%Libs% "%LibDir%\thread.lib"
SET Libs=%Libs% "%LibDir%\qapi_thread.lib"
SET Libs=%Libs% "%LibDir%\BLUETOPIA_SERVICES.lib"
SET Libs=%Libs% "%LibDir%\BLUETOPIA_QAPI_SERVICES.lib"
SET Libs=%Libs% "%LibDir%\mdns.lib"
SET Libs=%Libs% "%LibDir%\dnssd.lib"
SET Libs=%Libs% "%LibDir%\otp_tlv.lib"
SET Libs=%Libs% "%LibDir%\base64.lib"
SET Libs=%Libs% "%LibDir%\PERSIST_M4.lib"
SET Libs=%Libs% "%LibDir%\json.lib"
SET Libs=%Libs% "%LibDir%\json_qapi.lib"
SET Libs=%Libs% "%LibDir%\master_sdcc.lib"
SET Libs=%Libs% "%LibDir%\nichestack.lib"
SET Libs=!Libs! "%LibDir%\EDLManager.lib"
SET Libs=!Libs! "%LibDir%\tlv_transport.lib"
SET Libs=!Libs! "%LibDir%\crypto_port.lib"
SET Libs=!Libs! "%LibDir%\tee_master.lib"
SET Libs=!Libs! "%LibDir%\dnsclient.lib"
SET Libs=!Libs! "%LibDir%\securefs.lib"
SET Libs=!Libs! "%LibDir%\securefs_port.lib"
SET Libs=!Libs! "%LibDir%\v2core.lib"
REM Place all v2 patches here
SET PatchObjs=!PatchObjs! "%LibDir%\patch.lib"


IF /I "%CHIPSET_VARIANT%" == "qca4020" (
	GOTO:Wlan_lib
) ELSE (
	GOTO:Skip_Wlan_lib
)

:Wlan_lib
SET Libs=%Libs% "%LibDir%\WLAN_QAPI.lib"
SET Libs=%Libs% "%LibDir%\CUST_IPSTACK_INICHE.lib"
SET Libs=%Libs% "%LibDir%\wlan_lib_common_xip.lib"
SET Libs=%Libs% "%LibDir%\cust_wlan_lib.lib"

:Skip_Wlan_lib

IF /I "%QMESH%"=="true" (
	SET Libs=!Libs! "%LibDir%\qmesh.lib"
)

REM Setup the build variables
SET Compiler=arm-none-eabi-gcc
SET ObjCopy=arm-none-eabi-objcopy
SET Archiver=arm-none-eabi-ar
SET Linker=arm-none-eabi-ld
SET ARM_OBJDUMP=arm-none-eabi-objdump

SET COpts=-c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -fno-exceptions -O2 -ffunction-sections -Wall

SET Defines=-D WLAN_DEBUG -D ENABLE_P2P_MODE -D SUPPORTED_MODE=%MODE%
SET Defines=!Defines! "-D JSMN_PARENT_LINKS=1"
REM SET Defines=%Defines% "-D ENABLE_IOT_DEBUG"
REM SET Defines=%Defines% "-D ENABLE_IOT_ERROR"
REM SET Defines=%Defines% "-D ENABLE_IOT_WARN"
REM SET Defines=%Defines% "-D ENABLE_IOT_TRACE"

IF /I "%CHIPSET_VERSION%" == "v1" (
   SET Defines=!Defines! "-D V1"
) ELSE (
   SET Defines=!Defines! "-D V2"
   SET Defines=!Defines! "-D qurt_mutex_init(x)=qurt_mutex_create(x)"
   SET Defines=!Defines! "-D qurt_mutex_destroy(x)=qurt_mutex_delete(x)"
   SET Defines=!Defines! "-D qurt_signal_init(x)=qurt_signal_create(x)"
   SET Defines=!Defines! "-D qurt_signal_destroy(x)=qurt_signal_delete(x)"
   SET Defines=!Defines! "-D qurt_mutex_destroy(x)=qurt_mutex_delete(x)"
   SET Defines=!Defines! "-D FEATURE_QUARTZ_V2"
)

IF /I "%ENABLE_CPU_PROFILER%" == "1" (
   SET Defines=!Defines! "-D ENABLE_CPU_PROFILER"
)

IF /I "%ENABLE_DBGCALL%"=="1" (
   SET Includes=%Includes% -I"%RootDir%\quartz\sys\dbgcall\include"
   SET Defines=!Defines! "-D ENABLE_DBGCALL"
   SET Libs=!Libs! "%LibDir%\dbgcall.lib"
   SET Libs=!Libs! "%LibDir%\swintr.lib"
)

IF /I "%Ecosystem%" == "awsiot" (
   SET Defines=!Defines! "-D AWS_IOT"
   SET Defines=!Defines! "-D JSMN_PARENT_LINKS=1"
REM This is required has AWS is not working with CALIST
   SET Defines=!Defines! "-D DISABLE_CA_LIST"
   SET Defines=!Defines! "-D ENABLE_DIMMER = 0"
)
IF /I "%Ecosystem%" == "offline" (
   SET Defines=!Defines! "-D OFFLINE"
   SET Defines=!Defines! "-D JSMN_PARENT_LINKS=1"
REM This is required has AWS is not working with CALIST
   SET Defines=!Defines! "-D DISABLE_CA_LIST"
   SET Defines=!Defines! "-D ENABLE_DIMMER = 0"
)
SET Defines=!Defines! "-D CHIPSET_VARIANT=\"%CHIPSET_VARIANT%\""
SET Defines=!Defines! "-D RTOS=\"%RTOS%\""

IF /I "%BOARD_VARIANT%" == "CDB" (SET Defines=!Defines! "-D CONFIG_CDB_PLATFORM")
IF /I %BOARD_SUPPORTS_WIFI% == 1 (
   SET Defines=!Defines! "-D BOARD_SUPPORTS_WIFI=1"
)
IF /I %DEVICE_BATTERY_OPERATED% == 1 (
   SET Defines=!Defines! "-D DEVICE_BATTERY_OPERATED=1"
) ELSE (
   SET Defines=!Defines! "-D DEVICE_BATTERY_OPERATED=0"
)

IF /I "%ONBOARD_VIA%" == "WIFI" (
   SET Defines=!Defines! "-D ONBOARD_VIA_WIFI"
)
IF /I "%ONBOARD_VIA%" == "BLE" (
   SET Defines=!Defines! "-D ONBOARD_VIA_BLE"
)
IF /I "%RADIO%" ==  "THREAD" (
   SET Defines=!Defines! "-D THREAD"
)
IF /I "%RADIO%" ==  "ZIGBEE" (
   SET Defines=!Defines! "-D ZIGBEE"
)


SET ExtraLibs=

SET CFlags=%COpts% %Defines% %Includes% -D_WANT_IO_C99_FORMATS

SET LDFlags=-eSBL_Entry -no-wchar-size-warning --no-warn-mismatch -R"%SymFile%" -R"%SymFileUnpatched%" -T"%LINKFILE%" -Map="%OutDir%\%Project%.map" -n --gc-sections

IF /I "%Ecosystem%" == "awsiot" (
   echo "Building for AWS"
   SET LDFlags=!LDFlags! -L"%NEWLIBPATH%" -L"%TOOLLIBPATH%"
   SET ExtraLibs=!ExtraLibs! -lc_nano -lnosys -lgcc -u _scanf_float
) ELSE IF /I "%Ecosystem%" == "azure" (
   SET LDFlags=!LDFlags! -L"%NEWLIBPATH%\v7e-m" -L"%TOOLLIBPATH%\v7e-m" -u tlsio_template_interface_description -u tlsio_template_get_interface_description
   SET ExtraLibs=!ExtraLibs! -lc_nano -lnosys -lgcc -lm
   SET CThirdPartyOpts=-c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -fno-exceptions -ffunction-sections -w -O2
   SET CThirdPartyFlags=!CThirdPartyOpts! !Defines! !Includes! -D_WANT_IO_C99_FORMATS
) ELSE IF /I "%Ecosystem%" == "offline" (
   REM LDFlags=!LDFlags! -L"%NEWLIBPATH%\v7e-m" -L"%TOOLLIBPATH%\v7e-m" -u tlsio_template_interface_description -u tlsio_template_get_interface_description
   REM ExtraLibs=!ExtraLibs! -lc_nano -lnosys -lgcc -lm
   SET LDFlags=!LDFlags! -L"%NEWLIBPATH%" -L"%TOOLLIBPATH%"
   SET ExtraLibs=!ExtraLibs! -lc_nano -lnosys -lgcc -u _scanf_float
)

REM Clean the output directory. Note the delay is to give Windows an opportunity to remove the old directory tree
RMDIR /s/q "%OutDir%" >NUL 2>&1
TIMEOUT /t 1          >NUL 2>&1
RMDIR /s/q "4020" >NUL 2>&1
MKDIR "%OutDir%"
MKDIR "%ObjDir%"

REM Clean up EDL files.
DEL /F "%SrcDir%\export\UsrEDL.c" >NUL 2>&1

SET Libs=!Libs! "%LibDir%\%CHIPSET_VARIANT%_%CHIPSET_REVISION%_%NVM_LIB_VARIANT%_NVM.lib"

IF "%NVM_FILE%" == "" (
   SET NVM_FILE=%NvmDir%\config\%CHIPSET_REVISION:~0,1%.%CHIPSET_REVISION:~2,1%\%CHIPSET_VARIANT:~3,4%\%BOARD_VARIANT%\%CHIPSET_VARIANT%_%CHIPSET_REVISION%.nvm
)

python %NvmDir%\tool\NVM2C.py -o %SrcDir%\export\UsrEDL.c -i %NVM_FILE%

if errorlevel 1 goto EndOfFile

:Prepare

ECHO Exporting Device config files....

MKDIR %SrcDir%\export >NUL 2>&1

robocopy %RootDir%\quartz\platform\export\ %SrcDir%\export platform_oem.h /XO >NUL 2>&1
robocopy %RootDir%\quartz\platform\export\ %SrcDir%\export platform_oem.c /XO >NUL 2>&1
robocopy %RootDir%\quartz\platform\export\ %SrcDir%\export platform_oem_som.c /XO >NUL 2>&1
robocopy %RootDir%\quartz\platform\export\ %SrcDir%\export platform_oem_mom.c /XO >NUL 2>&1

IF /I "%RTOS%" == "threadx" (
   robocopy %RootDir%\build\tools\devcfg\threadx\ %SrcDir%\export *.* /XO >NUL 2>&1
) else (
   robocopy %RootDir%\build\tools\devcfg\freertos\ %SrcDir%\export *.* /XO >NUL 2>&1
)

IF /I "%exitflag%" == "true" ( GOTO EndOfFile )

:Propgen
ECHO GENERATING DEVCFG....
DEL /F %SrcDir%\export\DALConfig_devcfg.c >NUL 2>&1
DEL /F %SrcDir%\export\devcfg_devcfg_data.c >NUL 2>&1
DEL /F %SrcDir%\export\DALConfig_fom.c >NUL 2>&1
DEL /F %SrcDir%\export\devcfg_fom_data.c >NUL 2>&1

IF /I "%BOARD_VARIANT%" == "CDB" (
REM CDB board uses "CDB" as BOARD_VARIANT. CDB board and carrier board use different DevCfg XML files.
   python %RootDir%/build/tools/devcfg/propgen.py --XmlFile=%SrcDir%/export/DevCfg_master_devcfg_out_cdb.xml --DirName=%SrcDir%/export --ConfigFile=%SrcDir%/export/DALConfig_devcfg.c --DevcfgDataFile=%SrcDir%/export/devcfg_devcfg_data.c --ConfigType=%CHIPSET_VARIANT%_devcfg_xml
   python %RootDir%/build/tools/devcfg/propgen.py --XmlFile=%SrcDir%/export/DevCfg_master_fom_out_cdb.xml --DirName=%SrcDir%/export --ConfigFile=%SrcDir%/export/DALConfig_fom.c --DevcfgDataFile=%SrcDir%/export/devcfg_fom_data.c --ConfigType=%CHIPSET_VARIANT%_fom_xml
) ELSE (
   python %RootDir%/build/tools/devcfg/propgen.py --XmlFile=%SrcDir%/export/DevCfg_master_devcfg_out.xml --DirName=%SrcDir%/export --ConfigFile=%SrcDir%/export/DALConfig_devcfg.c --DevcfgDataFile=%SrcDir%/export/devcfg_devcfg_data.c --ConfigType=%CHIPSET_VARIANT%_devcfg_xml
   python %RootDir%/build/tools/devcfg/propgen.py --XmlFile=%SrcDir%/export/DevCfg_master_fom_out.xml --DirName=%SrcDir%/export --ConfigFile=%SrcDir%/export/DALConfig_fom.c --DevcfgDataFile=%SrcDir%/export/devcfg_fom_data.c --ConfigType=%CHIPSET_VARIANT%_fom_xml
)

REM Compile the source
FOR %%F IN (%CSrcs%) DO (
   ECHO Building %%F
   "%Compiler%" %CFlags% "-D __FILENAME__=\"%%~nF.c\"" -o"%ObjDir%\%%~nF.o" "%SrcDir%\%%F"
   if errorlevel 1 goto EndOfFile
   SET Libs=!Libs! "%ObjDir%\%%~nF.o"
)

FOR %%F IN (%CWallSrcs%) DO (
   ECHO Building %%F
   "%Compiler%" %CFlags% -Werror "-D __FILENAME__=\"%%~nF.c\"" -o"%ObjDir%\%%~nF.o" "%SrcDir%\%%F"
   if errorlevel 1 goto EndOfFile
   SET Libs=!Libs! "%ObjDir%\%%~nF.o"
)

REM Compile thirdparty source
FOR %%F IN (%CThirdPartySrcs%) DO (
   ECHO Building %%F
   "%Compiler%" %CThirdPartyFlags% "-D __FILENAME__=\"%%~nF.c\"" -o"%ObjDir%\%%~nF.o" "%SrcDir%\%%F"
   if errorlevel 1 goto EndOfFile
   SET Libs=!Libs! "%ObjDir%\%%~nF.o"
)

REM Generate the file with the libraries to link
IF EXIST %LIBSFILE% del %LIBSFILE%
IF EXIST %LINKFILE% del %LINKFILE%

FOR %%F IN (%Libs%) DO (
   ECHO %%F >>%LIBSFILE%
)

FOR %%F IN (%PatchObjs%) DO (
   ECHO %%F >>%LIBSFILE%
)

REM Convert paths to unix style
python fixpaths.py %LIBSFILE%

echo Generating Linker Scripts...
REM Update application PlacementFile
python %LinkerScriptDir%\CreateAppPlacementFile.py %RootDir%\bin\cortex-m4\%RTOS%\sys.placement %RootDir%\bin\cortex-m4\%RTOS%\cust.placement app.config app.placement 2>dbg.CreateApp
if %errorlevel% == 1 (
echo Failed to update application placement file. Check dbg.CreateApp for detail...
goto EndOfFile
)

REM Create a Quartz.ld linker script
python %LinkerScriptDir%\MakeLinkerScript.py %RootDir%\bin\cortex-m4\%RTOS%\DefaultTemplateLinkerScript.ld app.placement %LIBSFILE% > %LINKFILE% 2>dbg.Make
REM python %LinkerScriptDir%\MakeLinkerScript.py %LDFILE% app.placement %LIBSFILE% > %LINKFILE% 2>dbg.Make
if %errorlevel% == 1 (
echo Failed to create linker script file. Check dbg.Make for detail...
goto EndOfFile
)

REM Link appropiate set of files
REM based on the chipset version
ECHO Linking...

IF /I "%CHIPSET_VERSION%" == "v1" (
   REM link the image
   "%Linker%" %LDFlags% --start-group @%LIBSFILE% --end-group %ExtraLibs% -o"%OutDir%\%Project%.elf"
)  ELSE (
   REM link the image
   "%Linker%" %LDFlags% --start-group @%LIBSFILE%  --end-group %ExtraLibs%  -o"%OutDir%\%Project%_nocompact.elf"
)
   REM Run the diag compaction script to generate the final ELF
   python %ScriptDir%\diagMsgCompact.py %OutDir%\%Project%.elf %RootDir%\bin\cortex-m4\diag_msg_QCLI_demo.strdb %OutDir%\%Project%_nocompact.elf %RootDir%/bin/cortex-m4/diag_msg.pkl Final > dictLog
)

if %errorlevel% == 1 goto EndOfFile

REM Hash
ECHO Hashing...
python %SCRIPTDIR%\createxbl.py -f%OUTDIR%\%PROJECT%.elf -a32 -o%OUTDIR%\%PROJECT%_HASHED.elf
if /I "%SECBOOT%" == "true" (
IF /I "%BOARD_VARIANT%" == "CDB" (
    python %SectoolsQdnDir%\sectools.py iot -p 4020 -g m4 -i %OUTDIR%\%PROJECT%.elf -k %SectoolsCertsDir%\qpsa_rootca.key -c %SectoolsCertsDir%\qpsa_rootca.cer --cfg_oem_id=0xffff --cfg_model_id=0x0000 -o . -s
    python %SectoolsQdnDir%\sectools.py iot -p 4020 -g m0 -i %RootDir%\bin\cortex-m0\threadx\ioe_ram_m0_threadx_ipt.mbn -k %SectoolsCertsDir%\qpsa_rootca.key -c %SectoolsCertsDir%\qpsa_rootca.cer --cfg_oem_id=0xffff --cfg_model_id=0x0000 -o . -s
    python %SectoolsQdnDir%\sectools.py iot -p 4020 -g kf -i %RootDir%\bin\wlan\wlan_fw_img.bin -k %SectoolsCertsDir%\qpsa_rootca.key -c %SectoolsCertsDir%\qpsa_rootca.cer --cfg_oem_id=0xffff --cfg_model_id=0x0000 -o . -s
) ELSE (
   python %SectoolsDir%\sectools.py secimage -i %OUTDIR%\%PROJECT%.elf -c 4020_secimage.xml -sa -g m4 -o .
   python %SectoolsDir%\sectools.py secimage -i %RootDir%\bin\cortex-m0\threadx\ioe_ram_m0_threadx_ipt.mbn -c 4020_secimage.xml -sa -g m0 -o .
   python %SectoolsDir%\sectools.py secimage -i %RootDir%\bin\wlan\wlan_fw_img.bin -c 4020_secimage.xml -sa -g kf -o .
)
)
GOTO:EndOfFile

:Clobber
ECHO Deleting all temporary files ...
RMDIR /s/q "%OutDir%" >NUL 2>&1
RMDIR /s/q %SrcDir%\export >NUL 2>&1
GOTO:EndOfFile

:Usage
ECHO.
REM ECHO Usage: build.bat [RTOS (FreeRTOS = f, ThreadX = t)] [chipset variant (4020, 4024, 4025)] [board variant (Carrier = c, DUT = d, CDB = cdb)] [chipset revision (1.2 = 1p2, 2.0 = 2p0)] [NVM file]
ECHO Usage: build.bat [RTOS (FreeRTOS = f, ThreadX = t)] [chipset variant (4020, 4024, 4025)] [Onbording-mode ( 1.WIFI 2.BLE)] [Radio (1.THREAD 2.ZIGBEE)] [Thread Supported-Mode (1.BORDER ROUTER 2.JOINER ROUTER)] [Zigbee Supported-Mode (1.COORDINATOR 2.ROUTER)] [board variant (Carrier = c, DUT = d, CDB = cdb)] [chipset revision (1.2 = 1p2, 2.0 = 2p0)] [NVM file]
ECHO Example: build.bat t 4020 1 2 1 cdb
GOTO:EndOfFile

:EndOfFile
:exit

