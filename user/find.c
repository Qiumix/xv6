#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#define BUF_SIZE 512

char *fmtname(char *path) {
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}

void find_exec(char *path, char *name, int argc, char **argv) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: failed to open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: failed to stat %s\n", path);
    close(fd);
    return;
  }

  if (read(fd, &de, sizeof(de)) != sizeof(de)) {
    exit(1);
  }

  switch (st.type) {
  case T_FILE:
    if (strcmp(name, de.name) == 0) {
      int pid = fork();
      if (pid == 0) {
        char *temp[20];
        for (int i = 0; i < argc; i++) {
          temp[i] = argv[i];
        }
        temp[argc] = path;
        temp[argc + 1] = name;
        temp[argc + 2] = 0;
        exec(argv[0], temp);
      } else {
        wait((int *)0);
      }
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
          int pid = fork();
          if (pid == 0) {
            char *temp[20];
            for (int i = 0; i < argc; i++) {
              temp[i] = argv[i];
            }
            temp[argc] = buf;
            temp[argc + 1] = 0;
            exec(argv[0], temp);
          } else {
            wait((int *)0);
          }
        }
      } else if (st.type == T_DIR) {
        find_exec(buf, name, argc, argv);
      }
    }
    break;
  }
  close(fd);
}
void find(char *path, char *name) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: failed to open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: failed to stat %s\n", path);
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
        }
      } else if (st.type == T_DIR) {
        find(buf, name);
      }
    }
    break;
  }
  close(fd);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(2, "usage: find [dirname] [filename]");
    exit(1);
  }
  char *path = argv[1];
  char *filename = argv[2];
  if (argc == 3) {
    find(path, filename);
  } else if (strcmp(argv[3], "-exec") == 0 && argc > 4) {
    find_exec(path, filename, argc - 4, argv + 4);
  }
  exit(0);
}