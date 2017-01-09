#pragma once

void uv_error_log(int err, const char* file, int line);

#define UV_ERROR_LOG(err) uv_error_log(err, __FILE__, __LINE__)
