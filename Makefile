SRC=src/util.c src/iter.c src/db.c src/image.c src/pixel.c
OBJ=$(SRC:.c=.o)
CFLAGS?=-Wall -Wextra
LDFLAGS?=-lpthread
PIC?=-fPIC
DEST?=/usr/local
INTRIN?=yes

ifeq (yes,$(INTRIN))
HAS_SSE=$(shell $(CC) -mavx -dM -E - < /dev/null | egrep "SSE" | sort | grep '__SSE__ 1')
ifneq (,$(HAS_SSE))
	CFLAGS+= -msse
endif # HAS_SSE
endif # INTRIN

$(shell echo $(CFLAGS) > .cflags)
$(shell echo $(LDFALGS) > .ldflags)

build: lib bin
fresh: src/imaged.h .cflags .ldflags build

.PHONY: bin
bin: src/imaged.h .cflags $(OBJ)
	$(CC) -o imaged $(CFLAGS) $(OBJ) bin/imaged.c $(LDFLAGS) `pkg-config --cflags --libs ezimage`

lib: $(OBJ)
	$(AR) rcs libimaged.a $(OBJ)

shared: $(OBJ)
	$(CC) $(CFLAGS) $(PIC) -shared -o libimaged.so $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) libimaged.a libimaged.so .cflags .ldflags

install:
	mkdir -p $(DEST)/lib $(DEST)/include $(DEST)/bin
	install libimaged.a $(DEST)/lib
	install src/imaged.h $(DEST)/include
	install imaged $(DEST)/bin

uninstall:
	rm -f $(DEST)/lib/libimaged.a $(DEST)/include/imaged.h $(DEST)/bin/imaged

.PHONY: rust
rust:
	cd rust && $(MAKE)
	cargo build

%.o: %.c base
	$(CC) $(PIC) -Wall -O3 -c $*.c $(CFLAGS) -o $@
