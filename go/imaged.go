package imaged

// #cgo CFLAGS: -I../src
// #cgo LDFLAGS: -lm -lpthread -L.. -ltiff -limaged
// #cgo pkg-config: babl libraw
// #include <string.h>
// #include <stdlib.h>
// #include "imaged.h"
// ImageMeta _meta(size_t w, size_t h, ImageColor color, ImageKind kind, uint8_t bits);
import "C"

import (
	"errors"
	"unsafe"
)

// Imaged is used to keep track of raw image data on disk
type Imaged struct {
	ptr *C.Imaged
}

// Open an Imaged instance, Close should always be called when finished
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

// Close an Imaged instance
func (db *Imaged) Close() {
	if db.ptr == nil {
		return
	}
	C.imagedClose(db.ptr)
	db.ptr = nil
}

// Destroy all images in the DB
func (db *Imaged) Destroy() {
	if db.ptr == nil {
		return
	}
	C.imagedDestroy(db.ptr)
	db.ptr = nil
}

// Iter returns a new iterator
func (db *Imaged) Iter() *Iter {
	iter := C.imagedIterNew(db.ptr)
	return &Iter{
		ptr: iter,
	}
}

// Create a new key with the specified size and metadata
func (db *Imaged) Create(key string, width, height uint64, color Color, t Type) (*Handle, error) {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ref := C.ImagedHandle{}
	rc := C.imagedSet(db.ptr, cKey, C.long(len(key)), C._meta(C.ulong(width), C.ulong(height), C.ImageColor(color), t.kind, C.uint8_t(t.bits)), nil, &ref)
	if rc != C.IMAGED_OK {
		err := C.GoString(C.imagedError(rc))
		return nil, errors.New(err)
	}

	return &Handle{
		ref: ref,
	}, nil
}

// Set an existing image to the specified key
func (db *Imaged) Set(key string, image *Image) (*Handle, error) {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ref := C.ImagedHandle{}
	rc := C.imagedSet(db.ptr, cKey, C.long(len(key)), C._meta(C.ulong(image.Width()), C.ulong(image.Height()), C.ImageColor(image.Color()), C.ImageKind(image.Type().Kind()), C.uint8_t(image.Type().Bits())), image.ptr.data, &ref)
	if rc != C.IMAGED_OK {
		err := C.GoString(C.imagedError(rc))
		return nil, errors.New(err)
	}

	return &Handle{
		ref: ref,
	}, nil
}

// Get an image
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

// Remove an image
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

// IsLocked checks if an image is locked
func (db *Imaged) IsLocked(key string) bool {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return bool(C.imagedKeyIsLocked(db.ptr, cKey, C.ssize_t(len(key))))
}

// IsValidFile checks if the specified key is a valid imgd file
func (db *Imaged) IsValidFile(key string) bool {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return bool(C.imagedIsValidFile(db.ptr, cKey, C.ssize_t(len(key))))
}
