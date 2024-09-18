#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Error handler
int error(int src) {
  extern int errno;
  if (errno != 0) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    if (src && errno != ENOENT) {
      close(src);
    }
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  extern int errno;
  errno = 0;

  // Simple error handling if no arguments are specified
  if (argc == 1) {
    fprintf(stderr, "Error: No Arguments Specified\n");
    return -1;
  }

  // TODO: Allow buffering across files
  int buf_size = 4096;

  // Stores the determined output file.
  int output_file = STDOUT_FILENO;

  // Argument Counter
  int c;
  extern char *optarg;
  extern int optind;
  optind = 1;
  // while (c = getopt(argc, argv, "b:") != -1) {
  //   buf_size = atoi(optarg);

  // }

  while ((c = getopt(argc, argv, "o:b:")) != -1) {
    switch (c) {
    case 'o':
      // printf("o argument recieved");
      output_file = open(optarg, O_TRUNC | O_APPEND | O_CREAT | O_RDWR);
      break;
    case 'b':
      buf_size = atoi(optarg);
      if (buf_size <= 0) {
        perror("Invalid buffer size \n");
        return -1;
        break;
      }
    }
  }

  char buf[buf_size];
  int i = optind;
  if (error(output_file))
    return -1;

  // Opening files for reading
  int file_desc;
  int cur_size = buf_size + 1;
  int write_out;

  for (i; i < argc; i++) {
    file_desc = STDIN_FILENO;
    if (argv[i][0] != '-') {
      // printf("Opening Read File %s\n", argv[i]);
      file_desc = open(argv[i], O_RDONLY);
    }
    if (error(file_desc))
      return -1;

    cur_size = buf_size;
    while (cur_size >= buf_size) {
      cur_size = read(file_desc, &buf, buf_size);

      if (cur_size < 0) {
        close(file_desc);
        close(output_file);
        error(0);
        return -1;
      }
      write_out = write(output_file, &buf, cur_size);

      // Partial Writes are being treated as errors
      // They are unwanted behavior
      if (write_out < cur_size) {
        close(write_out);
        if (!error(0)) {
          perror("Error: Partial Write \n");
        }
        return -1;
      }
    }

    if (file_desc != STDIN_FILENO) {
      close(file_desc);
    }
  }
  close(output_file);
  return 0;
}
