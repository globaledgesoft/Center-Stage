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
SET Project=Quartz
SET RootDir=%~dp0\..\..\..\..\..
SET LibDir=%RootDir%\lib\cortex-m4IPT\%Platform%
SET LinkerScriptDir=%RootDir%\build\scripts\linkerScripts
SET LINKFILE_T="linkerCfg\v2\threadx\%Project%.icf"
SET LINKFILE_F="linkerCfg\v2\freertos\%Project%.icf"
SET LINKERLIST="LinkerLibs.txt"
SET QUARTZ_IAR_EWP="Quartz_4020_v2.ewp"


REM -------------Generate link icf file for Freertos------------------------
echo Generating linker file for freertos
SET Platform=freertos

IF EXIST %LINKFILE_F% del %LINKFILE_F%
IF EXIST %LINKERLIST% del %LINKERLIST%
IF EXIST app.placement del app.placement

REM Update application PlacementFile
python %LinkerScriptDir%\CreateAppPlacementFile.py %RootDir%\bin\cortex-m4\%Platform%\sys.placement %RootDir%\bin\cortex-m4\%Platform%\cust.placement app.config app.placement 2>dbg.CreateAppF
if %errorlevel% == 1 (
echo Failed to update application placement file. Check dbg.CreateAppF for detail... 
goto EndOfFile
) 
REM Update application list file
python %LinkerScriptDir%\CreateIarLinkLibsFile.py %QUARTZ_IAR_EWP% %Platform% %LINKERLIST% 2>dbg.CreateListF
if %errorlevel% == 1 (
echo Failed to update application list file. Check dbg.CreateListF for detail... 
goto EndOfFile
) 
REM Create a linker script
python %LinkerScriptDir%\MakeLinkerScript.py %RootDir%\bin\cortex-m4\%Platform%\DefaultTemplateLinkerScript.icf app.placement %LINKERLIST% iar app.config > %LINKFILE_F% 2>dbg.MakeF
if %errorlevel% == 1 (
echo Failed to create linker script file. Check dbg.MakeF for detail... 
goto EndOfFile
) 

REM -------------Generate link icf file for ThreadX------------------------
echo Generating linker file for ThreadX
SET Platform=threadx

IF EXIST %LINKFILE_T% del %LINKFILE_T%
IF EXIST %LINKERLIST% del %LINKERLIST%
IF EXIST app.placement del app.placement

REM Update application PlacementFile
python %LinkerScriptDir%\CreateAppPlacementFile.py %RootDir%\bin\cortex-m4\%Platform%\sys.placement %RootDir%\bin\cortex-m4\%Platform%\cust.placement app.config app.placement 2>dbg.CreateAppT
if %errorlevel% == 1 (
echo Failed to update application placement file. Check dbg.CreateAppT for detail... 
goto EndOfFile
) 
REM Update application list file
python %LinkerScriptDir%\CreateIarLinkLibsFile.py %QUARTZ_IAR_EWP% %Platform% %LINKERLIST% 2>dbg.CreateListT
if %errorlevel% == 1 (
echo Failed to update application list file. Check dbg.CreateListT for detail... 
goto EndOfFile
) 
REM Create a linker script
python %LinkerScriptDir%\MakeLinkerScript.py %RootDir%\bin\cortex-m4\%Platform%\DefaultTemplateLinkerScript.icf app.placement %LINKERLIST% iar app.config > %LINKFILE_T% 2>dbg.MakeT
if %errorlevel% == 1 (
echo Failed to create linker script file. Check dbg.MakeT for detail... 
goto EndOfFile
) 

echo Done.
GOTO:EndOfFile

:EndOfFile

