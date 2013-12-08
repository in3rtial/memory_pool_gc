/* gc.c --- Gestionnaire mémoire.  */

#include "gc.h"
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

/* Au lieu d'allouer notre mémoire dynamiquement avec malloc/sbrk/mmap,
   on utilise ici un tas préalloué, de taille fixe.  */
static char heap[32 * 1024 * 1024];


struct GCobject *gc_malloc (struct GCclass *c)
{
   /* On présume que `c' est aligné sur un multiple de 2, pour pouvoir
      utiliser le dernier bit du champ `class' comme "markbit".  */
   assert (!((size_t)c & 1));
   /* ¡!¡ À REMPLIR !¡! */
}

struct GCstats gc_stats (void)
{
   /* ¡!¡ À REMPLIR !¡! */
}

static bool marked (struct GCobject *o)
{
   return (size_t)o->class & 1;
}

void gc_mark (struct GCobject *o)
{
   /* ¡!¡ À REMPLIR !¡! */
}

void gc_protect (struct GCroot *r)
{
   assert (r->next == NULL); /* Pour notre usage, pas celui du mutateur!  */
   /* ¡!¡ À REMPLIR !¡! */
}

void gc_unprotect (struct GCroot *r)
{
   /* ¡!¡ À REMPLIR !¡! */
}

int garbage_collect (void)
{
   /* ¡!¡ À REMPLIR !¡! */
}
