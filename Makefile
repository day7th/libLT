##*****************************************************************************
## Title      : Lib LT
## Project    : -
##*****************************************************************************
## File       : Makefile
## Author     : Pasquale Cataldi, Andrea Tomatis, Mario Pastorelli
## Company    : 
## Last update: 2010/12/09
## Platform   : Ansi C
##*****************************************************************************
## Description: This file allows to compile the LT library
##*****************************************************************************
## Revisions  :
## Date        Version  Author  Description
## 2008/07/17  0.1      PC,AT   First release
## 2010/07/30  0.2      PC      Use wildcard for the position of the source files
## 2010/12/09  0.3      MP      Add option to build a shared library
##*****************************************************************************

#CFLAGS=-DDEBUGprintRealeasedSymbol -msse2 -mmmx -Wall -funroll-loops # In python crash
CFLAGS= -g -o3 -funroll-loops -Wall -Wstrict-aliasing -msse2 -mmmx -fPIC
#CFLAGS=-O3 -funroll-loops -msse2 -mmmx

SRCS= $(wildcard */*.c)

OBJS=$(SRCS:.c=.o)

LIBNAME=libLT.a
SLIBNAME=_libLT.so

all: 
	make static
	make shared
	make examples

$(LIBNAME): $(OBJS) 
	ar rc $@ $(OBJS)
	ar r $(LIBNAME) $(OBJS)
	ranlib $@
	
.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean: 
	rm -f $(OBJS)
	rm -f $(LIBNAME)	
	rm -f $(SWIGRES)
	rm -f $(SLIBNAME)
	rm -f libLT.pyc
	cd examples && make clean && cd ..

static: $(SRCS) $(LIBNAME)
	rm -f -r $(OBJS)

shared: $(OBJS)
	ld -shared $(OBJS) -o $(SLIBNAME)
	rm -f -r $(OBJS)

redo:
	make clean
	make all
	
examples:
	cd examples && make tests && cd ..
	
.PHONY: all examples

