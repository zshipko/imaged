package imaged

// #include "imaged.h"
import "C"

import (
	"errors"
	"strings"

	"github.com/zshipko/worm"
)

type Context struct {
	DB *Imaged
}

func (c *Context) Set(client *worm.Client, key, width, height, channels, ty *worm.Value) error {
	keyString := key.ToString()
	w := uint64(width.ToInt64())
	h := uint64(height.ToInt64())
	ch := uint8(channels.ToInt64())

	t := U8
	if x, ok := namesToTypes[strings.ToLower(ty.ToString())]; ok {
		t = x
	}

	handle, err := c.DB.Set(keyString, w, h, ch, t)
	if err != nil {
		return err
	}
	defer handle.Free()

	return client.WriteOK()
}

func (c *Context) Remove(client *worm.Client, key *worm.Value) error {
	keyString := key.ToString()

	if err := c.DB.Remove(keyString); err != nil {
		return err
	}

	return client.WriteOK()
}

func (c *Context) List(client *worm.Client) error {
	iter := c.DB.Iter()
	defer iter.Close()
	names := []*worm.Value{}
	for iter.Next() {
		w, h, c, t := iter.Image().Meta()
		entry := []*worm.Value{
			worm.New(iter.Key()),
			worm.New(w),
			worm.New(h),
			worm.New(int(c)),
			worm.New(typesToNames[t]),
		}
		names = append(names, worm.New(entry))
	}
	return client.WriteValue(worm.New(names))
}

func (c *Context) RemoveAll(client *worm.Client) error {
	iter := c.DB.Iter()
	defer iter.Close()

	for iter.Next() {
		C.imagedHandleFree(&iter.ptr.handle)
		c.DB.Remove(iter.Key())
	}

	return client.WriteOK()
}

func (c *Context) GetPixel(client *worm.Client, key, x, y *worm.Value) error {
	keyStr := key.ToString()
	xPos := x.ToInt64()
	yPos := y.ToInt64()
	pixel := EmptyPixel()
	handle, err := c.DB.Get(keyStr)
	if err != nil {
		return err
	}
	defer handle.Free()

	if !handle.Image().GetPixel(uint(xPos), uint(yPos), &pixel) {
		return errors.New("Unable to get pixel")
	}

	client.WriteArrayHeader(4)

	for i := 0; i < 4; i++ {
		client.WriteValue(worm.New(pixel.Get(i)))
	}

	return nil
}

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
	defer handle.Free()

	if !handle.Image().SetPixel(uint(xPos), uint(yPos), &pixel) {
		return errors.New("Unable to set pixel")
	}

	return client.WriteOK()
}

var typesToNames = map[Type]string{
	U8:  "u8",
	U16: "u16",
	F32: "f32",
	F64: "f64",
}

var namesToTypes = map[string]Type{
	"u8":  U8,
	"u16": U16,
	"f32": F32,
	"f64": F64,
}
