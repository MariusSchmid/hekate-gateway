#ifndef E3479528_F4CE_400C_8130_F62244765195
#define E3479528_F4CE_400C_8130_F62244765195
#include <stdint.h>


#define ENSURE(val) \
    if (!val)       \
        return;

#define ENSURE_RET(val, ret) \
    if (!val)                \
        return ret;


void hekate_utils_remove_character(char* s, char c);

void hekate_utils_strcpy_s(char* dst, uint32_t dst_size, char *source, uint32_t source_size);

void hekate_utils_strremove(char *str, const char *sub);

#endif /* E3479528_F4CE_400C_8130_F62244765195 */
