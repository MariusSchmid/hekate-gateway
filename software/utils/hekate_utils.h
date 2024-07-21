#ifndef E3479528_F4CE_400C_8130_F62244765195
#define E3479528_F4CE_400C_8130_F62244765195

#define ENSURE(val) \
    if (!val)       \
        return;

#define ENSURE_RET(val, ret) \
    if (!val)                \
        return ret;

#endif /* E3479528_F4CE_400C_8130_F62244765195 */
