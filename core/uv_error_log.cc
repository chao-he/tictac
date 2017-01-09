#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "uv.h"

static const char* unk_err_msg = "Unknown system error";

void uv_error_log(int err, const char* file, int line) {
  if (err >= 0) return ;
  const char* errmsg = uv_strerror(err);
  if (errmsg != NULL
      && strncmp(errmsg, unk_err_msg, strlen(unk_err_msg)) == 0
      && strlen(errmsg) > strlen(unk_err_msg)) {
    ::free ((void*)errmsg);
    fprintf(stderr, "uv unknown error [%d] %s:%d\n", err, file, line);
    return ;
  } 
  const char* errname = uv_err_name(err);
  fprintf(stderr, "uv error [%d] %s %s %s:%d\n", err, errname, errmsg, file, line);
}
