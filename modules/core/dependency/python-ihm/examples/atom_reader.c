/* This is a simple demonstration of using the C mmCIF parser
   directly from C code. It will read the named mmCIF file and
   print the name and coordinates of each atom in the file.

   It is probably most instructive to read the comments in this file starting
   at the bottom (main function) and working back up.

   Compile with something like
     gcc -g -Wall atom_reader.c ../src/ihm_format.c -I ../src/ -o atom_reader
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "ihm_format.h"

/* Data that is passed to our callback function */
struct atom_site_data {
  struct ihm_keyword *id, *x, *y, *z;
};

/* Callback function called for each data item in atom_site */
static void atom_site_handler(struct ihm_reader *reader, void *data,
                              struct ihm_error **err)
{
  struct atom_site_data *ad = data;

  /* Here we assume that data is actually present in the file for each keyword.
     More generally, we should query the in_file, omitted, and unknown flags
     in the ihm_keyword struct to handle missing keywords or those that have
     the '.' or '?' values, respectively */
  printf("Atom %s at %s,%s,%s\n", ad->id->data, ad->x->data, ad->y->data,
                                  ad->z->data);
}

/* Register a callback function with the ihm_reader to handle the atom_site
   category */
static void add_atom_site_handler(struct ihm_reader *reader)
{
  struct atom_site_data *data = malloc(sizeof(struct atom_site_data));

  /* Register a callback for the atom_site category. 'data' will be passed
     to it (and 'data' will be freed with 'free' when we're done) */
  struct ihm_category *c = ihm_category_new(reader, "_atom_site",
                                            atom_site_handler, NULL, NULL, data,
                                            free);

  /* Ask the reader to extract a set of keywords from the atom_site
     category. ihm_keywords are stored in the ihm_category and are automatically
     freed when no longer needed. The actual values are stored in the
     ihm_keyword objects, so we give our callback a pointer to each one so
     it can get the values. */
  data->id = ihm_keyword_new(c, "label_atom_id");
  data->x = ihm_keyword_new(c, "cartn_x");
  data->y = ihm_keyword_new(c, "cartn_y");
  data->z = ihm_keyword_new(c, "cartn_z");
}

static void read_mmcif_filedesc(int fd)
{
  int more_data;
  /* IHM error indicator. NULL corresponds to no error. If a function fails
     this will be set to non-NULL */
  struct ihm_error *err = NULL;

  /* Point an ihm_reader object to the file */
  struct ihm_file *fh = ihm_file_new_from_fd(fd);
  struct ihm_reader *reader = ihm_reader_new(fh);

  /* Add callback functions that will handle file data */
  add_atom_site_handler(reader);

  /* Actually read the file. more_data will be set TRUE on return iff the
     file contains more data blocks after this one. */
  if (!ihm_read_file(reader, &more_data, &err)) {
    fprintf(stderr, "IHM error: %s\n", err->msg);
    ihm_error_free(err);
    ihm_reader_free(reader);
    exit(1);
  }
  ihm_reader_free(reader);
}

static void read_mmcif_filename(const char *fname)
{
  int fd;
  printf("Reading atoms from %s\n", fname);

  fd = open(fname, O_RDONLY);
  if (fd >= 0) {
    read_mmcif_filedesc(fd);
    close(fd);
  } else {
    fprintf(stderr, "Could not open %s: %s\n", fname, strerror(errno));
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: atom_reader filename.cif\n");
    return 1;
  }
  read_mmcif_filename(argv[1]);
  return 0;
}
