WRK Systable Generator
======================
Copyright (c) 2009. Alexander Schmidt. All rights reserved.

Project Contents
----------------

This project contains the following files:

- makefile: Builds the tool.
- makesystable.cpp: Implementation of the tool.
- makesystable.h: Header file for the implementation.
- readme.txt: The file you are reading.

How to build the tool
---------------------

1. Open a Visual Studio command prompt.
2. Navigate to this directory.
3. Type "nmake". This will compile the tool.
4. The executable is called mksystbl.exe. Please follow the installation 
   instructions on how to use this tool with the WRK build process.
   
   
Installation
------------

The WRK Systable Generator is designed to work as part of the WRK build process.
To incorporate it in the build process, we recommend copying the mksystbl.exe 
tool into the tools\x86 and tools\amd64 directories in the WRK source tree. 

Add the following nmake target to the base\ntos\makefile file:

$(systable): $(services)
	@echo Generating system services table ...
	@cd $(MAKEDIR)\ke\$(targ)
		@mksystbl.exe
	@cd $(MAKEDIR)
	
Additonally define the following two variables:

services = $(MAKEDIR)\ke\$(targ)\services.tab
systable = $(MAKEDIR)\ke\$(targ)\systable.asm

The target above triggers the generation of the systable.asm file iff the
service.tab file has been modified (is newer than systable.asm). 

References
----------

If you need additional help, please refer to 
http://www.dcl.hpi.uni-potsdam.de/research/WRK/?p=78 or contact us directly:
wrk@hpi.uni-potsdam.de.
