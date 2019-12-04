package imaged

// #include "imaged.h"
import "C"

// Iter wraps the libimaged ImagedIter type
type Iter struct {
	ptr *C.ImagedIter
}

// Next loads the next entry, returns false when iteration has finished
func (i *Iter) Next() bool {
	im := C.imagedIterNext(i.ptr)
	return im != nil
}

// Image returns the current image
// NOTE: this image is owned by the iterator and should not be used
// after it has been closed
func (i *Iter) Image() *Image {
	ptr := &i.ptr.handle.image
	return &Image{
		ptr:   ptr,
		owner: false,
	}
}

// Key returns the current key
func (i *Iter) Key() string {
	s := i.ptr.key
	return C.GoString(s)
}

// Reset the iterator
func (i *Iter) Reset() {
	C.imagedIterReset(i.ptr)
}

// Close the iterator
func (i *Iter) Close() {
	C.imagedIterFree(i.ptr)
}
