NDES_PATH=../..

export CC=gcc

export CFLAGS=-Wall -I$(NDES_PATH)/include
export LDFLAGS=-O3 -L$(NDES_PATH)/lib -lndes -lm


mm1 : mm1.c
	$(CC) $(CFLAGS) mm1.c -o mm1 $(LDFLAGS)

ex2: ex2byme.c
	$(CC) $(CFLAGS) ex2byme.c -o ex2 $(LDFLAGS)

hs: httpmodel.c
	$(CC) $(CFLAGS) httpmodel.c -o hs $(LDFLAGS)

clean :
	\rm -f mm1

