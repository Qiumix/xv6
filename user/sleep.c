#include "kernel/types.h"
#include "user/user.h"
#include "syscall.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(2, "Usage: sleep time\n");
    exit(1);
  }
  int time = atoi(argv[1]) * 10; /* What? */
  pause(time);
  exit(0);
}