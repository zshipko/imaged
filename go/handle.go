package imaged

// #include "imaged.h"
import "C"

type Handle struct {
	ref C.ImagedHandle
}

func (h *Handle) Close() {
	C.imagedHandleClose(&h.ref)
}

func (i *Handle) Image() *Image {
	ptr := &i.ref.image
	return &Image{
		ptr:   ptr,
		owner: false,
	}
}
