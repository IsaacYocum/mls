#include <stdio.h>

char* human_readable_size(long long bytes, char *buf) {
    int i = 0;
    // Units table, using 1024 as the base for binary prefixes (KB, MB, etc.)
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double size = (double)bytes;

    while (size >= 1024 && i < sizeof(units) / sizeof(units[0]) - 1) {
        size /= 1024;
        i++;
    }

    // Format the result into the provided buffer 'buf'
    sprintf(buf, "%.*f%s", i, size, units[i]);
    return buf;
}
