VERSION=0.1
SRC=src/util.c src/iter.c src/db.c src/image.c src/pixel.c src/color.c src/io.c
OBJ=$(SRC:.c=.o)

RAW?=1
PKGS?= babl ezimage

ifeq ($(RAW),1)
	PKGS += libraw
endif

CFLAGS?=-Wall -Wextra `pkg-config --cflags $(PKGS)`
LDFLAGS?=-lpthread `pkg-config --libs $(PKGS)`
PIC?=-fPIC
DEST?=/usr/local

ifneq ($(RAW),1)
	CFLAGS += -DIMAGED_NO_RAW
endif

$(shell echo $(CFLAGS) > .cflags)
$(shell echo $(LDFALGS) > .ldflags)

ifeq ($(shell uname -s),Linux)
	TEST_LDFLAGS=-lrt -lsubunit
endif

build: lib bin
fresh: src/imaged.h .cflags .ldflags build

.PHONY: bin
bin: src/imaged.h .cflags $(OBJ)
	$(CC) -o imaged $(CFLAGS) $(OBJ) bin/imaged.c $(LDFLAGS)

lib: $(OBJ)
	$(AR) rcs libimaged.a $(OBJ)
	@printf "Name: imaged\nDescription: Imaging storage library\nVersion: $(VERSION)\nLibs: -L$(DEST)/lib -limaged $(LDFLAGS)\nCflags: -I/usr/local/include -I$(DEST)/include\nRequires: $(PKGS)\n" > imaged.pc

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
	cp go/imaged-server/imaged-server .

.PHONY: test
test: lib
	@$(CC) -g $(CFLAGS) -o test/test test/test.c libimaged.a -lcheck -lm $(LDFLAGS) $(TEST_LDFLAGS)
	@./test/test

%.o: %.c base
	$(CC) $(PIC) -Wall -O3 -c $*.c $(CFLAGS) -o $@
