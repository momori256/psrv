#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

static int add(char *const s, char *const buf);
static int length(const char *const s, char *const buf);

const int NAME_SIZE = 10;

static int streq(const char *const s1, const char *const s2) {
  const size_t l1 = strlen(s1);
  if (l1 != strlen(s2)) {
    return 0;
  }
  return strncmp(s1, s2, l1) == 0;
}

int call_cmd(const char *const msg, char *const result) {
  // format: [<command> <arg string>].
  const size_t len = strlen(msg);
  char *const buf = (char *)malloc(len + 1);
  strncpy(buf, msg, len);

  const char *beg = buf;
  char *end = buf;
  char cmdname[NAME_SIZE];
  {
    end = strchr(end, ' ');
    *end++ = '\0';
    strcpy(cmdname, beg);
    beg = end;
  }

  int ret = -1;
  if (streq(cmdname, "add")) {
    ret = add(end, result);
  } else if (streq(cmdname, "length")) {
    ret = length(end, result);
  } else if (streq(cmdname, "f")) {
    void *handle = dlopen("./lib/libf.so", RTLD_LAZY);
    if (!handle) {
      fprintf(stderr, "dlopen: %s\n", dlerror());
      exit(1);
    }
    dlerror();
    int (*f)(const char *const, char *const) =
        (int (*)(const char *const, char *const))dlsym(handle, "f");
    const char *const err = dlerror();
    if (err != NULL) {
      fprintf(stderr, "dlsym: %s\n", err);
      exit(1);
    }
    ret = f(end, result);
    dlclose(handle);
  }

  free(buf);
  return ret;
}

static int add(char *const s, char *buf) {
  char *beg = s;
  char *end = s;

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

  sprintf(buf, "%d", atoi(arg0) + atoi(arg1));
  return 0;
}

static int length(const char *const s, char *const buf) {
  sprintf(buf, "%llu", (unsigned long long)strlen(s));
  return 0;
}
