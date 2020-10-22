#
# Makefile for dplane's utility programs
#



#
# PF_RING
#
PFRINGDIR  = /usr/local/lib
LIBPFRING  = ${PFRINGDIR}/libpfring.a


#
# DNA Support
DNA_DEFINE = #-DENABLE_DNA_SUPPORT

#
# PF_RING aware libpcap
#
O_FLAG     = #-O2
EXTRA_LIBS = #-ldag
PCAPDIR    = /usr/local/lib
LIBPCAP    = ${PCAPDIR}/libpcap.a  ${EXTRA_LIBS}

#
# Search directories
#
PFRING_KERNEL=../../kernel
INCLUDE    = -I${PFRINGDIR} -I${PCAPDIR}

#
# C compiler and flags
#
#
# CROSS_COMPILE=arm-mv5sft-linux-gnueabi-
#
CC         = ${CROSS_COMPILE}gcc #--platform=native
CFLAGS     = -g ${O_FLAG} -Wall -Wno-parentheses ${INCLUDE} ${DNA_DEFINE} #-O
# LDFLAGS  =

#
# User and System libraries
#
LIBS       = ${LIBPFRING} ${LIBPCAP} -lpthread -lrt

# How to make an object file
%.o: %.c
#	@echo "=*= making object $@ =*="
	${CC} ${CFLAGS} -c $< -o $@

#
# Main targets
#
PFPROGS   = dboss

TARGETS   = ${PFPROGS} dqa gather dpstat dspeed scatspeed

all: dboss dqa gather dpstat dspeed scatspeed

dboss: dboss.o miscellany.o
	${CC} dboss.o ../d-plane/miscellany.o ${LIBS} -o $@

dqa: dqa.o
	${CC} dqa.o ${LIBS} -o $@

gather: gather.o
	${CC} gather.o ${LIBS} -o $@

dpstat: dpstat.o
	${CC} dpstat.o ${LIBS} -o $@

dspeed: dspeed.o
	${CC} dspeed.o ${LIBS} -o $@

scatspeed: scatspeed.o scat_write.o miscellany.o writer.o
	${CC} scatspeed.o ../d-plane/scat_write.o \
        ../d-plane/miscellany.o ../d-plane/writer.o ${LIBS} -o $@

clean:
	@rm -f ${TARGETS} *.o *~

install:
	chgrp mark6 dboss dqa gather dpstat dspeed scatspeed
	chmod 4755 dboss dqa gather dpstat dspeed scatspeed
	mv dboss dqa gather dpstat dspeed scatspeed /usr/local/bin

dboss.o dpstat.o miscellany.o dqa.o gather.o scatspeed.o \
scat_write.o writer.o: ../d-plane/dplane.h ../d-plane/messages.h \
../d-plane/vdif.h ../d-plane/mk5b.h

