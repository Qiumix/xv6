#include <kernel/types.h>
#include <user/user.h>
#include <kernel/fcntl.h>

const char *sep = "\r\t\n-.,/";

int is_sep(char c) { return strchr(sep, c) != 0; }

int is_num(char c) { return c >= '0' && c <= '9'; }

void sixfive(int fd) {
  char c;
  int n;
  char num_buf[32];
  int num_len = 0;
  int is_reading = 0;

  while ((n = read(fd, &c, O_WRONLY)) > 0) {
    if (is_num(c)) {
      if (!is_reading) {
        num_len = 0;
        is_reading = 1;
      }
      if (num_len < sizeof(num_buf) - 1) {
        num_buf[num_len++] = c;
      }
    } else {
      if (is_sep(c)) {
        if (is_reading) {
          num_buf[num_len] = 0;
          int num = atoi(num_buf);
          if (num % 5 == 0 || num % 6 == 0) {
            write(1, num_buf, strlen(num_buf));
          }
          is_reading = 0;
        }
      } else {
        is_reading = 0;
      }
    }
  }

  if (n < 0) {
    fprintf(2, "failed to open the file\n");
    exit(1);
  }

  // judge tail
  if (is_reading) {
    num_buf[num_len] = 0;
    int num = atoi(num_buf);
    if (num % 5 == 0 || num % 6 == 0) {
      write(1, num_buf, strlen(num_buf));
    }
  }
}

int main(int argc, char **argv) {
  int fd, i;

  if (argc <= 1) {
    fprintf(2, "usage: sixfive [file ...]\n");
    exit(1);
  }

  for (i = 1; i < argc; i++) {
    if ((fd = open(argv[i], O_RDONLY)) < 0) {
      printf("sixfive: cannot open %s\n", argv[i]);
      exit(1);
    }
    sixfive(fd);
    close(fd);
  }
  exit(0);
}