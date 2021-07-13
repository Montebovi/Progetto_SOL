#include <errno.h>
#include <stdio.h>
#include "errors.h"

void manageRetValError(int retval){
    if (retval != 0)
        errno = ERR_RET_VAL_ERROR;
}
