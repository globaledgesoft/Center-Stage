@ECHO OFF
REM
REM Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
REM 2016 Qualcomm Atheros, Inc.
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
) ELSE IF /I "%2" == "4020" (
   SET CHIPSET_VARIANT=qca4020
) ELSE IF /I "%2" == "4024" (
   SET CHIPSET_VARIANT=qca4024
) ELSE IF /I "%2" == "4025" (
   SET CHIPSET_VARIANT=qca4025
) ELSE (
   ECHO Invalid chipset variant: %2%
   GOTO:Usage
)

REM Validate the board variant -- carrier, dut and cdb. Default: carrier
IF /I "%~3" == "" (
   SET BOARD_VARIANT=carrier
   SET NVM_LIB_VARIANT=C
) ELSE IF /I "%3" == "c" (
   SET BOARD_VARIANT=carrier
   SET NVM_LIB_VARIANT=C
) ELSE IF /I "%3" == "d" (
   SET BOARD_VARIANT=DUT
   SET NVM_LIB_VARIANT=D
) ELSE IF /I "%3" == "cdb" (
   SET BOARD_VARIANT=CDB
   SET NVM_LIB_VARIANT=CDB
) ELSE (
   ECHO Invalid board variant: %3%
   GOTO:Usage
)

IF /I "%CHIPSET_VARIANT%" == "qca4020" (
   SET CFG_FEATURE_WLAN=true
) ELSE (
   SET CFG_FEATURE_WLAN=false
)

REM Validate the chipset revision. Default: 1.2 for v1, 2.0 for v2
IF /I "%~4" == "" (
   IF /I "%CHIPSET_VERSION%" == "v2" (
      SET CHIPSET_REVISION=2p0
   ) ELSE (
      SET CHIPSET_REVISION=1p2
   )
) ELSE IF /I "%4" == "1p2" (
   SET CHIPSET_REVISION=1p2
) ELSE IF /I "%4" == "2p0" (
   SET CHIPSET_REVISION=2p0
) ELSE (
   ECHO Invalid chipset revision: %4%
   GOTO:Usage
)

REM Get the path of the optional custom NVM file.
IF /I "%~5" == "" (
   SET NVM_FILE=
) ELSE (
   IF EXIST "%~5" (
      SET NVM_FILE=%5
   ) ELSE (
      ECHO Invalid NVM file path: %5%
      GOTO:Usage
   )
)

@ECHO ****************************************************************************
@ECHO                      Building Quartz QCLI Power Application for %CHIPSET_VERSION% Chipset
@ECHO                      RTOS             %RTOS%
@ECHO                      CHIPSET VARIANT  %CHIPSET_VARIANT%
@ECHO *****************************************************************************

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
SET CSrcs=sbrk.c
SET CSrcs=%CSrcs% qcli\qcli.c
SET CSrcs=%CSrcs% qcli\qcli_util.c
SET CSrcs=%CSrcs% qcli\pal.c
SET CSrcs=%CSrcs% qcli\pal_som.c
SET CSrcs=%CSrcs% qcli\pal_mom.c
SET CSrcs=%CSrcs% hmi\hmi_demo.c
SET CSrcs=%CSrcs% hmi\hmi_demo_som.c
SET CSrcs=%CSrcs% ble\ble_demo.c
SET CSrcs=%CSrcs% coex\coex_demo.c

SET CSrcs=%CSrcs% export\DALConfig_devcfg.c
SET CSrcs=%CSrcs% export\DALConfig_fom.c
SET CSrcs=%CSrcs% export\devcfg_devcfg_data.c
SET CSrcs=%CSrcs% export\devcfg_fom_data.c
SET CSrcs=%CSrcs% export\UsrEDL.c

REM Include directories
SET Includes=-I"%RootDir%\include"
SET Includes=%Includes% -I"%RootDir%\include\qapi"
SET Includes=%Includes% -I"%RootDir%\include\bsp"

REM just for test
SET Includes=%Includes% -I"%SrcDir%\qcli"
SET Includes=%Includes% -I"%SrcDir%\hmi"
SET Includes=%Includes% -I"%SrcDir%\ble"
SET Includes=%Includes% -I"%SrcDir%\coex"

