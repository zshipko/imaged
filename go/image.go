package imaged

// #include "imaged.h"
// #include <stdlib.h>
import "C"

import (
	"sync"
	"unsafe"
)

// Color is an enumeration of possible image types
type Color int

// Color types
const (
	GRAY    Color = 1
	GRAYA   Color = 2
	RGB     Color = 3
	RGBA    Color = 4
	CMYK    Color = 5
	CMYKA   Color = 6
	YCBCR   Color = 7
	YCBCRA  Color = 8
	CIELAB  Color = 9
	CIELABA Color = 10
	CIELCH  Color = 11
	CIELCHA Color = 12
	CIEXYZ  Color = 13
	CIEXYZA Color = 14
	YUV     Color = 15
	YUVA    Color = 16
	HSL     Color = 17
	HSLA    Color = 18
	HSV     Color = 19
	HSVA    Color = 20
	CIEXYY  Color = 21
	CIEXYYA Color = 22
	HCY     Color = 23
	HCYA    Color = 24
)

// Type determines the storage type of the underlying data pointer
type Type struct {
	bits uint8
	kind C.ImagedKind
}

// Bits returns the numbers of bits
func (t Type) Bits() uint {
	return uint(t.bits)
}

// Kind returns the image kind
func (t Type) Kind() uint {
	return uint(t.kind)
}

// U8 = uint8_t
var U8 = Type{
	bits: 8,
	kind: C.IMAGED_KIND_UINT,
}

// U16 = uint16_t
var U16 = Type{
	bits: 16,
	kind: C.IMAGED_KIND_UINT,
}

// F16 = half
var F16 = Type{
	bits: 32,
	kind: C.IMAGED_KIND_FLOAT,
}

// F32 = float
var F32 = Type{
	bits: 32,
	kind: C.IMAGED_KIND_FLOAT,
}

// F64 = double
var F64 = Type{
	bits: 64,
	kind: C.IMAGED_KIND_FLOAT,
}

// Image wraps the libimaged Image type
type Image struct {
	ptr   *C.Image
	owner bool
}

// Pixel wraps the libimaged Pixel type
type Pixel struct {
	inner C.Pixel
}

// Free an image, must be called for all images
func (i *Image) Free() {
	if i.owner {
		C.imageFree(i.ptr)
	}
}

// Meta returns information about the image
func (i *Image) Meta() (uint64, uint64, Color, Type) {
	meta := i.ptr.meta
	return uint64(meta.width), uint64(meta.height), Color(meta.color), Type{
		bits: uint8(meta.bits),
		kind: meta.kind,
	}
}

// Width returns the width of the image
func (i *Image) Width() uint64 {
	w, _, _, _ := i.Meta()
	return w
}

// Height returns the height of the image
func (i *Image) Height() uint64 {
	_, h, _, _ := i.Meta()
	return h
}

// Color returns the image's color
func (i *Image) Color() Color {
	_, _, c, _ := i.Meta()
	return Color(c)
}

// Channels returns the number of channels in an image.
// For example: Gray = 1, RGB = 3, etc...
func (i *Image) Channels() int {
	_, _, c, _ := i.Meta()
	return int(C.imagedColorNumChannels(C.ImagedColor(c)))
}

// Type returns the type of the image
func (i *Image) Type() Type {
	_, _, _, t := i.Meta()
	return t
}

// GetPixel loads the values at (x, y) into px, returning true if successful
func (i *Image) GetPixel(x, y uint, px *Pixel) bool {
	return bool(C.imageGetPixel(i.ptr, C.ulong(x), C.ulong(y), &px.inner))
}

// SetPixel set the values at (x, y) to px, returning true if successful
func (i *Image) SetPixel(x, y uint, px *Pixel) bool {
	return bool(C.imageSetPixel(i.ptr, C.ulong(x), C.ulong(y), &px.inner))
}

// NumBytes returns the total number of bytes occupied by the image data
func (i *Image) NumBytes() uint64 {
	return uint64(C.imageBytes(i.ptr))
}

