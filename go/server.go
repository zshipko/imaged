package imaged

// #include "imaged.h"
// #include <stdlib.h>
import "C"

import (
	"crypto/rand"
	"encoding/hex"
	"errors"
	"io"
	"os"
	"path/filepath"
	"unsafe"

	"github.com/zshipko/worm"
)

// Context defines the resources the server needs access to
type Context struct {
	DB *Imaged
}

// Set command
func (c *Context) Set(client *worm.Client, key, width, height, color, ty *worm.Value) error {
	keyString := key.ToString()
	w := uint64(width.ToInt64())
	h := uint64(height.ToInt64())

	colorStr := C.CString(color.ToString())
	typeStr := C.CString(ty.ToString())
	defer C.free(unsafe.Pointer(colorStr))
	defer C.free(unsafe.Pointer(typeStr))

	var cl C.ImagedColor
	var t C.ImagedKind
	var bits C.uint8_t
	if !bool(C.imagedParseColorAndType(colorStr, typeStr, &cl, &t, &bits)) {
		return client.WriteError("invalid color/type")
	}

	handle, err := c.DB.Set(keyString, w, h, Color(cl), Type{bits: uint8(bits), kind: t})
	if err != nil {
		return err
	}
	defer handle.Close()

	return client.WriteOK()
}

// Remove command
func (c *Context) Remove(client *worm.Client, key *worm.Value) error {
	keyString := key.ToString()

	if err := c.DB.Remove(keyString); err != nil {
		return err
	}

	return client.WriteOK()
}

// List command
func (c *Context) List(client *worm.Client) error {
	iter := c.DB.Iter()
	defer iter.Close()
	names := []*worm.Value{}
	for iter.Next() {
		w, h, c, t := iter.Image().Meta()
		typeName := C.GoString(C.imagedTypeName(t.kind, C.uint8_t(t.bits)))
		entry := []*worm.Value{
			worm.New(iter.Key()),
			worm.New(w),
			worm.New(h),
			worm.New(int(c)),
			worm.New(typeName),
		}
		names = append(names, worm.New(entry))
	}
	return client.WriteValue(worm.New(names))
}

// RemoveAll command
func (c *Context) RemoveAll(client *worm.Client) error {
	iter := c.DB.Iter()
	defer iter.Close()

	for iter.Next() {
		C.imagedHandleClose(&iter.ptr.handle)
		c.DB.Remove(iter.Key())
	}

	return client.WriteOK()
}

// GetPixel command
func (c *Context) GetPixel(client *worm.Client, key, x, y *worm.Value) error {
	keyStr := key.ToString()
	xPos := x.ToInt64()
	yPos := y.ToInt64()
	pixel := EmptyPixel()
	handle, err := c.DB.Get(keyStr)
	if err != nil {
		return err
	}
	defer handle.Close()

	if !handle.Image().GetPixel(uint(xPos), uint(yPos), &pixel) {
		return errors.New("Unable to get pixel")
	}

	client.WriteArrayHeader(4)

	for i := 0; i < 4; i++ {
		client.WriteValue(worm.New(pixel.Get(i)))
	}

	return nil
}

// SetPixel command
func (c *Context) SetPixel(client *worm.Client, key, x, y *worm.Value, px ...*worm.Value) error {
	keyStr := key.ToString()
	xPos := x.ToInt64()
	yPos := y.ToInt64()
	pxLen := len(px)

	if pxLen == 0 {
		return worm.ErrNotEnoughArguments
	}

	pixel := EmptyPixel()

	if pxLen == 4 {
		pixel.Set(3, float32(px[3].ToFloat64()))
	} else {
		pixel.Set(3, 1.0)
	}

	for i := 0; i < 3; i++ {
		pixel.Set(i, float32(px[i%pxLen].ToFloat64()))
	}

	handle, err := c.DB.Get(keyStr)
	if err != nil {
		return err
	}
	defer handle.Close()

	if !handle.Image().SetPixel(uint(xPos), uint(yPos), &pixel) {
		return errors.New("Unable to set pixel")
	}

	return client.WriteOK()
}

func tempFileName(prefix, suffix string) string {
	randBytes := make([]byte, 16)
	rand.Read(randBytes)
	return filepath.Join(os.TempDir(), prefix+hex.EncodeToString(randBytes)+suffix)
}

// Export command
func (c *Context) Export(client *worm.Client, key, fmt *worm.Value) error {
	handle, err := c.DB.Get(key.ToString())
	if err != nil {
		return err
	}
	defer handle.Close()

	img := handle.Image()

	tmp := tempFileName("imaged", "."+fmt.ToString())

	if !img.Write(tmp) {
		return errors.New("Unable to write image")
	}

	f, err := os.Open(tmp)
	if err != nil {
		return errors.New("File was not written")
	}
	defer f.Close()
	defer os.Remove(tmp)

	info, err := f.Stat()
	if err != nil {
		return errors.New("Unable to get file information")
	}

	client.WriteStringHeader(int(info.Size()))
	_, err = io.Copy(f, client.Input)
	return err
}
