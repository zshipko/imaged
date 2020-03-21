VERSION=0.1
SRC=src/util.c src/iter.c src/db.c src/image.c src/pixel.c src/color.c src/io.c src/aces.c src/halide.c src/threads.c
OBJ=$(SRC:.c=.o)

RAW=1
HALIDE=
PKGS= babl ezimage
FLAGS=
HAS_RAW?=$(shell pkg-config --cflags --libs libraw)

ifeq ($(RAW),1)
ifneq ($(HAS_RAW),)
	PKGS += libraw
else
	FLAGS += -DIMAGED_NO_RAW
endif
else
	FLAGS += -DIMAGED_NO_RAW
endif

ifeq ($(HALIDE),1)
	FLAGS += -DIMAGED_HALIDE
endif

CFLAGS?=-Wall -Wextra `pkg-config --cflags $(PKGS)` $(FLAGS)
LDFLAGS?=-lpthread `pkg-config --libs $(PKGS)`
PIC?=-fPIC
DEST?=/usr/local

$(shell echo $(CFLAGS) > .cflags)
$(shell echo $(LDFALGS) > .ldflags)

ifeq ($(shell uname -s),Linux)
	TEST_LDFLAGS=-lrt -lsubunit
endif

ifeq ($(shell uname -s),Darwin)
	SOEXT=dylib
else
	SOEXT?=so
endif

build: lib shared bin
fresh: src/imaged.h .cflags .ldflags build

debug:
	$(MAKE) CFLAGS="$(CFLAGS) -Rpass-missed=loop-vectorize"

.PHONY: bin
bin: src/imaged.h .cflags $(OBJ)
	$(CC) -o imaged $(CFLAGS) $(OBJ) bin/imaged.c $(LDFLAGS)

lib: $(OBJ)
	$(AR) rcs libimaged.a $(OBJ)
	@printf "Name: imaged\nDescription: Imaging storage library\nVersion: $(VERSION)\nLibs: -L$(DEST)/lib -limaged $(LDFLAGS)\nCflags: -I/usr/local/include -I$(DEST)/include\nRequires: $(PKGS)\n" > imaged.pc

shared: $(OBJ)
	$(CC) $(CFLAGS) $(PIC) -shared -o libimaged.$(SOEXT) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) libimaged.a libimaged.$(SOEXT) .cflags .ldflags imaged.pc

install-lib:
	mkdir -p $(DEST)/lib/pkgconfig $(DEST)/include
	install libimaged.a $(DEST)/lib
	install libimaged.$(SOEXT) $(DEST)/lib
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

%.o: %.c
	$(CC) $(CFLAGS) $(PIC) -Wall -O3 -c $*.c  -o $@
