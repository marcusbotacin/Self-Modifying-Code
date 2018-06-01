# Makefile for charater devices
obj-m += smcmod.o 

KDIR=/lib/modules/$(shell uname -r)/build
SOURCE="smclient.c"
BIN="smclient"

all: smclient.c
	gcc -o $(BIN) $(SOURCE)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -rvf *~
