/* gc.c --- Gestionnaire mémoire.  */

#include "gc.h"
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include <stdlib.h>
#pragma pack(1)

#define HEAPSIZE 33554432


#define GC_MALLOC(t, v) struct t *v = (struct t *)gc_malloc(&class_##t)
#define GC_MARK(o) gc_mark((struct GCobject *)(o))
#define GC_ROOT(r, p)               \
   struct GCroot r = { (struct GCobject **)&p, NULL };  \


/*TYPE DECLARATIONS*/
typedef char byte;
typedef unsigned int position;

/*GLOBAL HEAP*/
static byte pool[HEAPSIZE]; 

/*GLOBAL INDEX OF FREE POSITION*/
unsigned int freep = 0;

/*GLOBAL ROOT */
struct GCroot rootAnchor = {NULL, NULL};
struct GCroot* FIRSTROOT = &rootAnchor;
struct GCroot* LASTROOT = &rootAnchor;

/*FORWARD FUNCTION DECLARATIONS*/
void memMove(byte array[],
             unsigned int init, 
             unsigned int final,
             unsigned int size);


/*--------------------------BEGIN-PAGE-SYSTEM-----------------------------
 * DESCRIPTION
 * 
 * This is the page system. It's somewhat a virtual memory system.
 * 
 * It has a linked list of pages, all ordered by allocation (we always
 * allocate at the end, O(1) since we keep a reference to the last page
 * in memory).
 * 
 * It has globally accessible first element and uses NULL as sentinel.
 * 
 * Each page has a last byte (the right) which is allocated to
 * represent the mark bit (byte in our case). Since a mark byte is added, 
 * we don't really give a damn about the size and don't require it to be a
 * multiple of 2...
 * 
 * If during garbage collection, the last byte of the page isn't marked, the
 * page is destroyed and the next slides left on last known free position.
 *
 * 
 * Operations complexity (only the ones used in here) 
 * ADD -> O(1), we keep a pointer to the last page
 * DEFRAG -> O(n), this is a combination of walking through the list and
 * moving the memory left with memMove (which allows overlay of structures).
 * 
 * DEFRAG is called  in the special case that the memory left on the right
 * is too small to accomodate the next structure.
 * 
 * Although this system doesn't have suitable alignment, from a 
 * practical standpoint it has the advantage of no fragmentation (we can 
 * fill the block completely and it is guaranteed to provide almost as much 
 * space as advertised (we add 1 mark byte per object, so the more objects, 
 * the more marks...).
 * 
 * This system will be inefficient when a large quantity of small objects 
 * are used because we keep unique page for each object.
 * 
 * COMPOSITION
 * The system is composed of the following functions:
 * high level
 *      -addPage(size): adds new memory location
 *      -defrag(): goes through the pages and, delete unmarked ones, 
 *          and does the defragmentation.
 * 
 * mid level
 *      -MV(newLeft, PAGE, pool[]): slides page on leftmost available spot
 * 
 * 
 * low level
 *      -memMove(newLeft, OldLeft, Size): like memmove but on byte array
 *      -memSet(Left, Size, Char) : like memset but on byte array
 */

struct page 
{
    unsigned int left;
    unsigned int right;
    unsigned int size;
    struct GCobject * obj;
    struct page* next;
};
typedef struct page page;


page ANCHOR = {0, 0, 0, NULL ,NULL};
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
    (PAGE->obj) = (struct GCobject *) &(pool[(PAGE->left)]);
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


/*ADD PAGE TO PAGE SYSTEM
 * by definition, it is added at the end, so it becomes the lastpage
 */
struct GCobject** addPage(struct GCclass* class)
{
    int size = (class->size);
    
    /* allocate a new page */
    page* newPage = (page*) malloc(sizeof(page));
    
    /* the location will be at the end of the system 
       and we allocate +1 for the markbyte */
    (newPage->left) = freep;
    (newPage->right) = (freep+size+1);
    (newPage->size) = (size+1);
    (newPage->next) = NULL;
    (newPage->obj) = (struct GCobject*) &pool[freep];
    (newPage->obj->class) = class;
    /*(int*) &array[position];*/
    /* update the LASTPAGE */
    (LASTPAGE->next) = newPage;
    LASTPAGE = newPage;
    freep = (newPage->right)+1;
    pool[(newPage->left) + (newPage->size)] = 'U';
    return &(newPage->obj);
}
    
/*DEFRAG*/
/* to see if the byte is marked we introduce symbols 
 * that are located at the last byte of the structure.
 * They are reached through pointer arithmetic by
 * the page system and the marking system...
 * 'U' -> unmarked
 * 'M' -> marked
 * clustefuck quite possible... careful
 * Also need to call updatePointers after to insure completion
 */
