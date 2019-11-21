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
}

START_TEST(test_image_size) {
  ImagedMeta meta;
  meta.color = IMAGED_COLOR_GRAY;
  meta.width = 150;
  meta.height = 100;
  meta.bits = 8;
  meta.kind = IMAGED_KIND_UINT;
  ck_assert(imagedMetaNumPixels(&meta) == 15000);

  meta.color = IMAGED_COLOR_RGB;
  meta.width = 150;
  meta.height = 100;
  meta.bits = 32;
  meta.kind = IMAGED_KIND_UINT;
  ck_assert(imagedMetaTotalBytes(&meta) == 15000 * 3 * 4);
}
END_TEST;

START_TEST(test_open) {
  init_test_db();
  ck_assert((db = imagedOpen("test/db")) != NULL);
}
END_TEST

START_TEST(test_set) {
  ImagedMeta meta = {
      .width = 800,
      .height = 600,
      .color = IMAGED_COLOR_RGB,
      .kind = IMAGED_KIND_FLOAT,
      .bits = 32,
  };

  $ImagedHandle(handle);
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
  ck_assert(meta.color == IMAGED_COLOR_RGB);
  ck_assert(meta.bits == 32);
}
END_TEST

START_TEST(test_iter) {
  $ImagedIter(iter) = imagedIterNew(db);
  ck_assert(iter != NULL);

  ck_assert(imagedIterNextKey(iter) != NULL);
  ck_assert(strncmp(iter->key, "testing", strlen(iter->key)) == 0);
  ck_assert(imagedIterNextKey(iter) == NULL);
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
