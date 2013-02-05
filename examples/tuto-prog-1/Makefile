NDES_PATH=../..

export CC=gcc

export CFLAGS=-Wall -I$(NDES_PATH)/include #-DDEBUG_NDES
export LDFLAGS=-O3 -L$(NDES_PATH)/lib -lndes -lm


rr : rr.c
	$(CC) $(CFLAGS) rr.c -o rr $(LDFLAGS)


clean :
	\rm -f rr

