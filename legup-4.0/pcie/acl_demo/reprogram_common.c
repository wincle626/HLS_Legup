/*
 * Copyright (c) 2013, Altera Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/* This file contains helper functions shared between Linux
 * and Windows version of 'reprogram'.
 * The file should be #include'd into .c/.cpp files.
 */
#ifndef LINUX
typedef size_t ssize_t;
#endif

int filename_has_ext (const char *filename, const char *ext);
int can_read_filename (const char* filename);
void map_filenames_to_sof_and_rbf (const char* filename1, const char* filename2,
                                   const char** sof, const char** core_rbf, const char** rbf);
unsigned char *acl_loadFileIntoMemory (const char *in_file, size_t *file_size_out);


/* return 1 if given filename has given extension */
int filename_has_ext (const char *filename, const char *ext)
{
   size_t ext_len = strlen (ext);  
   return (strcmp (filename + strlen(filename) - ext_len, ext) == 0);
}


/* Return 1 if can open given filename for reading. */
int can_read_filename (const char* filename) {
  FILE *temp = fopen (filename, "r");
  if (temp == NULL) {
    printf ("Cannot read file: %s\n", filename);
    return 0;
  } else {
    fclose (temp);
    return 1;
  }
}


void map_filenames_to_sof_and_rbf (const char* filename1, const char* filename2,
                                   const char** sof, const char **core_rbf, const char** rbf) {

  assert (sof != NULL && rbf != NULL);
  if (filename_has_ext (filename1, ".sof") || 
      filename_has_ext (filename1, ".SOF") ) {
    *sof = filename1;
  }
  else if (filename_has_ext (filename1, ".core.rbf") || 
      filename_has_ext (filename1, ".CORE.RBF")) {
    *core_rbf = filename1;
  }
  else if (filename_has_ext (filename1, ".rbf") ||
      filename_has_ext (filename1, ".RBF")) {
    *rbf = filename1;
  }

  if (filename2 != NULL) {
    if (filename_has_ext (filename2, ".sof") || 
        filename_has_ext (filename2, ".SOF") ) {
      *sof = filename2;
    }
    else if (filename_has_ext (filename2, ".core.rbf") || 
        filename_has_ext (filename2, ".CORE.RBF")) {
      *core_rbf = filename2;
    }
    else if (filename_has_ext (filename2, ".rbf") || 
        filename_has_ext (filename2, ".RBF")) {
      *rbf = filename2;
    }
  }
}


/* given filename, load its content into memory.
 * Returns file size in file_size_out ptr and ptr to buffer (allocated
 * with malloc() by this function that contains the content of the file.*/
unsigned char *acl_loadFileIntoMemory (const char *in_file, size_t *file_size_out) {

  FILE *f = NULL;
  unsigned char *buf;
  size_t file_size;
  
  // When reading as binary file, no new-line translation is done.
  f = fopen (in_file, "rb");
  if (f == NULL) {
    fprintf (stderr, "Couldn't open file %s for reading\n", in_file);
    return NULL;
  }
  
  // get file size
  fseek (f, 0, SEEK_END);
  file_size = ftell (f);
  rewind (f);
  
  // slurp the whole file into allocated buf
  buf = (unsigned char*) malloc (sizeof(char) * file_size);
  *file_size_out = fread (buf, sizeof(char), file_size, f);
  fclose (f);
  
  if (*file_size_out != file_size) {
    fprintf (stderr, "Error reading %s. Read only %zu out of %zu bytes\n", 
                     in_file, *file_size_out, file_size);
    return NULL;
  }
  return buf;
}
