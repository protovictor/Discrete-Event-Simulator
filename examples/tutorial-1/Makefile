NDES_PATH=../..

export CC=gcc

export CFLAGS=-Wall -I$(NDES_PATH)/include
export LDFLAGS=-O3 -L$(NDES_PATH)/lib


mm1 : mm1.c
	$(CC) $(CFLAGS) $(LDFLAGS) mm1.c -o mm1 -lm -lndes


clean :
	\rm -f mm1

