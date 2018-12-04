@ECHO OFF

REM Note to use this debug script, the JlinkGDBServer and arm-non-eabi-gdb executables
REM needs to be in the system path. If either of these isn't in the system path,
REM JLINK_PATH and/or GCC_BIN_PATH must be defined with their location.

IF /I "%CHIPSET_VERSION%" == "v1" (SET CHIPSET=v1) ELSE (SET CHIPSET=v2)

IF /I "%CHIPSET%" == "v1" (
    @ECHO ****************************************************************************
    @ECHO    Starting M4 Debug session for Quartz QCLI Application for v1 Chipset
    @ECHO                  To debug v2, set CHIPSET_VERSION=v2                  
    @ECHO *****************************************************************************
) ELSE (
    @ECHO ****************************************************************************
    @ECHO    Starting M4 Debug session for Quartz QCLI Application for v2 Chipset
    @ECHO                  To debug v1, set CHIPSET_VERSION=v1                  
    @ECHO *****************************************************************************
)

REM Get the paths from the environment variables if they are set.
SET SERVER_PATH=
SET CLIENT_PATH=
IF NOT "%JLINK_PATH%"   == "" SET SERVER_PATH=%JLINK_PATH:"=%\
if NOT "%GCC_BIN_PATH%" == "" SET CLIENT_PATH=%GCC_BIN_PATH:"=%\

REM Set the options for the Jlink GDB server
SET JLinkOptions=-scriptfile "%~dp0%Quartz.JLinkScript"
SET JLinkOptions=%JLinkOptions% -select USB
SET JLinkOptions=%JLinkOptions% -device Cortex-M4
SET JLinkOptions=%JLinkOptions% -endian little
SET JLinkOptions=%JLinkOptions% -if JTAG
SET JLinkOptions=%JLinkOptions% -speed 1000
SET JLinkOptions=%JLinkOptions% -noir
SET JLinkOptions=%JLinkOptions% -singlerun
SET JLinkOptions=%JLinkOptions% -port 2331

REM Start the GDB Server.
START "JLinkGDBServer" /MIN "%SERVER_PATH%JLinkGDBServerCL.exe" %JLinkOptions%
"arm-none-eabi-gdb.exe" -x "%CHIPSET%\reset.gdbinit"
