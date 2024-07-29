#include "hekate_utils.h"
#include <string.h>

void hekate_utils_remove_character(char *s, char c)
{
    int j, n = strlen(s);
    for (int i = j = 0; i < n; i++)
        if (s[i] != c)
            s[j++] = s[i];

    s[j] = '\0';
}

void hekate_utils_strcpy_s(char *dst, uint32_t dst_size, char *src, uint32_t src_size)
{

    strncpy(dst, src, dst_size);
    if (src_size > dst_size)
    {
        dst[dst_size - 1] = '\0';
    }
}

void hekate_utils_strremove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if (len > 0)
    {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL)
        {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
}