void defrag()
{
    /* We start at zero and increment */
    freep = 0;
    
    /* We ignore the first page, its just there*/
    page* tmp_before = FIRSTPAGE;
    page* tmp = (FIRSTPAGE->next);
    int finished = 0;
    
    if(tmp==NULL)
    {
        finished = 1;
    }
    
    while(finished == 0)
    {
        /* get the mark byte */
        int index = (tmp->right);
        
        /* if the byte is marked, we unmark it and slide leftmost */
        if(pool[index] == 'M')
        {
            pool[index] ='U';
            if(freep != (tmp->left))
            {
                
                MV(freep, tmp, pool);
            }
            freep = ((tmp->right)+1);
            tmp_before = tmp;
            tmp = tmp->next;
        }
        
        /* if the byte is unmarked, we delete the structure */
        else if(pool[index] == 'U')
        {
            /* connect after to before */
            page* tmp_next = (tmp->next);
            (tmp_before->next) = tmp_next;
            
            /* free the node and move on */
            free(tmp);
            tmp = tmp_next;
        }
        
        if(tmp == NULL)
        {
            finished = 1;
        }
    }
    
    /* reassign LASTPAGE to the last */
    if(tmp == NULL)
    {
        LASTPAGE = tmp_before;
    }
    else
    {
        LASTPAGE = tmp;
    }
    
}


 /*MEMCPY FOR BYTE ARRAY*/
void memMove(byte array[],       //array to manipulate
             unsigned int init,  //initial position
             unsigned int final, //final position
             unsigned int size)  //size of the mem chunk
{
    byte buffer[size];
    int i = 0;
    for(; i<size; i++)
    {
        int j = i+init;
        buffer[i] = array[j];
        //array[j]='\0';
    }
    i=0;
    for(; i<size; i++)
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
    for(; i<size; i++)
    {
        array[i+position] = value;
    }
}
    

/*-----------------------------END-PAGE-SYSTEM----------------------------*/



/* -----------------------BEGIN-PRINT-FUNCTIONS---------------------------*/
/* PRINT A SINGLE MEMORY PAGE */
void printPage(page* p)
{
    printf("PAGE           size             %u\n", (p->size));
    printf("               left position    %u\n", (p->left));
    printf("               right position   %u\n", (p->right));
    printf("               array location   %p\n", &pool[(p->left)]);
    printf("               pointer location %p\n", (p->obj));
    printf("               mark byte        %c\n", pool[(p->right)]);
    if((p->next)==NULL) 
    {
        printf("               TERMINAL\n");
    }
    printf("\n");
}
/* PRINT ALL REACHABLE MEMORY PAGES */
void printAllPages()
{
    page* tmp = (FIRSTPAGE->next);
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

/* ----------------------END-PRINT-FUNCTIONS------------------------------ */


    
struct GCobject** gc_malloc (struct GCclass *c)
{  
   /* Get the memory size */
   unsigned int memSize = (unsigned int) (c->size);
   
   if(HEAPSIZE<memSize)
   {
        printf("NO MEM LEFT, IMMINENT SEGFAULT :)\n");
        return NULL;
   }
   
   /* on first pass, if there isn't enough mem, we defrag */
   if (AVAILABLEMEM() < memSize)
   {
       garbage_collect();
   }
   /* if there still isn't enough memory, we allocate nothing */
   if (AVAILABLEMEM()< memSize)
   {
       printf("NO MEM LEFT, IMMINENT SEGFAULT :)\n");
       return NULL;
   }
   
   /* if it gets there, we allocate */
   /* (int*) &array[position]; */
   return addPage(c);
}




/* --------------------------BEGIN-STATS---------------------------------- */

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
    printf("HEAP STATUS    objects          %d\n", stats.count);
    printf("               used memory      %d\n", stats.used);
    printf("               available memory %d\n", stats.free);
    printf("\n");
    
}
/* ----------------------------END-STATS---------------------------------- */







/* -----------------------BEGIN-MARKING-FUNCTIONS------------------------- */


void gc_mark (struct GCobject* o)
{
    /* get the size of the class */
    int size = (o->class->size);
    /* cast to size_t, would also protect against negative integers */
    size_t size2 = size;
    /* reach the pointee + 1 byte */
    byte* b = (byte*) (o);
    b+=size2;
    b++;
    /* mark the byte */
    (*b) = 'M';
}



int rootLen()
{
    int i = 0;
    
    int adequate = 1;
    struct GCroot* tmp = FIRSTROOT;
    
    while(adequate)
    {
        if((tmp->next) == NULL)
        {
            adequate = 0;
        }
        else
        {
            i += 1;
            tmp = (tmp->next);
        }
    }
    return i;
}

