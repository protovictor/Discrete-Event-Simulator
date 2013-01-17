SRC_FILES= $(wildcard *.c)
OBJ_FILES= $(SRC_FILES:.c=.o)

EXAMPLES = debits example-1 example-2 example-3

.PHONY: clean 

all : $(EXAMPLES)

debits : debits.o ../$(SRC_DIR)/libndes.a
	$(CC) debits.o -o debits -lm -L../$(SRC_DIR) -lndes

example-1 : example-1.o ../$(SRC_DIR)/libndes.a
	$(CC) example-1.o -o example-1 -lm -L../$(SRC_DIR) -lndes

example-2 : example-2.o ../$(SRC_DIR)/libndes.a
	$(CC) example-2.o -o example-2 -lm -L../$(SRC_DIR) -lndes

example-3 : example-3.o ../$(SRC_DIR)/libndes.a
	$(CC) example-3.o -o example-3 -lm -L../$(SRC_DIR) -lndes

clean :
	\rm -f $(OBJ_FILES) $(EXAMPLES)

.c.o :
	$(CC) $(CFLAGS) -I../$(INCL_DIR) $< -c

