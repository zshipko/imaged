VERSION=0.1
SRC=src/util.c src/iter.c src/db.c src/image.c src/pixel.c
OBJ=$(SRC:.c=.o)
CFLAGS?=-Wall -Wextra
LDFLAGS?=-lpthread
PIC?=-fPIC
DEST?=/usr/local
INTRIN?=yes
BABL?=yes

ifeq (yes,$(INTRIN))
HAS_SSE=$(shell $(CC) -mavx -dM -E - < /dev/null | egrep "SSE" | sort | grep '__SSE__ 1')
ifneq (,$(HAS_SSE))
	CFLAGS+= -msse
endif # HAS_SSE
endif # INTRIN


HAS_BABL=$(shell pkg-config --cflags babl)
ifeq (yes,$(BABL))
ifneq (,$(HAS_BABL))
	CFLAGS+= -DIMAGED_BABL `pkg-config --cflags babl`
	LDFLAGS+= `pkg-config --libs babl`
endif # HAS_BABL
endif #BABL

$(shell echo $(CFLAGS) > .cflags)
$(shell echo $(LDFALGS) > .ldflags)

build: lib bin
fresh: src/imaged.h .cflags .ldflags build

.PHONY: bin
bin: src/imaged.h .cflags $(OBJ)
	$(CC) -o imaged $(CFLAGS) $(OBJ) bin/imaged.c $(LDFLAGS) `pkg-config --cflags --libs ezimage`

lib: $(OBJ)
	$(AR) rcs libimaged.a $(OBJ)
	@printf "Name: imaged\nDescription: Imaging storage library\nVersion: $(VERSION)\nLibs: -L$(DEST)/lib -limaged $(LDFLAGS)\nCflags: -I/usr/local/include -I$(DEST)/include\nRequires: babl\n" > imaged.pc

shared: $(OBJ)
	$(CC) $(CFLAGS) $(PIC) -shared -o libimaged.so $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) libimaged.a libimaged.so .cflags .ldflags

install:
	mkdir -p $(DEST)/lib/pkgconfig $(DEST)/include $(DEST)/bin
	install libimaged.a $(DEST)/lib
	install src/imaged.h $(DEST)/include
	install imaged $(DEST)/bin
	install imaged.pc $(DEST)/lib/pkgconfig

uninstall:
	rm -f $(DEST)/lib/libimaged.a $(DEST)/include/imaged.h $(DEST)/bin/imaged $(DEST)/lib/pkgconfig/imaged.pc

.PHONY: rust
rust:
	cd rust && $(MAKE)
	cargo build

%.o: %.c base
	$(CC) $(PIC) -Wall -O3 -c $*.c $(CFLAGS) -o $@
