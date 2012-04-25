##*****************************************************************************
## Title      : SW LT example
## Project    : example for the libLT
##*****************************************************************************
## File       : Makefile
## Author     : Pasquale Cataldi
## Company    :
## Last update: 2010/11/26
## Platform   : Ansi C
##*****************************************************************************
## Description: This file allows to compile a SW LT code
##*****************************************************************************
## Revisions  :
## Date        Version  Author  Description
## 2010/12/08  0.1      PC   First release
##*****************************************************************************

MATHLIB=m

LIBNAME=LT
LIBPATH=../../

CFLAGS=-I$(LIBPATH) -o0 -g -L$(LIBPATH) -funroll-loops -Wall -mmmx -msse2

SRCS=explicitSerializedFLY.c
OBJS=$(SRCS:.c=.o)
EXE=explicitSerializedFLY

all: $(SRCS) $(EXE)
	rm -f $(OBJS)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -l$(LIBNAME) -l$(MATHLIB) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(EXE)

redo:
	make clean
	make all

