#include "hekate.h"
#include "sys_linux.h"
#include "s2conf.h"

extern int sys_main ();

static hekate_hal_t this_hekate_hal;
void hekate_init(hekate_hal_t hekate_hal){
       this_hekate_hal = hekate_hal;
       sys_init(this_hekate_hal.sys_hal);
       sys_main();
       // s2conf_ini();
}