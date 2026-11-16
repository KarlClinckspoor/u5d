// Resident code, CS:216c: a module of its own in the original, between assembly modules.

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"
#include "audio/aud_sfx.h"
#include "event/event.h"
#include "time/time.h"
#include <stdlib.h>
#include <time.h>


int ULTIMA_216c_strlen(char* param_1)
{
    int local_4;

    local_4 = 0;
    while (*param_1++ != '\0')
    {
        local_4++;
    }
    return local_4;
}
