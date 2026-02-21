#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <sys/stat.h>

void human_readable_permissions(mode_t mode, char *out_str);

#endif