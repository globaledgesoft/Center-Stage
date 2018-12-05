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

REM SETLOCAL EnableDelayedExpansion

REM Default CHIPSET_VERSION is v2
if /I "%CHIPSET_VERSION%" == ""  (SET CHIPSET_VERSION=v2)

REM Determine the RTOS to build. Default to threadx.
if /I "%~1" == "" (
   SET RTOS=threadx
) ELSE IF /I "%~1" == "t" (
   SET RTOS=threadx
) ELSE IF /I "%~1" == "f" (
   SET RTOS=freertos
) else (
   echo Invalid RTOS: %1
   GOTO:Usage
)



REM Validate the chipset variant. Default: 4020
if /I "%~2"=="" (
   SET CHIPSET_VARIANT=qca4020
) ELSE IF /I "%~2" == "4020" (
   SET CHIPSET_VARIANT=qca4020
) ELSE IF /I "%~2" == "4024" (
   SET CHIPSET_VARIANT=qca4024
) ELSE IF /I "%~2" == "4025" (
   SET CHIPSET_VARIANT=qca4025
) else (
   echo Invalid chipset variant: %2
   GOTO:Usage
)


echo ****************************************************************************
echo                 Building Programmer for Quartz %CHIPSET_VERSION% Chipset
echo                 RTOS             %RTOS%
echo                 CHIPSET VARIANT  %CHIPSET_VARIANT%
echo  ***************************************************************************

REM Setup the paths for the build
SET Project=Quartz
SET RootDir=..\..\..
SET SrcDir=..\..\..\quartz\platform\export
SET XmlSrcDir=../../../quartz/demo/QCLI_demo/src
SET NvmDir=%RootDir%/quartz/nvm
SET OutDir=output
SET ObjDir=%OutDir%\objs
SET SectoolsDir=$RootDir/sectools
SET SECBOOT=false
SET LibDir=.
SET SymFileUnpatched="IOE_ROM_IPT_IMG_ARNNRI_orig_fcns_gcc.sym"

SET SymFile="JTAGPROGRAMMER_IMG_ARNTRI.sym"

SET ScriptDir=%RootDir%\build\scripts
SET LinkerScriptDir=%RootDir%\build\scripts\linkerScripts
SET LINKFILE=qca402x_devprog.ld
SET LIBSFILE=%OutDir%\LinkerLibs.txt
SET ELFPROGRAM=%OutDir%/DEVICEPROGRAMMER_IMG_ARNTRI
SET ThirdpartyDir=..\..\..\..

SET Libs=%LibDir%\devprg.lib
SET Libs=%Libs% %LibDir%\programmer.lib


REM Sources to compile
SET CSrcs=%SrcDir%\flash_init_config.c

REM Include directories
SET Includes=-I%RootDir%\include
SET Includes=%Includes% -I%RootDir%\include\qapi
SET Includes=%Includes% -I%RootDir%\include\bsp


REM Setup the build variables
SET TOOL_DIR="C:\Program Files (x86)\GNU Tools ARM Embedded\6.2 2016q4"
SET Compiler=%TOOL_DIR%\bin\arm-none-eabi-gcc
SET ObjCopy=%TOOL_DIR%\bin\arm-none-eabi-objcopy
SET Archiver=%TOOL_DIR%\bin\arm-none-eabi-ar
SET Linker=%TOOL_DIR%\bin\arm-none-eabi-ld
SET ARM_OBJDUMP=%TOOL_DIR%\bin\arm-none-eabi-objdump

SET COpts=-c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -fno-exceptions -ffunction-sections
SET Defines=-D WLAN_DEBUG -D ENABLE_P2P_MODE

if "%CHIPSET_VERSION%" == "v1" (
   SET Defines=%Defines% "-D V1"
) else (
   SET Defines=%Defines% -D V2
)

SET CFlags=%COpts% %Defines% %Includes% -D_WANT_IO_C99_FORMATS


SET LDFlags=--thumb-entry=SBL_Entry -no-wchar-size-warning --no-warn-mismatch -T%LINKFILE% -Map=%ELFPROGRAM%.map -lc -n -ujstorage_stack -L%TOOL_DIR%\arm-none-eabi\lib\thumb\v7e-m -L%TOOL_DIR%\lib\gcc\arm-none-eabi\6.2.1\thumb\v7e-m --gc-sections

REM Compile the source

if NOT exist %OutDir% (
   mkdir %OutDir%
   echo Done
)


if NOT exist %ObjDir% (
   mkdir %ObjDir%
)


for %%i in (%CSrcs%) do (
   SET fname=%%~ni
   
   echo === Compiling %%~ni.c
   @%Compiler% %CFlags% %%i -o %ObjDir%\%%~ni.o
   SET Libs=%Libs% %ObjDir%\%%~ni.o
)


SET Libs=%Libs% -lc -lgcc -lnosys

echo Link the image

@%Linker% %LDFlags% --start-group %Libs% --end-group -o %OutDir%/../DEVICEPROGRAMMER_IMG_ARNTRI.elf 
echo Done
 
:Usage

