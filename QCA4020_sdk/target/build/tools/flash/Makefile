#================================================================
# Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
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

#
#
#  TOOL_DIR have to be set before run make
#
#  TOOL_DIR should be set to arm tools root path
#  for example
#  export TOOL_DIR=/opt/gcc-arm-none-eabi-6_2-2016q4
#  
#  this Makefile is verified on arm-gcc 6.2
#

RTOS=threadx

RootDir=../../..
SrcDir=.
OutDir=output
ObjDir=$(OutDir)/objs
LibDir=.

Compiler=$(TOOL_DIR)/bin/arm-none-eabi-gcc
Linker=$(TOOL_DIR)/bin/arm-none-eabi-ld
LINKFILE=qca402x_devprog.ld

OBJS=$(ObjDir)/flash_init_config.o
ELFPROGRAM=DEVICEPROGRAMMER_IMG_ARNTRI
ELF=$(OutDir)/../DEVICEPROGRAMMER_IMG_ARNTRI.elf
COpts=-c -g -mcpu=cortex-m4 -mthumb -fno-short-enums -fno-exceptions -ffunction-sections
Defines=-D WLAN_DEBUG -D ENABLE_P2P_MODE

Includes=-I$(RootDir)/include -I$(RootDir)/include/qapi -I$(RootDir)/include/bsp



CFlags=$(COpts) $(Defines) $(Includes) -D_WANT_IO_C99_FORMATS
LDFlags=--thumb-entry=SBL_Entry -no-wchar-size-warning --no-warn-mismatch -T$(LINKFILE) -Map=$(OutDir)/$(ELFPROGRAM).map -ujstorage_stack -L$(TOOL_DIR)/arm-none-eabi/lib/thumb/v7e-m -L$(TOOL_DIR)/lib/gcc/arm-none-eabi/6.2.1/thumb/v7e-m --gc-sections


DevLibs=-lc -lgcc -lnosys $(LibDir)/devprg.lib $(LibDir)/programmer.lib

JTAGELFPROGRAM=JTAGPROGRAMMER_IMG_ARNTRI
JTAGELF=$(OutDir)/../$(JTAGELFPROGRAM).elf
JTAGLibs=-lc -lgcc -lnosys $(LibDir)/programmer.lib
JTAGLINKFILE=qca402x_jtagprog.ld
JTAGLDFlags=-ejstorage_init -no-wchar-size-warning --no-warn-mismatch -T$(JTAGLINKFILE) -Map=$(OutDir)/$(JTAGELFPROGRAM).map -ujstorage_stack -L$(TOOL_DIR)/arm-none-eabi/lib/thumb/v7e-m -L$(TOOL_DIR)/lib/gcc/arm-none-eabi/6.2.1/thumb/v7e-m --gc-sections


all:	devprg

jtagprg:	$(JTAGELF)

$(JTAGELF):	$(OBJS)
	@echo === Linking  $(JTAGELF)
	@$(Linker) $(JTAGLDFlags) --start-group $(OBJS) $(JTAGLibs) --end-group -o $(JTAGELF)

devprg:	$(ELF)

$(ELF):	$(OBJS)
	@echo === Linking  $(ELF)
	@$(Linker) $(LDFlags) --start-group $(OBJS) $(DevLibs) --end-group -o $(ELF)

$(ObjDir)/flash_init_config.o:	$(SrcDir)/flash_init_config.c
	@echo === Compiling  $<
	@if [ ! -d $(OutDir) ]; then mkdir $(OutDir); fi;
	@if [ ! -d $(ObjDir) ]; then mkdir $(ObjDir); fi;
	@$(Compiler) $(CFlags) $< -o $@ 
	
clean:
	@if [ -a $(JTAGELF) ]; then rm $(JTAGELF); fi;
	@if [ -a $(ELF) ]; then rm $(ELF); fi;
	
	@rm -rf output
