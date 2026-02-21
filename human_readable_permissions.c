#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

void human_readable_permissions(mode_t mode, char *out_str)
{
  // 1. Determine File Type
  if (S_ISDIR(mode))
    out_str[0] = 'd';
  else if (S_ISLNK(mode))
    out_str[0] = 'l';
  else
    out_str[0] = '-';

  // 2. Owner permissions
  out_str[1] = (mode & S_IRUSR) ? 'r' : '-';
  out_str[2] = (mode & S_IWUSR) ? 'w' : '-';
  out_str[3] = (mode & S_IXUSR) ? 'x' : '-';

  // 3. Group permissions
  out_str[4] = (mode & S_IRGRP) ? 'r' : '-';
  out_str[5] = (mode & S_IWGRP) ? 'w' : '-';
  out_str[6] = (mode & S_IXGRP) ? 'x' : '-';

  // 4. Other permissions
  out_str[7] = (mode & S_IROTH) ? 'r' : '-';
  out_str[8] = (mode & S_IWOTH) ? 'w' : '-';
  out_str[9] = (mode & S_IXOTH) ? 'x' : '-';

  out_str[10] = '\0';
}
