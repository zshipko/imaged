package imaged

// #include "imaged.h"
import "C"

type Iter struct {
	ptr *C.ImagedIter
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
