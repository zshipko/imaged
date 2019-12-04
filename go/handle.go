package imaged

// #include "imaged.h"
import "C"

// Handle wraps the libimage ImagedHandle type
type Handle struct {
	ref C.ImagedHandle
}

// Close the handle
func (h *Handle) Close() {
	C.imagedHandleClose(&h.ref)
}

// Image returns the underlying image
func (h *Handle) Image() *Image {
	ptr := &h.ref.image
	return &Image{
		ptr:   ptr,
		owner: false,
	}
}
