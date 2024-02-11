#include "hekate.h"
#include "s2conf.h"

extern int sys_main ();

//register read and write function
void hekate_init(){
       sys_main();
       // s2conf_ini();
}