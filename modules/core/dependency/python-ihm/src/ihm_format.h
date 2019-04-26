/** \file ihm_format.h      Routines for handling mmCIF format files.
 *
 *  The file is read sequentially. All values for desired keywords in
 *  desired categories are collected (other parts of the file are ignored)
 *  At the end of the file and each save frame a callback function for
 *  each category is called to process the data. In the case of mmCIF loops,
 *  this callback will be called multiple times, one for each entry in the loop.
 */

#ifndef IHM_FORMAT_H
#define IHM_FORMAT_H

#include <stdlib.h> /* For size_t */
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <unistd.h> /* For ssize_t */
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* IHM error types */
typedef enum {
  IHM_ERROR_VALUE, /* Bad value */
  IHM_ERROR_IO, /* Input/output error */
  IHM_ERROR_FILE_FORMAT, /* File format error */
} IHMErrorCode;

/* Error reported by IHM functions. The caller is responsible for freeing
   the memory used by this struct by calling ihm_error_free(). */
struct ihm_error {
  /* The type of error */
  IHMErrorCode code;
  /* Human-readable error message */
  char *msg;
};

/* Free the memory used by an ihm_error */
void ihm_error_free(struct ihm_error *err);

/* Set the error indicator */
void ihm_error_set(struct ihm_error **err, IHMErrorCode code,
                   const char *format, ...);

/* A keyword in an mmCIF file. Holds a description of its format and any
   value read from the file. */
struct ihm_keyword {
  char *name;
  /* Last value read from the file */
  char *data;
  /* If TRUE, we own the memory for data */
  int own_data;
  /* TRUE iff this keyword is in the file (not necessarily with a value) */
  int in_file;
  /* TRUE iff the keyword is in the file but the value is omitted ('.') */
  int omitted;
  /* TRUE iff the keyword is in the file but the value is unknown ('?') */
  int unknown;
};

/* Opaque types */
struct ihm_reader;
struct ihm_category;

/* Callback for mmCIF category data. Should set err on failure */
typedef void (*ihm_category_callback)(struct ihm_reader *reader,
                                      void *data, struct ihm_error **err);

/* Callback for unknown mmCIF categories. Should set err on failure */
typedef void (*ihm_unknown_category_callback)(struct ihm_reader *reader,
                                              const char *category, int linenum,
                                              void *data,
                                              struct ihm_error **err);

/* Callback for unknown mmCIF keywords. Should set err on failure */
typedef void (*ihm_unknown_keyword_callback)(struct ihm_reader *reader,
                                             const char *category,
                                             const char *keyword, int linenum,
                                             void *data,
                                             struct ihm_error **err);

/* Callback to free arbitrary data */
typedef void (*ihm_free_callback)(void *data);

/* Make a new struct ihm_category and add it to the reader. */
struct ihm_category *ihm_category_new(struct ihm_reader *reader,
                                      const char *name,
                                      ihm_category_callback data_callback,
                                      ihm_category_callback end_frame_callback,
                                      ihm_category_callback finalize_callback,
                                      void *data, ihm_free_callback free_func);

/* Set a callback for unknown categories.
   The given callback is called whenever a category is encountered in the
   file that is not handled (by ihm_category_new).
 */
void ihm_reader_unknown_category_callback_set(struct ihm_reader *reader,
                                     ihm_unknown_category_callback callback,
                                     void *data, ihm_free_callback free_func);

/* Set a callback for unknown keywords.
   The given callback is called whenever a keyword is encountered in the
   file that is not handled (within a category that is handled by
   ihm_category_new).
 */
void ihm_reader_unknown_keyword_callback_set(struct ihm_reader *reader,
                                     ihm_unknown_keyword_callback callback,
                                     void *data, ihm_free_callback free_func);

/* Remove all categories from the reader.
   This also removes any unknown category or keyword callbacks.
 */
void ihm_reader_remove_all_categories(struct ihm_reader *reader);

/* Add a new struct ihm_keyword to a category. */
struct ihm_keyword *ihm_keyword_new(struct ihm_category *category,
                                    const char *name);

struct ihm_file;
struct ihm_string;

/* Read data into the ihm_file buffer.
   Return the number of bytes read (0 on EOF), or -1 (and sets err) on failure.
 */
typedef ssize_t (*ihm_file_read_callback)(char *buffer, size_t buffer_len,
                                          void *data, struct ihm_error **err);

/* Track a file (or filelike object) that the data is read from */
struct ihm_file {
  /* Raw data read from the file */
  struct ihm_string *buffer;
  /* Offset into buffer of the start of the current line */
  size_t line_start;
  /* Offset into buffer of the start of the next line, or line_start if the
     line hasn't been read yet */
  size_t next_line_start;
  /* Callback function to read more data into buffer */
  ihm_file_read_callback read_callback;
  /* Data to pass to callback function */
  void *data;
  /* Function to free callback_data (or NULL) */
  ihm_free_callback free_func;
};

/* Make a new ihm_file, used to handle reading data from a file.
   `read_callback` is used to read a chunk of data from the file;
   `data` is arbitrary data that is passed to the read callback;
   `free_func` is used to do any necessary cleanup of `data` when
   the ihm_file structure is freed. */
struct ihm_file *ihm_file_new(ihm_file_read_callback read_callback,
                              void *data, ihm_free_callback free_func);

/* Make a new ihm_file that will read data from the given file descriptor */
struct ihm_file *ihm_file_new_from_fd(int fd);

/* Make a new struct ihm_reader */
struct ihm_reader *ihm_reader_new(struct ihm_file *fh);

/* Free memory used by a struct ihm_reader.
   Note that this does not close the
   underlying file descriptor or object that is wrapped by ihm_file. */
void ihm_reader_free(struct ihm_reader *reader);

/* Read a data block from an mmCIF file.
   *more_data is set TRUE iff more data blocks are available after this one.
   Return FALSE and set err on error. */
int ihm_read_file(struct ihm_reader *reader, int *more_data,
                  struct ihm_error **err);

#ifdef  __cplusplus
}
#endif

#endif /* IHM_FORMAT_H */
