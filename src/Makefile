SRC_FILES= $(wildcard *.c)
OBJ_FILES= $(SRC_FILES:.c=.o)

.PHONY: clean 

lib : $(OBJ_FILES)
	ar rcs libndes.a $(OBJ_FILES)

clean :
	\rm -f $(OBJ_FILES) libndes.a

.c.o :
	$(CC) $(CFLAGS) -I../$(INCL_DIR) $< -c

