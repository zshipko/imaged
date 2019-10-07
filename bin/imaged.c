#define _GNU_SOURCE
#include "../src/imaged.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static const char *usage_s =
    "Usage: imaged -r [PATH] [COMMAND] [ARGS...]\nCommands:"
    "\n\tlist"
    "\n\tget [KEY]"
    "\n\tset [KEY] [WIDTH] [HEIGHT] [CHANNELS] [TYPE]"
    "\n\tremove [KEY]"
    "\n";

static void usage() { fputs(usage_s, stderr); }

int main(int argc, char *argv[]) {
  int opt;

  const char *root = "/tmp/imaged";

  while ((opt = getopt(argc, argv, "r:")) != -1) {
    switch (opt) {
    case 'r':
      root = optarg;
      break;
    default:
      fprintf(stderr, "Unknown flag %c\n", opt);
      usage();
      return 1;
    }
  }

  if (argc == optind) {
    usage();
    return 0;
  }

  defer(Imaged, db) = imagedOpen(root);
  if (db == NULL) {
    perror("Unable to open database");
    return 1;
  }

  const char *cmd = argv[optind++];

  if (strncasecmp(cmd, "list", 4) == 0) {
    ImagedIter *iter = imagedIterNew(db);

    while (imagedIterNext(iter) != NULL) {
      printf("%s\t%lux%lux%u\t%s%d\n", iter->key, iter->handle.image.meta.width,
             iter->handle.image.meta.height, iter->handle.image.meta.channels,
             iter->handle.image.meta.kind == IMAGED_KIND_INT
                 ? "i"
                 : iter->handle.image.meta.kind == IMAGED_KIND_UINT ? "u" : "f",
             iter->handle.image.meta.bits);
    }

    imagedIterFree(iter);
  } else if (strncasecmp(cmd, "remove", 6) == 0) {
    if (argc < optind + 1) {
      usage();
      return 1;
    }

    const char *key = argv[optind++];
    ImagedStatus rc;

    if ((rc = imagedRemove(db, key, -1)) != IMAGED_OK) {
      imagedPrintError(rc, "Unable to remove image");
      return 1;
    }
    puts("OK");

  } else if (strncasecmp(cmd, "get", 3) == 0) {
    if (argc < optind + 1) {
      usage();
      return 1;
    }

    const char *key = argv[optind++];
    ImagedStatus rc;
    ImagedHandle handle;
    if ((rc = imagedGet(db, key, -1, &handle)) != IMAGED_OK) {
      imagedPrintError(rc, "Unable to get image");
      return 1;
    }
    printf("%lux%lux%u\t%s%d\n", handle.image.meta.width,
           handle.image.meta.height, handle.image.meta.channels,
           handle.image.meta.kind == IMAGED_KIND_INT
               ? "i"
               : handle.image.meta.kind == IMAGED_KIND_UINT ? "u" : "f",
           handle.image.meta.bits);
    imagedHandleFree(&handle);
  } else if (strncasecmp(cmd, "set", 3) == 0) {
    if (argc < optind + 5) {
      usage();
      return 1;
    }

    const char *key = argv[optind++];
    uint64_t width = strtoll(argv[optind++], NULL, 10);
    uint64_t height = strtoll(argv[optind++], NULL, 10);
    uint8_t channels = atoi(argv[optind++]);
    uint8_t bits = 8;
    ImagedKind kind = IMAGED_KIND_UINT;
    if (strncasecmp(argv[optind], "u16", 3) == 0) {
      bits = 16;
    } else if (strncasecmp(argv[optind], "f32", 3) == 0) {
      bits = 32;
      kind = IMAGED_KIND_FLOAT;
    } else if (strncasecmp(argv[optind], "f64", 3) == 0) {
      bits = 64;
      kind = IMAGED_KIND_FLOAT;
    } else if (strncasecmp(argv[optind], "u8", 2) != 0) {
      fprintf(stderr, "Invalid data type: %s\n", argv[optind + 3]);
      return 1;
    }

    ImagedMeta meta = {
        .width = width,
        .height = height,
        .channels = channels,
        .bits = bits,
        .kind = kind,
    };

    ImagedStatus rc;
    if ((rc = imagedSet(db, key, -1, meta, NULL, NULL)) != IMAGED_OK) {
      imagedPrintError(rc, "Unable to set image");
      return 1;
    }
    puts("OK");
  } else {
    fprintf(stderr, "Invalid command: %s\n", cmd);
    usage();
  }

  return 0;
}