void gc_protect (struct GCroot *r)
{
    /* we can always add at the end because its next is NULL*/
   assert ((r->next) == NULL);
   assert ((r->ptr) != NULL);
   (LASTROOT->next) = r;
   LASTROOT = r;
}


void gc_markMethod(struct GCobject ** pointed)
{
    (*((*pointed)->class->mark))(pointed);
}


void gc_markAll(void)
{
    struct GCroot* tmp;
    int length = rootLen();
    int i = 0;
    if(length>0)
    {    
        tmp = (FIRSTROOT->next);
        for(; i< length ; i++)
        {
            struct GCobject** pointed = (tmp->ptr);
            assert(pointed != NULL);
            gc_markMethod((struct GCobject**) pointed);
            tmp = (tmp->next);
        }
    }
}


void gc_unprotect (struct GCroot *r)
{
    /* undefined if many roots pointing to the same... we go O(n) 
     * until we meet the equal */
    struct GCroot* tmp_before = FIRSTROOT;
    /* tmp cannot be null... */
    struct GCroot* tmp = (FIRSTROOT->next);
    int adequate = 1;
    while(adequate)
    {
        if(tmp == NULL)
        {
            /* arrived at the end */
            adequate = 0;
        }
        else if(tmp == r)
        {
            /* get the root out of the root system */
            adequate = 0;
            /* get next */
            tmp = (tmp->next);
            /* assign next to before */
            (tmp_before->next) = tmp;

        }
        else
        {
            /* still not there */
            tmp_before = tmp;
            tmp = (tmp->next);
        }
    }
}


int garbage_collect (void)
{
   int start = gc_stats().used;
   gc_markAll();
   defrag();
   int end = gc_stats().used;
   return (start - end);
}

struct GCclass testStruct1 = {250, NULL};
struct GCclass testStruct2 = {1000, NULL};
int testPages(void)
{
    int testPassed = 1;

    addPage(&testStruct1);
    addPage(&testStruct2);

    pool[251] = 'M';
    struct GCstats g = gc_stats();
    
    if(g.count != 2)
    {
        testPassed = 0;
    }
    
    defrag();
    
    g = gc_stats();
    if(g.count != 1)
    {
        testPassed = 0;
    }
    
    defrag();
    
    return testPassed;
}




/* Linked list of integers */
struct ListInt {
   struct GCclass *class;
   int n;
   struct ListInt ** next;
   
};

void mark_ListInt(struct GCobject **o)
{
    /* recursive marking for ListInt
     * the marking function must only be called from the root, which
     * is allocated on the stack, not through gc_malloc, otherwise BOOM
     * We only mark the next object.
     */
    struct ListInt ** l = (struct ListInt**) o;
    
    if(((*l)->next) != NULL)
    {
        gc_mark((struct GCobject *) (*((*l)->next)));
        mark_ListInt((struct GCobject**) (*l)->next);
    }
}

struct GCclass class_ListInt = { sizeof (struct ListInt), NULL };

struct GCclass class_ListInt2 = {sizeof (struct ListInt), &mark_ListInt};

struct ListInt** cons (int car, struct ListInt **cdr)
{
    struct ListInt** l = (struct ListInt**) gc_malloc(&class_ListInt);
    (*l)->n = car;
    (*l)->next = cdr;
    return l;
}


int test_valueAssign(void)
{
    int testPassed = 1;
    struct ListInt** g = (struct ListInt**) gc_malloc(&class_ListInt);
    
    /* try assigning  */
    ((*g)->n)= 25;
    if(((*g)->n) != 25)
    {
        testPassed = 0;
    }
    
    struct GCstats s = gc_stats();
    
    /* verify that one object is counted */
    if(s.count != 1)
    {
        testPassed = 0;
    }
    
    
    /* verify that the object is freed */
    defrag();
    struct GCstats s2 = gc_stats();
    if(s2.count != 0 )
    {
        testPassed = 0;
    }
    
    
    return testPassed;
    

}



void printArray(int begin, int end)
{
    /* low level byte array printing function */
    if( begin >= 0 && end < HEAPSIZE)
    {
        for( ; begin < end; begin ++)
        {
            printf("pool[%d] = %c\n",begin, pool[begin]);
        }
    }
}


int test_defragging(void)
{
    int testPassed = 1;
    struct ListInt** g = (struct ListInt**) gc_malloc(&class_ListInt);
    
    /* try assigning  */
    ((*g)->n)= 25;
    //((*g)->class) = (&class_ListInt);
    
    /* check value */
    if(((*g)->n) != 25)
    {
        testPassed = 0;
    }
    
    /* Mark the structure */
    gc_mark((struct GCobject*)(*g));
    

    /* Test the defrag function */
    defrag();
    
    struct GCstats s = gc_stats();
    if(s.count != 1)
    {
        testPassed = 0;
    }
    
    defrag();
    s = gc_stats();
    if(s.count != 0)
    {
        testPassed = 0;
    }
    
    return testPassed;
}

