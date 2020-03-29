#define _DEFAULT_SOURCE
#include "../src/imaged.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <check.h>

#define ASSERT_OK(x) ck_assert_int_eq(x, IMAGED_OK)
#define ASSERT_NOT_OK(x) ck_assert_int_eq(x, IMAGED_ERR)

Imaged *db;

static void init_test_db() {
  mkdir("test/db", 0755);
  remove("test/db/data.mdb");
  remove("test/db/lock.mdb");

  remove("test/out.png");
  remove("test/out.jpeg");
  remove("test/out.a.exr");
  remove("test/out.b.exr");
  remove("test/out.hdr");
}

START_TEST(test_image_size) {
  ImageMeta meta;
  meta.color = IMAGE_COLOR_GRAY;
  meta.width = 150;
  meta.height = 100;
  meta.bits = 8;
  meta.kind = IMAGE_KIND_UINT;
  ck_assert(imageMetaNumPixels(&meta) == 15000);

  meta.color = IMAGE_COLOR_RGB;
  meta.width = 150;
  meta.height = 100;
  meta.bits = 32;
  meta.kind = IMAGE_KIND_UINT;
  ck_assert(imageMetaTotalBytes(&meta) == 15000 * 3 * 4);
}
END_TEST;

START_TEST(test_open) {
  init_test_db();
  ck_assert((db = imagedOpen("test/db")) != NULL);
}
END_TEST

START_TEST(test_set) {
  ImageMeta meta = {
      .width = 800,
      .height = 600,
      .color = IMAGE_COLOR_RGB,
      .kind = IMAGE_KIND_FLOAT,
      .bits = 32,
  };

  $ImagedHandle(handle);
  ck_assert(imagedKeyIsLocked(db, "testing", -1) == false);
  ck_assert(imagedSet(db, "testing", -1, meta, NULL, &handle) == IMAGED_OK);
  float *data = (float *)handle.image.data;
  ck_assert(data);
  data[123] = 0.25;
  imagedHandleClose(&handle);

  ck_assert(imagedGet(db, "testing", -1, false, &handle) == IMAGED_OK);

  data = (float *)handle.image.data;

  ck_assert(data != NULL);
  ck_assert(data[123] == 0.25);
  ck_assert(meta.width == 800);
  ck_assert(meta.height == 600);
  ck_assert(meta.color == IMAGE_COLOR_RGB);
  ck_assert(meta.bits == 32);
}
END_TEST

START_TEST(test_iter) {
  $ImagedIter(iter) = imagedIterNew(db);
  ck_assert(iter != NULL);

  ck_assert(imagedIterNext(iter) != NULL);
  ck_assert(strncmp(iter->key, "testing", strlen(iter->key)) == 0);
  ck_assert(imagedIterNext(iter) == NULL);
}
END_TEST

START_TEST(test_remove) {
  ASSERT_OK(imagedRemove(db, "testing", -1));

  $ImagedHandle(handle);
  ck_assert(imagedGet(db, "testing", -1, false, &handle) != IMAGED_OK);
}
END_TEST

START_TEST(test_imaged_reset) {
  $ImagedIter(iter) = imagedIterNew(db);

  const char *key = NULL;

  while ((key = imagedIterNextKey(iter))) {
    ASSERT_OK(imagedRemove(db, key, -1));
  }
}
END_TEST

START_TEST(test_pixel) {
  Pixel a = pixelEmpty();
  Pixel b = pixelRGBA(0.0, 0.0, 0.0, 0.0);
  ck_assert(pixelEqAll(&a, 0.0f));
  ck_assert(pixelEq(&a, &b));

  a.data[0] = 1.0;
  a.data[1] = 1.0;
  a.data[2] = 1.0;

  ck_assert(pixelSum(&a) == 3.0f);
  ck_assert(!pixelEq(&a, &b));

  a.data[0] = 2.0;
  ck_assert(pixelSum(&a) == 4.0f);
  pixelClamp(&a);
  ck_assert(pixelSum(&a) == 3.0f);
}
END_TEST;

START_TEST(test_image) {
  $Image(im) =
      imageAlloc(800, 600, IMAGE_COLOR_RGB, IMAGE_KIND_FLOAT, 32, NULL);
  ck_assert(im != NULL);
  ck_assert(im->meta.width == 800);
  ck_assert(im->meta.height == 600);
  ck_assert(im->meta.color == IMAGE_COLOR_RGB);
  ck_assert(im->meta.kind == IMAGE_KIND_FLOAT);
  ck_assert(im->meta.bits == 32);
  ck_assert(imageColorNumChannels(im->meta.color) == 3);

  Pixel p = pixelRGB(1.0, 0.5, 0.75), q;
  imageSetPixel(im, 400, 300, &p);

  imageGetPixel(im, 400, 300, &q);

  ck_assert(pixelEq(&p, &q));
}
END_TEST;

