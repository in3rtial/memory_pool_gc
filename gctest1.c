/* gctest1.c --- Un petit test.  */

#include "gc.h"
#include "stdlib.h"
#include "stdio.h"

//#define XY(x,y) x##y concatenates
#define GC_MALLOC(t, v) struct t *v = (struct t *)gc_malloc(&class_##t)
#define GC_MARK(o) gc_mark((struct GCobject *)(o))
#define GC_ROOT(r, p)				\
   struct GCroot r = { (struct GCobject **)&p, NULL };	\

struct ListInt {
   struct GCclass *class;
   int n;
   struct ListInt *next;
};


void mark_ListInt (struct GCobject *o)
{
   struct ListInt *l = (struct ListInt *)o;
   GC_MARK (l->next);
}


struct GCclass class_ListInt = { sizeof (struct ListInt), mark_ListInt };


struct ListInt* cons (int car, struct ListInt *cdr)
{
   GC_MALLOC (ListInt, l);
   l->n = car;
   l->next = cdr;
   return l;
}

void print_stats (void)
{
   struct GCstats before = gc_stats ();
   int freed = garbage_collect ();
   struct GCstats after = gc_stats ();
   printf ("Count = %d -> %d; Used = %d -> %d; Free = %d -> %d\n",
	   before.count, after.count,
	   before.used, after.used,
	   before.free, after.free);
   if (freed < 0
       || before.free + freed != after.free
       || before.used != after.used + freed
       || (freed == 0
	   ? before.count == after.count
	   : before.count > after.count))
      printf ("GC's stats inconsistency!\n");
}

int main (int narg, char **args)
{
   struct ListInt *l1 = NULL, *l2 = NULL;
   int i;
   GC_ROOT (r1, l1);
   GC_ROOT (r2, l2);
   gc_protect (&r1);
   gc_protect (&r2);
   print_stats ();
   for (i = 0; i < 100; i++)
      l1 = cons (i, l1);
   print_stats ();
   l2 = l1;
   for (i = 0; i < 100; i++)
      l1 = cons (i, l1);
   for (i = 0; i < 100; i++)
      l2 = cons (i, l2);
   print_stats ();
   l1->next = l1;
   print_stats ();
   return 0;
}
