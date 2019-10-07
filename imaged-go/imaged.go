package imaged

// #cgo CFLAGS: -I../src
// #cgo LDFLAGS: -L.. -limaged
// #include <stdlib.h>
/*
#include "wrap.c"
*/
import "C"

import (
	"errors"
	"log"
	"unsafe"
)

type Imaged struct {
	ptr *C.Imaged
}

type Type struct {
	bits uint8
	kind C.ImagedKind
}

var U8 = Type{
	bits: 8,
	kind: C.IMAGED_KIND_UINT,
}

var U16 = Type{
	bits: 16,
	kind: C.IMAGED_KIND_UINT,
}

var F32 = Type{
	bits: 32,
	kind: C.IMAGED_KIND_FLOAT,
}

var F64 = Type{
	bits: 64,
	kind: C.IMAGED_KIND_FLOAT,
}

type Handle struct {
	ref C.ImagedHandle
}

type Image struct {
	ptr   *C.Image
	owner bool
}

type Iter struct {
	ptr *C.ImagedIter
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

func (h *Handle) Free() {
	C.imagedHandleFree(&h.ref)
}

func (i *Handle) Image() *Image {
	ptr := &i.ref.image
	return &Image{
		ptr:   ptr,
		owner: false,
	}
}

func (i *Iter) Next() bool {
	im := C.imagedIterNext(i.ptr)
	return im != nil
}

func (i *Iter) Image() *Image {
	ptr := &i.ptr.handle.image
	return &Image{
		ptr:   ptr,
		owner: false,
	}
}

func (i *Iter) Key() string {
	s := i.ptr.key
	return C.GoString(s)
}

func (i *Iter) Reset() {
	C.imagedIterReset(i.ptr)
}

func (i *Iter) Close() {
	C.imagedIterFree(i.ptr)
}

func (i *Image) Free() {
	if i.owner {
		C.imageFree(i.ptr)
	}
}

func (i *Image) Meta() (uint64, uint64, uint8, Type) {
	meta := i.ptr.meta
	return uint64(meta.width), uint64(meta.height), uint8(meta.channels), Type{
		bits: uint8(meta.bits),
		kind: meta.kind,
	}
}

func (i *Image) Width() uint64 {
	w, _, _, _ := i.Meta()
	return w
}
func (i *Image) Height() uint64 {
	_, h, _, _ := i.Meta()
	return h
}

func (i *Image) Channels() uint8 {
	_, _, c, _ := i.Meta()
	return c
}

func (i *Image) Type() Type {
	_, _, _, t := i.Meta()
	return t
}

func (db *Imaged) Iter() *Iter {
	iter := C.imagedIterNew(db.ptr)
	return &Iter{
		ptr: iter,
	}
}

func (db *Imaged) Set(key string, width, height uint64, channels uint8, t Type) (*Handle, error) {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ref := C.ImagedHandle{}
	rc := C.imagedSet(db.ptr, cKey, C.long(len(key)), C._meta(C.ulong(width), C.ulong(height), C.uint8_t(channels), t.kind, C.uint8_t(t.bits)), nil, &ref)
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