REM External objects and libraries
SET Libs=%Libs% "%LibDir%\core.lib"
SET Libs=%Libs% "%LibDir%\qurt.lib"
SET Libs=%Libs% "%LibDir%\quartzplatform.lib"
SET Libs=%Libs% "%LibDir%\WLAN.lib"

IF /I "%CFG_FEATURE_WLAN%" == "true" (
    SET Libs=!Libs! "%LibDir%\WLAN_QAPI.lib"
    SET Libs=!Libs! "%LibDir%\CUST_IPSTACK_INICHE.lib"
    SET Libs=!Libs! "%LibDir%\wlan_lib_common_xip.lib"
    SET Libs=!Libs! "%LibDir%\cust_wlan_lib.lib"
)

SET Libs=%Libs% "%LibDir%\WLAN_PROFILER.lib"
SET Libs=%Libs% "%LibDir%\net.lib"
SET Libs=%Libs% "%LibDir%\net_ram.lib"
SET Libs=%Libs% "%LibDir%\csr.lib"
SET Libs=%Libs% "%LibDir%\dhcpv6c.lib"
SET Libs=%Libs% "%LibDir%\sntpc.lib"
SET Libs=%Libs% "%LibDir%\dnssrvr.lib"
SET Libs=%Libs% "%LibDir%\sharkssl.lib"
SET Libs=%Libs% "%LibDir%\cryptolib.lib"
SET Libs=%Libs% "%LibDir%\httpsvr.lib"
SET Libs=%Libs% "%LibDir%\httpc.lib"
SET Libs=%Libs% "%LibDir%\mqttc.lib"
SET Libs=%Libs% "%LibDir%\vfs.lib"
SET Libs=%Libs% "%LibDir%\userpass.lib"
SET Libs=%Libs% "%LibDir%\i2s.lib"
SET Libs=%Libs% "%LibDir%\fwup.lib"
SET Libs=%Libs% "%LibDir%\fwup_engine.lib"
SET Libs=%Libs% "%LibDir%\qapi_ed25519.lib"
SET Libs=%Libs% "%LibDir%\qapi_securefs.lib"
SET Libs=%Libs% "%LibDir%\pka_port.lib"
SET Libs=%Libs% "%LibDir%\fs_helper.lib"
SET Libs=%Libs% "%LibDir%\zigbee.lib"
SET Libs=%Libs% "%LibDir%\quartz_zigbee.lib"
REM SET Libs=%Libs% "%LibDir%\thread.lib"
REM SET Libs=%Libs% "%LibDir%\qapi_thread.lib"
SET Libs=%Libs% "%LibDir%\BLUETOPIA_SERVICES.lib"
SET Libs=%Libs% "%LibDir%\BLUETOPIA_QAPI_SERVICES.lib"
SET Libs=%Libs% "%LibDir%\mdns.lib"
SET Libs=%Libs% "%LibDir%\dnssd.lib"
SET Libs=%Libs% "%LibDir%\otp_tlv.lib"
SET Libs=%Libs% "%LibDir%\base64.lib"
SET Libs=%Libs% "%LibDir%\nichestack.lib"
SET Libs=%Libs% "%LibDir%\quartzplatform_xip.lib"

REM Patch object files go here

IF /I "%CHIPSET_VERSION%" == "v1" (
   SET Libs=!Libs! "%LibDir%\pka.lib"
   SET Libs=!Libs! "%LibDir%\homekit.lib"
   SET Libs=!Libs! "%LibDir%\v1core.lib"
   SET Libs=!Libs! "%LibDir%\net_v1.lib"
   SET Libs=!Libs! "%LibDir%\OSAL_M4.lib"
   SET Libs=!Libs! "%LibDir%\i15p4_hmi.lib"

   REM Place all v1 patches here
   SET PatchObjs=!PatchObjs! "%LibDir%\patch.lib"

) ELSE (
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
)

REM Setup the build variables
SET Compiler=arm-none-eabi-gcc
SET ObjCopy=arm-none-eabi-objcopy
SET Archiver=arm-none-eabi-ar
SET Linker=arm-none-eabi-ld
SET ARM_OBJDUMP=arm-none-eabi-objdump

