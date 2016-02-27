#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define BUF_SIZE 0x1000

int get_app_info(const char *app_name, char *output) {
#if 0
    FILE *f = fopen("/proc/app_info", "r");
    char *buf = calloc(1, BUF_SIZE);

    if (f && buf && fread(buf, 1, BUF_SIZE, f) > 0) {
        char *p = strstr(buf, app_name);
        char *colon = p ? strchr(p, ':') : NULL;
        if (colon) {
            char *end = strchr(colon, '\n');
            if (! end) {
                strcpy(output, colon+1);
            } else {
                strncpy(output, colon+1, end - colon);
            }
        }
    }

    if (buf) free(buf);
#endif
    return 0;
}