int testTranslation(void)
{
    /* create 4 objects, mark 2nd and 4th, defrag and query the values */
    int testPassed = 1;
    
    struct ListInt ** l1 = (struct ListInt**) gc_malloc(&class_ListInt);
    struct ListInt ** l2 = (struct ListInt**) gc_malloc(&class_ListInt);
    struct ListInt ** l3 = (struct ListInt**) gc_malloc(&class_ListInt);
    struct ListInt ** l4 = (struct ListInt**) gc_malloc(&class_ListInt);
    
    (*l1)->n = 1;
    (*l2)->n = 2;
    (*l3)->n = 3;
    (*l4)->n = 4;
    
    gc_mark((struct GCobject *)(*l2));
    gc_mark((struct GCobject *)(*l4));
    
    //printAllPages();
    defrag();
    struct GCstats s = gc_stats();
    if(s.count != 2)
    {
        testPassed = 0;
    }
    

    if(((*l2)->n) != 2)
    {
        testPassed = 0;
    }
    if(((*l4)->n) != 4)
    {
        testPassed = 0;
    }
    
    //printAllPages();
    defrag();
    return testPassed;
}




int testRoot(void)
{
    /* mark through the root system */
    int testPassed = 1;
    defrag();
    
    
    /* declare nodes on pool memory */
    struct ListInt** a = (struct ListInt**) gc_malloc(&class_ListInt2);
    struct ListInt** b = (struct ListInt**) gc_malloc(&class_ListInt2);
    struct ListInt** d = (struct ListInt**) gc_malloc(&class_ListInt2);
    struct ListInt** c = (struct ListInt**) gc_malloc(&class_ListInt2);
    
    (*a)->n = 1;
    (*b)->n = 2;
    (*c)->n = 3;
    (*d)->n = 42;
    (*a)->next = b;
    (*b)->next = c;
    
    /* declare root on stack */
    struct ListInt l1 =  {&class_ListInt2, 1000, a};
    struct ListInt l2 = {&class_ListInt2, 1001, d};
    
    /* create a double pointer to l1 */
    struct ListInt * l1_ptr = &l1; 
    struct ListInt ** l1_2ptr = &l1_ptr;
    
    struct ListInt* l2_ptr = &l2;
    struct ListInt** l2_2ptr = &l2_ptr;
    
    /* add the root */
    struct GCroot root = { (struct GCobject **) l1_2ptr, NULL };
    struct GCroot root2 = { (struct GCobject **) l2_2ptr, NULL};
    gc_protect(&root);
    gc_protect(&root2);
    
    
    
    
    
    
    if( (*(a)) != (*(l1.next)))
    {
        printf("Error on addresses in l1\n");
        testPassed = 0;
    }
    
    if( (*(b)) != (*((*a)->next)))
    {
        printf("Error on pointing from a to b\n");
        testPassed = 0;
    }
    
    
    garbage_collect();
    
    //printAllPages();

    
    
   
    if(((*a)->n) != 1)
    {
        testPassed = 0;
    }
    if(((*c)->n) != 3)
    {
        testPassed = 0;
    }
    defrag();
    
    gc_unprotect(&root);
    gc_unprotect(&root2);
    
    if(rootLen() != 0)
    {
        testPassed = 0;
    }
    
    return testPassed;
    
}






int main(void)
{ 
    

    int goOn = 1;
    if(goOn)
    {
        /* page system test */
        if(testPages())
        {
            printf("pages : ok\n");
        }
        else
        {
            goOn = 0;
            printf("PAGES : PROBLEM\n");
        }
    }
    
    if(goOn)
    {
        /* assignment of values test */
        if (test_valueAssign())
        {
            printf("assign : ok\n");
        }
        else
        {
            goOn = 0;
            printf("ASSIGN : PROBLEM\n");
        }
    }
    
    
    if(goOn)
    {
        /* defrag and mark test */
        if(test_defragging())
        {
            printf("defragging : ok\n");
        }
        else
        {
            goOn = 0;
            printf("DEFRAGGING : PROBLEM\n");
        }
        
    }
    
    if(goOn)
    {
        /* object translation in memory pool test */
        if(testTranslation())
        {
            printf("translation : ok\n");
        }
        else
        {
            goOn = 0;
            printf("TRANSLATION : PROBLEM\n");
        }
    }
    
    if(goOn)
    {
        /* general root system test
         *includes protect, unprotect, marking, garbage collection
         */
        if(testRoot())
        {
            printf("root functions : ok\n");
        }
        else
        {
            goOn = 0;
            printf("ROOT FUNCTIONS : PROBLEM\n");
        }
    }
    
    
   return goOn;


   
 
}