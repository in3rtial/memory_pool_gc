/* gc.h --- Interface du gestionnaire de mémoire.  */

#ifndef GCOBJECT_H
#define GCOBJECT_H

/* Type des objets gérés par le GC.
   Tout objet géré par le GC doit être une structure qui commence de manière
   identique.  */
struct GCobject {
   /* Pointeur sur la structure qui décrit la classe de ces objets.
      Attention, ce champ peut être temporairement modifié pendant le GC,
      donc il ne peut pas être utilisé par la méthode `mark'.  */
   struct GCclass *class;
};

/* Type des classes qui décrivent les objets gérés par le GC.  */
struct GCclass {
   /* Taille (en bytes) des objets de cette classe.  */
   int size;

   /* Méthode de marquage des objets de cette classe.
      Appelée par le GC, elle doit appeler `gc_mark' sur chacun des pointeurs
      qui apparaissent dans l'objet `o'.  */
   void (*mark) (struct GCobject **o);
};

/* Allocation d'un nouvel objet de la classe `c'.  */
struct GCobject **gc_malloc (struct GCclass *c);

/* Fonction de marquage du GC.
   Cette fonction marque tous les objets accessibles depuis `o'.  */
void gc_mark (struct GCobject *o);

struct GCroot {
   /* Ce champ doit pointer sur la variable qui contient une racine.  */
   struct GCobject **ptr;
   /* Ce champ est utilisé par le GC; ne pas toucher.  */
   struct GCroot *next;
};
/* Ajout d'une racine.  */
void gc_protect (struct GCroot *r);
/* Élimination d'une racine.  */
void gc_unprotect (struct GCroot *r);

/* Récupération mémoire.
   Il n'est normalement pas nécessaire de l'appeler explicitement, car elle
   est appelée par gc_malloc au besoin.
   Renvoie le nombre de bytes récupérés.  */
int garbage_collect (void);

/* Fonction de test.  */
struct GCstats {
   int count;			/* Nombre d'objets dans le tas.  */
   int used;			/* Bytes utilisés.  */
   int free;			/* Bytes restants.  */
};
struct GCstats gc_stats (void);

#endif
