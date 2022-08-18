#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

static int add(int x, int y);
static int sub(int x, int y);

const int NAME_SIZE = 10;

static int streq(const char *const s1, const char *const s2) {
  const size_t l1 = strlen(s1);
  if (l1 != strlen(s2)) {
    return 0;
  }
  return strncmp(s1, s2, l1) == 0;
}

int call_cmd(const char *const msg) {
  // format: [<command> (<data-type> <data-size> <data>)*].
  // format: [<command> <int> <int>].
  const size_t len = strlen(msg);
  char *const buf = (char *)malloc(len + 1);
  strncpy(buf, msg, len);

  char *beg = buf, *end = buf;
  char cmdname[NAME_SIZE];
  {
    end = strchr(end, ' ');
    *end++ = '\0';
    strcpy(cmdname, beg);
    beg = end;
  }

  char arg0[10];
  {
    end = strchr(end, ' ');
    *end++ = '\0';
    strcpy(arg0, beg);
    beg = end;
  }

  char arg1[10];
  {
    end = strchr(end, '\r');
    *end++ = '\0';
    strcpy(arg1, beg);
    beg = end;
  }

  free(buf);

  if (streq(cmdname, "add")) {
    return add(atoi(arg0), atoi(arg1));
  } else if (streq(cmdname, "sub")) {
    return sub(atoi(arg0), atoi(arg1));
  }
  return -1;
}

static int add(int x, int y) {
  return x + y;
}

static int sub(int x, int y) {
  return x - y;
}
