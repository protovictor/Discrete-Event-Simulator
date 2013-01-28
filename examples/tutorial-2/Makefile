NDES_PATH=../..

export CC=gcc

export CFLAGS=-Wall -I$(NDES_PATH)/include
export LDFLAGS=-O3 -L$(NDES_PATH)/lib -lndes -lm


mm1 : mm1.c
	$(CC) $(CFLAGS) mm1.c -o mm1 $(LDFLAGS)


clean :
	\rm -f mm1

