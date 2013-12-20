/* gctest2.c version modifiee de gctest1.c */

#include "gc.h"
#include "stdlib.h"
#include "stdio.h"

struct ListInt {
    struct GCclass *class;
    int n;
    struct ListInt *next;
};

void mark_ListInt(struct GCobject *o)
{
    
}