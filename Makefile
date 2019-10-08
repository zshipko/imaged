SRC=src/util.c src/iter.c src/db.c src/image.c
OBJ=$(SRC:.c=.o)
CFLAGS?=-Wall -Wextra
LDFLAGS?=
PIC?=-fPIC
DEST?=/usr/local
INTRIN?=yes

ifeq (yes,$(INTRIN))
HAS_AVX=$(shell $(CC) -mavx -dM -E - < /dev/null | egrep "AVX" | sort | grep '__AVX__ 1')
ifneq (,$(HAS_AVX))
	CFLAGS+= -mavx
endif # HAS_AVX
endif # INTRIN

$(shell echo $(CFLAGS) > .cflags)
$(shell echo $(LDFALGS) > .ldflags)

build: lib bin

base: src/imaged.h .cflags .ldflags

.PHONY: bin
bin: base $(OBJ)
	$(CC) -o imaged $(CFLAGS) $(OBJ) bin/imaged.c $(LDFLAGS) `pkg-config --cflags --libs ezimage`

lib: base $(OBJ)
	ar rcs libimaged.a $(OBJ)

shared: base $(OBJ)
	$(CC) $(CFLAGS) $(PIC) -shared -o libimaged.so $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) libimaged.a libimaged.so .cflags .ldflags

install:
	mkdir -p $(DEST)/lib $(DEST)/include
	install libimaged.a $(DEST)/lib
	install src/imaged.h $(DEST)/include

uninstall:
	rm -f $(DEST)/lib/libimaged.a $(DEST)/include/imaged.h

%.o: %.c base
	$(CC) $(PIC) -Wall -O3 -c $*.c $(CFLAGS) -o $@
