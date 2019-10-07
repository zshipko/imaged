SRC=src/util.c src/iter.c src/db.c
OBJ=$(SRC:.c=.o)
CFLAGS?=-Wall -Wextra
LDFLAGS?=
PIC?=-fPIC
DEST?=/usr/local

build: lib bin

.PHONY: bin
bin: $(OBJ)
	$(CC) -o imaged $(CFLAGS) $(OBJ) bin/imaged.c $(LDFLAGS)

lib: $(OBJ)
	ar rcs libimaged.a $(OBJ)

shared: $(OBJ)
	$(CC) $(CFLAGS) $(PIC) -shared -o libimaged.so $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) libimaged.a libimaged.so
	rm -rf test/imaged*

install:
	mkdir -p $(DEST)/lib $(DEST)/include
	install libimaged.a $(DEST)/lib
	install src/imaged.h $(DEST)/include

uninstall:
	rm -f $(DEST)/lib/libimaged.a $(DEST)/include/imaged.h

%.o: %.c
	$(CC) $(PIC) -Wall -O3 -c $*.c $(CFLAGS) -o $@
