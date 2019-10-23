package imaged

// #cgo CFLAGS: -I../src
// #cgo LDFLAGS: -L.. -limaged -lm -lpthread
// #cgo pkg-config: babl
// #include <string.h>
// #include <stdlib.h>
// #include "imaged.h"
// ImagedMeta _meta(size_t w, size_t h, int color, ImagedKind kind, uint8_t bits);
import "C"

import (
	"errors"
	"unsafe"
)

type Imaged struct {
	ptr *C.Imaged
}

func Open(path string) (*Imaged, error) {
	tmp := C.CString(path)
	defer C.free(unsafe.Pointer(tmp))

	ptr := C.imagedOpen(tmp)
	if ptr == nil {
		return nil, errors.New("Cannot open DB")
	}

	return &Imaged{
		ptr: ptr,
	}, nil
}

func (db *Imaged) Close() {
	if db.ptr == nil {
		return
	}
	C.imagedClose(db.ptr)
	db.ptr = nil
}

func (db *Imaged) Destroy() {
	if db.ptr == nil {
		return
	}
	C.imagedDestroy(db.ptr)
	db.ptr = nil
}

func (db *Imaged) Iter() *Iter {
	iter := C.imagedIterNew(db.ptr)
	return &Iter{
		ptr: iter,
	}
}

func (db *Imaged) Set(key string, width, height uint64, color Color, t Type) (*Handle, error) {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ref := C.ImagedHandle{}
	rc := C.imagedSet(db.ptr, cKey, C.long(len(key)), C._meta(C.ulong(width), C.ulong(height), C.int(color), t.kind, C.uint8_t(t.bits)), nil, &ref)
	if rc != C.IMAGED_OK {
		err := C.GoString(C.imagedError(rc))
		return nil, errors.New(err)
	}

	return &Handle{
		ref: ref,
	}, nil
}

func (db *Imaged) Get(key string) (*Handle, error) {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ref := C.ImagedHandle{}
	rc := C.imagedGet(db.ptr, cKey, C.long(len(key)), true, &ref)
	if rc != C.IMAGED_OK {
		err := C.GoString(C.imagedError(rc))
		return nil, errors.New(err)
	}

	return &Handle{
		ref: ref,
	}, nil
}

func (db *Imaged) Remove(key string) error {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	rc := C.imagedRemove(db.ptr, cKey, C.long(len(key)))
	if rc != C.IMAGED_OK {
		err := C.GoString(C.imagedError(rc))
		return errors.New(err)
	}

	return nil
}