START_TEST(test_image_convert) {
  $Image(a) = imageAlloc(800, 600, IMAGE_COLOR_RGB, IMAGE_KIND_FLOAT, 32, NULL);
  $Image(b) = imageConvert(a, IMAGE_COLOR_RGBA, IMAGE_KIND_UINT, 16);

  ck_assert(b->meta.width == 800);
  ck_assert(b->meta.height == 600);
  ck_assert(b->meta.color == IMAGE_COLOR_RGBA);
  ck_assert(b->meta.kind == IMAGE_KIND_UINT);
  ck_assert(b->meta.bits == 16);
  ck_assert(imageColorNumChannels(b->meta.color) == 4);

  Pixel p, q;

  imageGetPixel(a, 400, 300, &p);
  imageGetPixel(b, 400, 300, &q);
  ck_assert(pixelEq(&p, &q));
}
END_TEST;

START_TEST(test_image_resize) {
  $Image(a) = imageAlloc(800, 600, IMAGE_COLOR_RGB, IMAGE_KIND_FLOAT, 32, NULL);
  $Image(b) = imageScale(a, 2.0f, 1.5f);
  ck_assert(b->meta.width == 1600);
  ck_assert(b->meta.height == 900);
  ck_assert(b->meta.color == IMAGE_COLOR_RGB);
  ck_assert(b->meta.kind == IMAGE_KIND_FLOAT);
  ck_assert(b->meta.bits == 32);
}
END_TEST;

START_TEST(test_image_io) {
  $Image(a) = imageRead("test/test.exr", IMAGE_COLOR_RGB, IMAGE_KIND_FLOAT, 32);
  ck_assert(a->meta.color == IMAGE_COLOR_RGB);
  ck_assert(a->meta.kind == IMAGE_KIND_FLOAT);
  ck_assert(a->meta.bits == 32);

  imageConvertInPlace(&a, IMAGE_COLOR_RGB, IMAGE_KIND_UINT, 8);
  ASSERT_OK(imageWrite("test/out.png", a));
  ASSERT_OK(imageWrite("test/out.jpeg", a));
}
END_TEST;

START_TEST(test_image_io_exr) {
  $Image(a) = imageRead("test/test.exr", IMAGE_COLOR_RGB, IMAGE_KIND_FLOAT, 32);
  ck_assert(a->meta.color == IMAGE_COLOR_RGB);
  ck_assert(a->meta.kind == IMAGE_KIND_FLOAT);
  ck_assert(a->meta.bits == 32);

  $Image(b) = imageConvert(a, IMAGE_COLOR_RGB, IMAGE_KIND_FLOAT, 16);

  ASSERT_OK(imageWrite("test/out.a.exr", a));
  ASSERT_OK(imageWrite("test/out.b.exr", b));
}
END_TEST;

bool parallel_fn(IMAGED_UNUSED uint64_t x, IMAGED_UNUSED uint64_t y, Pixel *px,
                 IMAGED_UNUSED void *userdata) {
  px->data[0] = px->data[1] = px->data[2] = px->data[3] = 1.0;
  return true;
}

START_TEST(test_each_pixel) {
  Image *im =
      imageAlloc(800, 600, IMAGE_COLOR_RGBA, IMAGE_KIND_FLOAT, 32, NULL);
  ck_assert(imageEachPixel(im, parallel_fn, 4, NULL) == IMAGED_OK);

  float *data = im->data;
  for (size_t i = 0; i < im->meta.width * im->meta.height *
                             imageColorNumChannels(im->meta.color);
       i++) {
    ck_assert(data[i] == 1.0);
  }
  imageFree(im);
}
END_TEST;

#define BASIC(name) tcase_add_test(basic, name);

Suite *imaged_test_suite() {
  Suite *s;
  TCase *basic;
  s = suite_create("Imaged");

  // Basic
  basic = tcase_create("Basic");
  BASIC(test_image_size);
  BASIC(test_open);
  BASIC(test_set);
  BASIC(test_iter);
  BASIC(test_remove);
  BASIC(test_imaged_reset);
  BASIC(test_pixel);
  BASIC(test_image);
  BASIC(test_image_convert);
  BASIC(test_image_resize);
  BASIC(test_image_io);
  BASIC(test_image_io_exr);
  BASIC(test_each_pixel);

  suite_add_tcase(s, basic);
  return s;
}

int main(void) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = imaged_test_suite();
  sr = srunner_create(s);

  srunner_set_fork_status(sr, CK_NOFORK);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  imagedClose(db);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
