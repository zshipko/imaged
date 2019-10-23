VERSION=0.1
SRC=src/util.c src/iter.c src/db.c src/image.c src/pixel.c src/color.c src/io.c
OBJ=$(SRC:.c=.o)
CFLAGS?=-Wall -Wextra `pkg-config --cflags babl ezimage`
LDFLAGS?=-lpthread `pkg-config --libs babl ezimage`
PIC?=-fPIC
DEST?=/usr/local

$(shell echo $(CFLAGS) > .cflags)
$(shell echo $(LDFALGS) > .ldflags)

build: lib bin
fresh: src/imaged.h .cflags .ldflags build

.PHONY: bin
bin: src/imaged.h .cflags $(OBJ)
	$(CC) -o imaged $(CFLAGS) $(OBJ) bin/imaged.c $(LDFLAGS)

lib: $(OBJ)
	$(AR) rcs libimaged.a $(OBJ)
	@printf "Name: imaged\nDescription: Imaging storage library\nVersion: $(VERSION)\nLibs: -L$(DEST)/lib -limaged $(LDFLAGS)\nCflags: -I/usr/local/include -I$(DEST)/include\nRequires: babl\n" > imaged.pc

shared: $(OBJ)
	$(CC) $(CFLAGS) $(PIC) -shared -o libimaged.so $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) libimaged.a libimaged.so .cflags .ldflags imaged.pc

install-lib:
	mkdir -p $(DEST)/lib/pkgconfig $(DEST)/include
	install libimaged.a $(DEST)/lib
	install src/imaged.h $(DEST)/include
	install imaged.pc $(DEST)/lib/pkgconfig

install: install-lib
	mkdir -p $(DEST)/bin
	install imaged $(DEST)/bin

uninstall:
	rm -f $(DEST)/lib/libimaged.a $(DEST)/include/imaged.h $(DEST)/bin/imaged $(DEST)/lib/pkgconfig/imaged.pc

.PHONY: rust
rust:
	cd rust && $(MAKE)
	cargo build

.PHONY: go
go:
	cd go && go build && cd imaged-server && go build

.PHONY: test
test: lib
	@$(CC) $(CFLAGS) -o test/test test/test.c libimaged.a -lcheck -lm -lsubunit -lrt $(LDFLAGS)
	@./test/test

%.o: %.c base
	$(CC) $(PIC) -Wall -O3 -c $*.c $(CFLAGS) -o $@