// ConvertTo converts an image to the type specified by the destination image
func (i *Image) ConvertTo(other *Image) bool {
	return bool(C.imageConvertTo(i.ptr, other.ptr))
}

// Convert returns a new image with the specified color and type
func (i *Image) Convert(color Color, t Type) *Image {
	im := C.imageConvert(i.ptr, C.ImagedColor(color), t.kind, C.uchar(t.bits))
	return &Image{
		ptr:   im,
		owner: true,
	}
}

// ResizeTo resizes an image to the size specified by the destination image
func (i *Image) ResizeTo(other *Image) {
	C.imageResizeTo(i.ptr, other.ptr)
}

// Resize returns a new image with the specified size
func (i *Image) Resize(width, height uint) *Image {
	im := C.imageResize(i.ptr, C.size_t(width), C.size_t(height))
	return &Image{
		ptr:   im,
		owner: true,
	}
}

// Scale returns a new image scaled by the specified factor on each axis
func (i *Image) Scale(x, y float64) *Image {
	im := C.imageScale(i.ptr, C.double(x), C.double(y))
	return &Image{
		ptr:   im,
		owner: true,
	}
}

// NewImage allocates a new image
// NOTE: Image.Free should be called when you're done with the image
func NewImage(width int, height int, color Color, t Type) *Image {
	im := C.imageAlloc(C.size_t(width), C.size_t(height), C.ImagedColor(color), C.ImagedKind(t.kind), C.uchar(t.bits), nil)
	return &Image{
		ptr:   im,
		owner: true,
	}
}

// EmptyPixel return a new empty pixel
func EmptyPixel() Pixel {
	return Pixel{inner: C.pixelEmpty()}
}

// Map executes f for each channel
func (px *Pixel) Map(f func(x float32) float32) {
	for i := 0; i < 4; i++ {
		px.Set(i, f(px.Get(i)))
	}
}

// All returns true when f is true for every channel
func (px *Pixel) All(f func(x float32) bool) bool {
	dest := true
	for i := 0; i < 4; i++ {
		dest = dest && f(px.Get(i))
	}
	return dest
}

// Any returns true when f is true for any channel
func (px *Pixel) Any(f func(x float32) bool) bool {
	dest := false
	for i := 0; i < 4; i++ {
		dest = dest || f(px.Get(i))
	}
	return dest
}

// Set pixel index
func (px *Pixel) Set(index int, f float32) {
	if index > 3 || index < 0 {
		return
	}
	px.inner.data[index] = C.float(f)
}

// Get pixel index
func (px *Pixel) Get(index int) float32 {
	if index > 3 || index < 0 {
		return 0.0
	}

	return float32(px.inner.data[index])
}

// EachPixel applies f to each pixel in an image
func (i *Image) EachPixel(f func(x, y uint, px *Pixel)) {
	var wg sync.WaitGroup

	wg.Add(int(i.Height()))
	for y := uint64(0); y < i.Height(); y++ {
		yCopy := y
		go func() {
			px := EmptyPixel()
			for x := uint64(0); x < i.Width(); x++ {
				i.GetPixel(uint(x), uint(yCopy), &px)
				f(uint(x), uint(yCopy), &px)
				i.SetPixel(uint(x), uint(yCopy), &px)
			}
			wg.Done()
		}()
	}
	wg.Wait()
}

// ReadImage from disk using ezimage
func ReadImage(filename string, color Color, t Type) *Image {
	s := C.CString(filename)
	defer C.free(unsafe.Pointer(s))
	im := C.imageRead(s, C.ImagedColor(color), C.ImagedKind(t.kind), C.uchar(t.bits))
	return &Image{
		ptr:   im,
		owner: true,
	}
}

// Write image to disk using ezimage
func (i *Image) Write(filename string) bool {
	s := C.CString(filename)
	defer C.free(unsafe.Pointer(s))

	return C.imageWrite(s, i.ptr) == C.IMAGED_OK
}
