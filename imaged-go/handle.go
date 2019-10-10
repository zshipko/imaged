package imaged

// #include "imaged.h"
import "C"

type Handle struct {
	ref C.ImagedHandle
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
