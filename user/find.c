#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#define BUF_SIZE 512
char *fmtname(char *path) {
  static char buf[DIRSIZ];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = '\0';
  return buf;
}

// char *re_build_pre(char *pre, char *cur) {
//   static char buf[BUF_SIZE];
//   int pre_len = strlen(pre);
//   int cur_len = strlen(cur);
//   if (pre_len + cur_len > BUF_SIZE) {
//     return pre;
//   }
//   strcpy(buf, pre);
//   buf[pre_len] = '/';
//   strcpy(buf + pre_len + 1, cur);
//   return buf;
// }

void find(char *path, char *name) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0) {
    return;
  }

  if (fstat(fd, &st) < 0) {
    close(fd);
    return;
  }

  if (read(fd, &de, sizeof(de)) != sizeof(de)) {
    exit(1);
  }

  switch (st.type) {
  case T_FILE:
    if (strcmp(name, de.name) == 0) {
      printf("%s%s\n", path, name);
    }
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      fprintf(2, "find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0 || strcmp(".", de.name) == 0 ||
          strcmp("..", de.name) == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if (stat(buf, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", buf);
        continue;
      }
      if (st.type == T_FILE) {
        if (strcmp(name, de.name) == 0) {
          printf("%s\n", buf);
        } else {
          find(buf, name);
        }
      }
    }
    break;
  }
  close(fd);
}
int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(2, "find usage: find [dirname] [filename]");
    exit(1);
  }
  char *path = argv[1];
  find(path, argv[2]);
  return 0;
}