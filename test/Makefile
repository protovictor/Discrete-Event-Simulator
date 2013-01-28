SRC_FILES= $(wildcard *.c)
OBJ_FILES= $(SRC_FILES:.c=.o)

TESTS = generators-0 generators-1 \
	generators-3 generators-4 \
#	file-pdu file-pdu-2 file-pdu-3 \
#	debits \
#	probes-1 probes-2 probes-3 probes-4 \
#	muxfcfs-1 \
#	intconf


.PHONY: clean 

all : $(TESTS)

tests :  $(TESTS)
	./run-tests.sh $(TESTS)

generators-0 : generators-0.o ../$(SRC_DIR)/libndes.a
	$(CC) generators-0.o -o generators-0 -lm -L../$(SRC_DIR) -lndes

generators-1 : generators-1.o ../$(SRC_DIR)/libndes.a
	$(CC) generators-1.o -o generators-1 -lm -L../$(SRC_DIR) -lndes

generators-3 : generators-3.o ../$(SRC_DIR)/libndes.a
	$(CC) generators-3.o -o generators-3 -lm -L../$(SRC_DIR) -lndes

generators-4 : generators-4.o ../$(SRC_DIR)/libndes.a
	$(CC) generators-4.o -o generators-4 -lm -L../$(SRC_DIR) -lndes

intconf : intconf.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) intconf.o -o intconf -lm -L../$(SRC_DIR) -lndes

debits : debits.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) debits.o -o debits -lm -L../$(SRC_DIR) -lndes

muxfcfs-1 : muxfcfs-1.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) muxfcfs-1.o -o muxfcfs-1 -lm -L../$(SRC_DIR) -lndes

probes-1 : probes-1.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) probes-1.o -o probes-1 -lm -L../$(SRC_DIR) -lndes

probes-2 : probes-2.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) probes-2.o -o probes-2 -lm -L../$(SRC_DIR) -lndes

probes-3 : probes-3.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) probes-3.o -o probes-3 -lm -L../$(SRC_DIR) -lndes

probes-4 : probes-4.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) probes-4.o -o probes-4 -lm -L../$(SRC_DIR) -lndes

file-pdu : file-pdu.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) file-pdu.o -o file-pdu -lm -L../$(SRC_DIR) -lndes

file-pdu-2 : file-pdu-2.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) file-pdu-2.o -o file-pdu-2 -lm -L../$(SRC_DIR) -lndes

file-pdu-3 : file-pdu-3.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) file-pdu-3.o -o file-pdu-3 -lm -L../$(SRC_DIR) -lndes

src-exp : src-exp.o ../$(SRC_DIR)/libndes.a
	$(CC) $(LDFLAGS) src-exp.o -o src-exp -lm -L../$(SRC_DIR) -lndes

clean :
	\rm -f $(OBJ_FILES) $(TESTS)

.c.o :
	$(CC) $(CFLAGS) -I../$(INCL_DIR) $< -c

