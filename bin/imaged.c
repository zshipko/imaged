#define _GNU_SOURCE
#define IMAGED_EZIMAGE
#include "../src/imaged.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *usage_s =
    "Usage: imaged -r [PATH] [COMMAND] [ARGS...]\nCommands:"
    "\n\tlist"
    "\n\tget [KEY]"
    "\n\tset [KEY] [WIDTH] [HEIGHT] [COLOR] [TYPE]"
    "\n\tremove [KEY]"
    "\n\timport [KEY] [PATH]"
    "\n\texport [KEY] [PATH]"
    "\n";

static void usage() { fputs(usage_s, stderr); }

int main(int argc, char *argv[]) {
  int opt;

  const char *root = NULL;

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

  $Imaged(db) = imagedOpen(root);
  if (db == NULL) {
    perror("Unable to open database");
    return 1;
  }

  const char *cmd = argv[optind++];

  if (strncasecmp(cmd, "list", 4) == 0) {
    ImagedIter *iter = imagedIterNew(db);

    while (imagedIterNext(iter) != NULL) {
      printf("%s\t%" PRIu64 "x%" PRIu64 "\t%s\t%s\n", iter->key,
             iter->handle.image.meta.width, iter->handle.image.meta.height,
             imageColorName(iter->handle.image.meta.color),
             imageTypeName(iter->handle.image.meta.kind,
                           iter->handle.image.meta.bits));
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
    $ImagedHandle(handle);
    if ((rc = imagedGet(db, key, -1, false, &handle)) != IMAGED_OK) {
      imagedPrintError(rc, "Unable to get image");
      return 1;
    }
    printf("%s\t%" PRIu64 "x%" PRIu64 "\t%s\t%s\n", key,
           handle.image.meta.width, handle.image.meta.height,
           imageColorName(handle.image.meta.color),
           imageTypeName(handle.image.meta.kind, handle.image.meta.bits));
  } else if (strncasecmp(cmd, "set", 3) == 0) {
    if (argc < optind + 5) {
      usage();
      return 1;
    }

    const char *key = argv[optind++];
    uint64_t width = strtoll(argv[optind++], NULL, 10);
    uint64_t height = strtoll(argv[optind++], NULL, 10);

    ImageColor color = 0;
    ImageKind kind = 0;
    uint8_t bits = 0;

    const char *a = argv[optind++];
    const char *b = argv[optind];
    if (!imageParseColorAndType(a, b, &color, &kind, &bits)) {
      fprintf(stderr, "Unable to parse color and type: %s %s\n", a, b);
      return 1;
    }

    ImageMeta meta = {
        .width = width,
        .height = height,
        .color = color,
        .bits = bits,
        .kind = kind,
    };

    ImagedStatus rc;
    if ((rc = imagedSet(db, key, -1, meta, NULL, NULL)) != IMAGED_OK) {
      imagedPrintError(rc, "Unable to set image");
      return 1;
    }
    puts("OK");

  } else if (strncasecmp(cmd, "import", 6) == 0) {
    if (argc < optind + 2) {
      usage();
      return 1;
    }
    const char *key = argv[optind++];
    const char *filename = argv[optind++];
    Image *image = imageRead(filename, -1, -1, 0);
    if (image == NULL) {
      fprintf(stderr, "Unable to open image: %s\n", filename);
      return 1;
    }

    imagedSet(db, key, -1, image->meta, image->data, NULL);
    imageFree(image);
    puts("OK");
  } else if (strncasecmp(cmd, "export", 6) == 0) {
    if (argc < optind + 2) {
      usage();
      return 1;
    }
    const char *key = argv[optind++];
    const char *filename = argv[optind++];

    $(ImagedHandle, ImagedHandle, handle);
    ImagedStatus rc;
    if ((rc = imagedGet(db, key, -1, false, &handle)) != IMAGED_OK) {
      imagedPrintError(rc, "Cannot open key");
      return 1;
    }

    if (!imageWrite(filename, &handle.image)) {
      fprintf(stderr, "Unable to write image: %s\n", filename);
    } else {
      puts("OK");
    }
  } else {
    fprintf(stderr, "Invalid command: %s\n", cmd);
    usage();
  }

  return 0;
}
