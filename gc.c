/* gc.c --- Gestionnaire mémoire.  */

#include "gc.h"
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>


#define HEAPSIZE 33554432


/* Based on the fixed pool memory allocator by Eli Bendersky
 * http://eli.thegreenplace.net/2008/10/17/
 * memmgr-a-fixed-pool-memory-allocator */

/*TYPE DECLARATIONS*/
typedef char byte;
typedef unsigned int position;


/*GLOBAL HEAP*/
static byte pool[HEAPSIZE]; 

/*GLOBAL INDEX OF FREE POSITION*/
static position freep = 0;

/*FORWARD FUNCTION DECLARATIONS*/
void memMove(byte array[],
             unsigned int init, 
             unsigned int final,
             unsigned int size);


/*PAGE SYSTEM----------------------------------------------------------------
 * This is the page system. 
 * Its a simple linked list. 
 * It has globally accessible first element and uses NULL as sentinel.
 * 
 * Each page has a last byte (the right) which is allocated to
 * represent the mark bit (byte in our case). If it isn't marked, the
 * page is destroyed and the next slides left on the last known free position.
 *
 * Operations complexity (only the ones used in here) 
 * ADD -> O(1), we keep a pointer to the last page
 * DEFRAG -> O(n), this is a combination of walking through the list and
 * moving the memory left with memMove (which allows overlay of structures).
 * 
 * DEFRAG is called only in the special case that the memory left on the right
 * is too small to accomodate the next structure.
 */

struct page 
{
    unsigned int left;
    unsigned int right;
    unsigned int size;
    struct page* next;
};
typedef struct page page;


page ANCHOR = {0, 0, 0, NULL};
page* FIRSTPAGE = &ANCHOR;
page* LASTPAGE =  &ANCHOR;


/*SHIFT PAGE LEFT IN MEMORY*/
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

/*GET FREE AVAILABLE MEMORY
 * meaningful only after defrag, otherwise
 * dead objects are conserved 
 */
unsigned int AVAILABLEMEM()
{
    return(HEAPSIZE - freep);
}


void printPage(page* p)
{
    printf("               size             %u\n", (p->size));
    printf(" PAGE          left position    %u\n", (p->left));
    printf("               right position   %u\n", (p->right));
    if((p->next)==NULL) 
    {
        printf("               TERMINAL\n");
    }
    printf("\n");
}

void printAllPages()
{
    page* tmp = FIRSTPAGE;
    int i = 1;
    while(i==1)
    {
        printPage(tmp);
        if((tmp->next) != NULL)
        {
            tmp = (tmp->next);
        }
        else
        {
            i=0;
        }
    }
}
/*ADD PAGE TO PAGE SYSTEM
 * by definition, it is added at the end, so it becomes the lastpage
 */
void addPage(unsigned int size)
{
    
    /* allocate a new page */
    page* newPage = (page*) malloc(sizeof(page));
    
    /* the location will be at the end of the system 
       and we allocate +1 for the markbyte */
    (newPage->left) = freep;
    (newPage->right) = (freep+size+1);
    (newPage->size) = (size+1);
    (newPage->next) = NULL;
    
    /* update the LASTPAGE */
    (LASTPAGE->next) = newPage;
    LASTPAGE = newPage;
    freep = (newPage->right)+1;
    
    
}

/*DEFRAG*/
/* to see if the byte is marked we introduce symbols 
 * that are located at the last byte of the structure.
 * They are reached through pointer arithmetic by
 * the page system and the marking system...
 * 'U' -> unmarked
 * 'M' -> marked
 */
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
   
   /* Get the memory size */
   int memSize = (c->size);
   
   if(HEAPSIZE<memSize)
   {
        printf("NO MEM LEFT, IMMINENT SEGFAULT :)\n");
        return NULL;
   }
   
   /* on first pass, if there isn't enough mem, we defrag */
   if (AVAILABLEMEM() < memSize)
   {
       defrag();
   }
   /* if there still isn't enough memory, we allocate fuckall */
   if (AVAILABLEMEM()< memSize)
   {
       printf("NO MEM LEFT, IMMINENT SEGFAULT :)\n");
       return NULL;
   }
   
   
   
}




/* --------------------------BEGIN-STATS------------------------------------ */

/*RETURNS STATUS OF MEM SYSTEM*/
struct GCstats gc_stats (void)
{
    /* We proceed through the pages */
    int count = -1;
    int usedSize = 0;
    page* tmp = FIRSTPAGE;
    int i = 1;
    while(i==1)
    {
        count++;
        usedSize += (tmp->size);
        if((tmp->next)==NULL)
        {
            i=0;
        }
        else
        {
            tmp = (tmp->next);
        }
    }
    
    struct GCstats r = {count, usedSize, ((HEAPSIZE) - usedSize)}; 
    return r;
}

/*PRETTY PRINT MEMORY STATUS*/
void printStats()
{
    struct GCstats stats = gc_stats();
    printf("\n");
    /* 15 char spacing */
    printf("               objects       %d\n", stats.count);
    printf("HEAP STATUS    used memory   %d\n", stats.used);
    printf("               unused memory %d\n", stats.free);
    printf("\n");
    
}
/* ----------------------------END-STATS------------------------------------ */




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
   
    addPage(250);
    addPage(1000);
    printStats();
    printAllPages();
    

    byte a = '\1';
    printf("%c \n", a);

    
    
    
}