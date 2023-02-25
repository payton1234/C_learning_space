#
# File          : Makefile
# Description   : Build file for CSE473 project 1


# Environment Setup
LIBDIRS=-L. 
INCLUDES=-I.
CC=gcc 
CFLAGS=-c $(INCLUDES) -g -Wall
LINK=gcc -g
LDFLAGS=$(LIBDIRS)
AR=ar rc
RANLIB=ranlib

# Suffix rules
.c.o :
	${CC} ${CFLAGS} $< -o $@

#
# Setup builds

PT-TARGETS=cse473-p1 cse473-ipc cse473-mypipe
CSE473LIB=cse473-p1-mypipe
CSE473LIBOBJS= cse473-p1-mypipe.o

# proj lib
LIBS=-l$(CSE473LIB)

#
# Project Protections

p1 : $(PT-TARGETS)

cse473-p1 : cse473-p1.o 
	$(LINK) $(LDFLAGS) cse473-p1.o -o $@

cse473-ipc : cse473-p1-ipc.o  
	$(LINK) $(LDFLAGS) cse473-p1-ipc.o -o $@

cse473-mypipe : cse473-p1-shm.o lib$(CSE473LIB).a
	$(LINK) $(LDFLAGS) cse473-p1-shm.o $(LIBS) -o $@

lib$(CSE473LIB).a : $(CSE473LIBOBJS)
	$(AR) $@ $(CSE473LIBOBJS)
	$(RANLIB) $@

clean:
	rm -f *.o *~ $(TARGETS) $(LIBOBJS) lib$(CSE473LIB).a 

BASENAME=p1
tar: 
	tar cvfz $(BASENAME).tgz -C ..\
	    $(BASENAME)/Makefile \
	    $(BASENAME)/cse473-p1.c \
	    $(BASENAME)/cse473-p1.h \
	    $(BASENAME)/cse473-p1-ipc.c \
	    $(BASENAME)/cse473-p1-shm.c \
	    $(BASENAME)/cse473-p1-mypipe.c \
	    $(BASENAME)/cse473-p1-mypipe.h 