SET COpts=-c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -fno-exceptions -O2 -Wall
SET Defines=-D WLAN_DEBUG -D ENABLE_P2P_MODE

IF /I "%BOARD_VARIANT%" == "CDB" (SET Defines=!Defines! "-D CONFIG_CDB_PLATFORM")

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

SET CFlags=%COpts% %Defines% %Includes%

SET LDFlags=-eSBL_Entry -no-wchar-size-warning --no-warn-mismatch -R"%SymFile%" -R"%SymFileUnpatched%" -T"%LINKFILE%" -Map="%OutDir%\%Project%.map" -uqca_init -ugTsensBsp_devcfg_xml -ugAdcBsp_devcfg_xml -u quartz_ram_function_table -uwlan_secboot_auth -umom_indirection_table -usom_indirection_table -ufom_indirection_table -n

REM Clean the output directory. Note the delay is to give Windows an opportunity to remove the old directory tree
RMDIR /s/q "%OutDir%" >NUL 2>&1
TIMEOUT /t 1          >NUL 2>&1
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
REM CDB board uses "mm" as BOARD_VARIANT. CDB board and carrier board use different DevCfg XML files.
   python %RootDir%/build/tools/devcfg/propgen.py --XmlFile=%SrcDir%/export/DevCfg_master_devcfg_out_cdb.xml --DirName=%SrcDir%/export --ConfigFile=%SrcDir%/export/DALConfig_devcfg.c --DevcfgDataFile=%SrcDir%/export/devcfg_devcfg_data.c --ConfigType=%CHIPSET_VARIANT%_devcfg_xml
   python %RootDir%/build/tools/devcfg/propgen.py --XmlFile=%SrcDir%/export/DevCfg_master_fom_out_cdb.xml --DirName=%SrcDir%/export --ConfigFile=%SrcDir%/export/DALConfig_fom.c --DevcfgDataFile=%SrcDir%/export/devcfg_fom_data.c --ConfigType=%CHIPSET_VARIANT%_fom_xml
) ELSE (
   python %RootDir%/build/tools/devcfg/propgen.py --XmlFile=%SrcDir%/export/DevCfg_master_devcfg_out.xml --DirName=%SrcDir%/export --ConfigFile=%SrcDir%/export/DALConfig_devcfg.c --DevcfgDataFile=%SrcDir%/export/devcfg_devcfg_data.c --ConfigType=%CHIPSET_VARIANT%_devcfg_xml
   python %RootDir%/build/tools/devcfg/propgen.py --XmlFile=%SrcDir%/export/DevCfg_master_fom_out.xml --DirName=%SrcDir%/export --ConfigFile=%SrcDir%/export/DALConfig_fom.c --DevcfgDataFile=%SrcDir%/export/devcfg_fom_data.c --ConfigType=%CHIPSET_VARIANT%_fom_xml
)

REM Compile the source
FOR %%F IN (%CSrcs%) DO (
   ECHO Building %%F
   "%Compiler%" %CFlags% -Wall -Werror "-D __FILENAME__=\"%%~nF.c\"" -o"%ObjDir%\%%~nF.o" "%SrcDir%\%%F"
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
if %errorlevel% == 1 (
   echo Failed to create linker script file. Check dbg.Make for detail...
   goto EndOfFile
)

REM Link appropiate set of files
REM based on the chipset version
ECHO Linking...

IF /I "%CHIPSET_VERSION%" == "v1" (

   REM link the image
   "%Linker%" %LDFlags% --start-group @%LIBSFILE% --end-group -o"%OutDir%\%Project%.elf"
)  ELSE (

   REM link the image
   "%Linker%" %LDFlags% --start-group @%LIBSFILE%  --end-group -o"%OutDir%\%Project%_nocompact.elf"
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
ECHO Usage: build.bat [RTOS (FreeRTOS = f, ThreadX = t)] [chipset variant (4020, 4024, 4025)] [board variant (Carrier = c, DUT = d, CDB = cdb)] [chipset revision (1.2 = 1p2, 2.0 = 2p0)] [NVM file]
GOTO:EndOfFile

:EndOfFile
