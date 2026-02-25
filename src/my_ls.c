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
  nlink_t links;
  char *pOwner;
  char *pGroup;
  char *pSize;
  char lastModified[13];
  char *pName;
} LS_ROW;

int compareByName(const void *a, const void *b)
{
  const LS_ROW *rowA = (const LS_ROW *)a;
  const LS_ROW *rowB = (const LS_ROW *)b;
  return strcmp(rowA->pName, rowB->pName);
}

int main(int argc, char *argv[])
{

  clock_t start = clock();

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

  // Loop through to get max sizes for consistent column lengths
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
      char buffer[32];
      int linkLength = snprintf(buffer, sizeof(buffer), "%lu", st.st_nlink);
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
      char *pGroup = grp == NULL ? "unknown" : grp->gr_name;
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

      rows[currFileIndex].links = st.st_nlink;

      struct passwd *pwd = getpwuid(st.st_uid);
      char *pOwner = pwd == NULL ? "unknown" : pwd->pw_name;
      rows[currFileIndex].pOwner = malloc(strlen(pOwner));
      strcpy(rows[currFileIndex].pOwner, pOwner);

      struct group *grp = getgrgid(st.st_gid);
      char *pGroup = grp == NULL ? "unknown" : grp->gr_name;
      rows[currFileIndex].pGroup = malloc(strlen(pGroup));
      strcpy(rows[currFileIndex].pGroup, pGroup);

      long long size_bytes = st.st_size;
      char size_buff[20];
      char *humanReadableSize = human_readable_size(size_bytes, size_buff);
      rows[currFileIndex].pSize = malloc(strlen(humanReadableSize));
      strcpy(rows[currFileIndex].pSize, humanReadableSize);

      char date_str[13];
      struct tm *tm_info;
      tm_info = localtime(&st.st_mtime);
      strftime(date_str, sizeof(date_str), "%b %d %H:%M", tm_info);
      strcpy(rows[currFileIndex].lastModified, date_str);

      time_t *lastModified = &st.st_mtime;
      char *lastModifiedString = lastModified == NULL ? "unknown" : ctime(lastModified);
      lastModifiedString[strlen(lastModifiedString) - 1] = '\0'; // remove new line from ctime()

      rows[currFileIndex].pName = malloc(strlen(de->d_name));
      strcpy(rows[currFileIndex].pName, de->d_name);
      rows[currFileIndex].pName[strlen(rows[currFileIndex].pName)] = '\0';

      currFileIndex++;
    }
    else
    {
      printf("Failed to stat: %s\n", full_path);
    };

    free(full_path);
  }

  qsort(rows, numFiles, sizeof(LS_ROW), compareByName);

  clock_t beforePrint = clock(); 
  double before_print_time_taken = (double)(beforePrint - start) / CLOCKS_PER_SEC;
  printf("main() took %.6f seconds before printing\n", before_print_time_taken);

  for (int i = 0; i < numFiles; i++)
  {
    printf(
        "%s %-*lu %-*s %-*s %*s %s %s\n",
        rows[i].permissions,
        maxLinksLength,
        rows[i].links,
        maxOwnerLength,
        rows[i].pOwner,
        maxGroupLength,
        rows[i].pGroup,
        maxSizeLength,
        rows[i].pSize,
        rows[i].lastModified,
        rows[i].pName);

    free(rows[i].pOwner);
    free(rows[i].pGroup);
    free(rows[i].pSize);
    free(rows[i].pName);
  }

  closedir(dr);
  free(rows);

  clock_t end = clock(); 
  double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
  printf("main() took %.6f seconds\n", time_taken);

  return EXIT_SUCCESS;
}
