#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include "human_readable_size.c"
#include "human_readable_permissions.c"

typedef struct
{
  char permissions[11];
  char *links;
} LS_ROW;

int main(int argc, char *argv[])
{
  const char *dirToOpen = ".";
  if (argc > 1)
  {
    dirToOpen = argv[1];
  }

  printf("Dir to list: %s\n", dirToOpen);
  struct dirent *de;
  DIR *dr = opendir(dirToOpen);

  if (dr == NULL)
  {
    printf("Could not open current directory");
    return EXIT_FAILURE;
  }

  struct stat st;

  size_t numFiles = 0;
  size_t maxLinksLength = 0;
  size_t maxOwnerLength = 0;
  size_t maxGroupLength = 0;
  size_t maxSizeLength = 0;

  // Get num files
  while ((de = readdir(dr)) != NULL)
  {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
    {
      continue;
    }
    numFiles++;
  }
  printf("total %d\n", numFiles);
  closedir(dr);

  LS_ROW *rows = calloc(numFiles, sizeof(LS_ROW));

  dr = opendir(dirToOpen);
  while ((de = readdir(dr)) != NULL)
  {
    size_t full_path_len = strlen(dirToOpen) + 1 + strlen(de->d_name) + 1;
    char *full_path = malloc(full_path_len);
    strcpy(full_path, dirToOpen);
    // Append a directory separator if necessary (example for Unix/Linux)
    if (dirToOpen[strlen(dirToOpen) - 1] != '/')
    {
      strcat(full_path, "/");
    }
    strcat(full_path, de->d_name);
    // printf("test");
    if (stat(full_path, &st) == 0)
    {
      if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
      {
        continue;
      }

      // Links max length for formatting
      char buffer[10];
      int linkLength = snprintf(buffer, sizeof(buffer), "%lld", st.st_nlink);
      if (linkLength > maxLinksLength)
        maxLinksLength = linkLength;

      // Owner max length for formatting
      struct passwd *pwd = getpwuid(st.st_uid);
      if (strlen(pwd->pw_name) > maxOwnerLength)
      {
        maxOwnerLength = strlen(pwd->pw_name);
      }

      // Group max length for formatting
      struct group *grp = getgrgid(st.st_gid);
      char *group = grp == NULL ? "unknown" : grp->gr_name;
      if (strlen(grp->gr_name) > maxGroupLength)
      {
        maxGroupLength = strlen(grp->gr_name);
      }

      // Size max length for formatting
      long long size_bytes = st.st_size;
      char size_buff[20];
      char *humanReadableSize = human_readable_size(size_bytes, size_buff);
      if (strlen(humanReadableSize) > maxSizeLength)
      {
        maxSizeLength = strlen(humanReadableSize);
      }
    }

    free(full_path);
  }
  closedir(dr);

  int currFileIndex = 0;
  dr = opendir(dirToOpen);
  while ((de = readdir(dr)) != NULL)
  {
    size_t full_path_len = strlen(dirToOpen) + 1 + strlen(de->d_name) + 1;
    char *full_path = malloc(full_path_len);
    if (full_path == NULL)
    {
      perror("full path malloc failed");
      exit(EXIT_FAILURE);
    }

    strcpy(full_path, dirToOpen);
    // Append a directory separator if necessary (example for Unix/Linux)
    if (dirToOpen[strlen(dirToOpen) - 1] != '/')
    {
      strcat(full_path, "/");
    }
    strcat(full_path, de->d_name);

    if (stat(full_path, &st) == 0)
    {
      if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
      {
        continue;
      }

      int mode = st.st_mode & 07777;
      char permissions[11];
      human_readable_permissions(mode, permissions);
      strcpy(rows[currFileIndex].permissions, permissions);
      // printf("Permissions struct value: %s\n", rows[0].permissions);

      struct passwd *pwd = getpwuid(st.st_uid);
      char *owner = pwd == NULL ? "unknown" : pwd->pw_name;

      struct group *grp = getgrgid(st.st_gid);
      char *group = grp == NULL ? "unknown" : grp->gr_name;

      time_t *lastModified = &st.st_mtime;
      char *lastModifiedString = lastModified == NULL ? "unknown" : ctime(lastModified);
      lastModifiedString[strlen(lastModifiedString) - 1] = '\0'; // remove new line from ctime()

      long long size_bytes = st.st_size;
      char size_buff[20];
      char *humanReadableSize = human_readable_size(size_bytes, size_buff);

      printf(
          "%s %-*lu %-*s %-*s %-*s %s %s \n",
          permissions,
          maxLinksLength,
          st.st_nlink,
          maxOwnerLength,
          owner,
          maxGroupLength,
          group,
          maxSizeLength,
          humanReadableSize,
          lastModifiedString,
          de->d_name);
      currFileIndex++;
    }
    else
    {
      printf("Failed to stat: %s\n", full_path);
    };

    free(full_path);
  }

  printf("test Permissions struct value: %s\n", rows[0].permissions);
  for (int i = 0; i < numFiles; i++)
  {
    if (rows[i].permissions != NULL)
    {
      printf("%s\n", rows[i].permissions);
    }
  }

  closedir(dr);
  free(rows);

  return EXIT_SUCCESS;
}
