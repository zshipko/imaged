package imaged

// #include "imaged.h"
import "C"

import (
	"sync"
)

type Type struct {
	bits uint8
	kind C.ImagedKind
}

func (t Type) Bits() uint {
	return uint(t.bits)
}

func (t Type) Kind() uint {
	return uint(t.kind)
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

type Image struct {
	ptr   *C.Image
	owner bool
}

type Pixel struct {
	inner C.Pixel
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

func (i *Image) GetPixel(x, y uint, px *Pixel) bool {
	return bool(C.imageGetPixel(i.ptr, C.ulong(x), C.ulong(y), &px.inner))
}

func (i *Image) SetPixel(x, y uint, px *Pixel) bool {
	return bool(C.imageSetPixel(i.ptr, C.ulong(x), C.ulong(y), &px.inner))
}

func EmptyPixel() Pixel {
	return Pixel{inner: C.pixelEmpty()}
}

func (px *Pixel) Set(index int, f float32) {
	if index > 3 || index < 0 {
		return
	}
	px.inner.data[index] = C.float(f)
}

func (px *Pixel) Get(index int) float32 {
	if index > 3 || index < 0 {
		return 0.0
	}

	return float32(px.inner.data[index])
}

func (image *Image) EachPixel(f func(x, y uint, px *Pixel)) {
	var wg sync.WaitGroup

	wg.Add(int(image.Height()))
	for y := uint64(0); y < image.Height(); y++ {
		yCopy := y
		go func() {
			px := EmptyPixel()
			for x := uint64(0); x < image.Width(); x++ {
				image.GetPixel(uint(x), uint(yCopy), &px)
				f(uint(x), uint(yCopy), &px)
				image.SetPixel(uint(x), uint(yCopy), &px)
			}
			wg.Done()
		}()
	}
	wg.Wait()
}
