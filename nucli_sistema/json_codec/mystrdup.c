#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mystrdup.h"

char *mystrdup(const char *s)
{
    if (s)
        return strdup(s);

    return "err";
}
