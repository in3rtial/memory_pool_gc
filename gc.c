/* gc.c --- Gestionnaire mémoire.  */

#include "gc.h"
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#pragma pack(1)

#define HEAPSIZE 33554432


/* Based on the fixed pool memory allocator by Eli Bendersky
 * http://eli.thegreenplace.net/2008/10/17/
 * memmgr-a-fixed-pool-memory-allocator */

/*TYPE DECLARATIONS*/
typedef char byte;


/*GLOBAL HEAP*/
static byte pool[HEAPSIZE]; 

/*GLOBAL INDEX OF FREE POSITION*/
static position freep = 0;

/*FORWARD FUNCTION DECLARATIONS*/



/*PAGE SYSTEM----------------------------------------------------------------
 * This is the page system. 
 * Its a simple linked list. 
 * It has globally accessible first element and uses NULL as sentinel.
 * 
 * All blocks have originally the same size
 */

struct page 
{
    bool free;
    unsigned int left;
    unsigned int right;
    unsigned int size;
    struct page* before;
    struct page* next;
};
typedef struct page page;


page FIRSTPAGE = {0, 0, 0, NULL};
page LASTPAGE = FIRSTPAGE;


/*SHIFT PAGE IN MEMORY*/
void MV(unsigned int newLeftPosition, 
               page* PAGE, byte pool[])
{
    /*only shift left, since we defrag*/
    assert (newLeftPosition < (PAGE->left));
    
    unsigned int oldPosition = (PAGE->left);
    int offset = (PAGE->left) - newLeftPosition;
    
    (PAGE->left) = newLeftPosition;
    (PAGE->right) -= offset;
    memMove(pool, oldPosition, newLeftPosition,(PAGE->size));
    
}
/*GET FREE MEMORY*/
unsigned int AVAILABLEMEM()
{
    return(HEAPSIZE - freep);
}


/*ADD PAGE TO PAGE SYSTEM*/
void addPage(unsigned int size)
{
    
}

/*DEFRAG*/
void defrag()
{
    
}

void newPage(size_t size)
{
    
}


/*MEMORY FUNCTIONS-----------------------------------------------------------*/

 /*MEMCPY FOR BYTE ARRAY*/
void memMove(byte array[],       //array to manipulate
             unsigned int init,  //initial position
             unsigned int final, //final position
             unsigned int size)  //size of the mem chunk
{
    byte buffer[size];
    int i = 0;
    for(i; i<size; i++)
    {
        int j = i+init;
        buffer[i] = array[j];
        //array[j]='\0';
    }
    i=0;
    for(i; i<size; i++)
    {
        int j = i+final;
        array[j] = buffer[i];
    }
}

/*MEMSET FOR BYTE ARRAY*/
void memSet(byte array[],
            byte value,
            unsigned int position,
            unsigned int size)
{
    int i = 0;
    for(i; i<size; i++)
    {
        array[i+position] = value;
    }
}
    



/*------------------------------------------------------------------*/

struct GCroot ROOTS = {NULL, NULL};




struct GCobject *gc_malloc (struct GCclass *c)
{
   /* On présume que `c' est aligné sur un multiple de 2, pour pouvoir
      utiliser le dernier byte du champ `class' comme "markbit".  */
   assert (!((size_t)c & 1));
   assert ((size_t)c < HEAPSIZE);
   
   
}






/*RETURNS STATUS OF MEM SYSTEM*/
struct GCstats gc_stats (void)
{
    /*Get the global permanent root*/
    struct GCroot *tmp = &ROOTS;
    
    /*We only want count and used memory*/
    int count = 0;
    int used = 0;
    
    /*Iterate through the linked list*/
    while((tmp->next) != NULL)
    {
        ++count;
        struct GCobject *pointee = *(tmp->ptr);
        if(pointee != NULL){
        used +=  pointee->class->size;}
        tmp = (tmp->next);
    }
    
   /*Assign the int values*/
   struct GCstats g = {count, used, ((32 * 1024 * 1024) - used)};
   return (g);
}

/*PRINTS THE MEMORY STATUS*/
void printStats(struct GCstats stats)
{
    printf("LIVING OBJECTS : %d\n", stats.count);
    printf("USED MEMORY SPACE : %d\n", stats.used);
    printf("UNUSED MEMORY SPACE : %d\n", stats.free);
}



static bool marked (struct GCobject *o)
{
   return (size_t)o->class & 1;
}

void gc_mark (struct GCobject *o)
{
   /* ¡!¡ À REMPLIR !¡! */
}

/*
struct GCobject {
   struct GCclass *class;
};

struct GCclass {
   int size;
   void (*mark) (struct GCobject *o);
};
*/

int *intMalloc(char array[], int position)
{
    return (int*) &array[position];
}
    
    

void gc_protect (struct GCroot *r)
{
   assert (r->next == NULL); /* Pour notre usage, pas celui du mutateur!  */
  
}

void gc_unprotect (struct GCroot *r)
{
   /* ¡!¡ À REMPLIR !¡! */
}

int garbage_collect (void)
{
   /* ¡!¡ À REMPLIR !¡! */
}




struct ListInt {
   struct GCclass *class;
   int n;
   struct ListInt *next;
};






void main(void)
{
 /*
    byte x[100];
    
    
    int *p = intMalloc(x,0);
    (*p) = 100;
    
    
    int *q = intMalloc(x,4);
    (*q) = 42;
    memMove(x, 0, 4, 4);
    
    
    int *r = intMalloc(x,8);
    printf("%d\n", *p);
    printf("%d\n", *q);
    
    
    (*r) = (*q) + (*p);
    (*r) *= 2;
    
    int *s = intMalloc(x,12);
    (*s) = 2;
    printf("%d\n", *s);
    
    
 
    printf("%d\n", *p);
    printf("%d\n", *q);
    printf("%d\n", *r);
    
    printf("char = %lu\n", sizeof(char));
    printf("int = %lu\n", (size_t) int);
    */
   
    
    
    
    printf("%d\n", 2);
    /*
    int *f = x[0];
   // *f += 1;
    *f ++;
    printf("%d\n", *f);
    *f ++;
    printf("%d\n", *f);
    *f ++;
    printf("%d\n", *f);
  */


    
    
    
}