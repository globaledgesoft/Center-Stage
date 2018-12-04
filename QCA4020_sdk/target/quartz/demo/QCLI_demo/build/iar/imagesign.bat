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

REM Setup the paths for the build
SET PROJECT=Quartz
SET RootDir=%~dp0\..\..\..\..\..
SET OutDir=%~dp0\output\Exe
SET ScriptDir=%RootDir%\build\scripts
SET SectoolsDir=%RootDir%\sectools

IF /I "%~1" == "" (
   SET CHIPSET_VARIANT=qca4020
) ELSE IF /I "%1" == "4020" (
   SET CHIPSET_VARIANT=qca4020
) ELSE IF /I "%1" == "4024" (
   SET CHIPSET_VARIANT=qca4024
) ELSE IF /I "%1" == "4025" (
   SET CHIPSET_VARIANT=qca4025
)

echo Signing Images for %CHIPSET_VARIANT%

python %SectoolsDir%\sectools.py secimage -i %OUTDIR%\%PROJECT%.elf -c 4020_secimage.xml -sa -g m4 -o .
python %SectoolsDir%\sectools.py secimage -i %RootDir%\bin\cortex-m0\threadx\ioe_ram_m0_threadx_ipt.mbn -c 4020_secimage.xml -sa -g m0 -o .
IF /I "%CHIPSET_VARIANT%" == "qca4020" (
    python %SectoolsDir%\sectools.py secimage -i %RootDir%\bin\wlan\wlan_fw_img.bin -c 4020_secimage.xml -sa -g kf -o .
)
