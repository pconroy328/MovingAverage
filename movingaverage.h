/* 
 * File:   movingaverage.h
 * Author: pconroy
 *
 * Created on January 19, 2015, 8:46 AM
 * 16-Dec-2015 - Just adding a comment so I can test out AWS Code Commit from Netbeans
 */

#ifndef MOVINGAVERAGE_H
#define	MOVINGAVERAGE_H

#ifdef	__cplusplus
extern "C" {
#endif

//
// Going to use Troy Hanson's Excellent LinkedList Macros
#include <stdio.h>
#include <UtHash/utlist.h>

#ifndef     FALSE
# define    FALSE       0
# define    TRUE        (!FALSE)
#endif

#define     MASUCCESS   0
#define     MAFAILURE   1


/*
 * You can use any structure with these macros, as long as the structure contains a next pointer. 
 * If you want to make a doubly-linked list, the element also needs to have a prev pointer.
 */

typedef struct BufferElement {
    double                  value;
    struct BufferElement    *next;  /* needed for singly- or doubly-linked lists */
    
    unsigned    long        sequence;   // debugging
} BufferElement_t;



typedef struct MovingAverageStruct {
    BufferElement_t     *head;
    int                 maxElements;                // total size of the ring buffer (eg. 10))
    int                 elementCount;               // how many elements (values) are in the buffer. Will range from [0 to maxElements ]
    int                 index;                      // next spot to use for a new value in the buffer [ 0.. maxE ]]
    double              runningSum;                 // we keep a running total
    double              currentAverage;
} MovingAverage_t;


extern  MovingAverage_t     *MovingAverage_CreateMovingAverage( int maxElements );
extern  int             MovingAverage_AddValue( MovingAverage_t *maPtr, double value );
extern  double          MovingAverage_GetAverage( MovingAverage_t *maPtr );
extern  int             MovingAverage_GetElementCount( MovingAverage_t *maPtr );
extern  int             MovingAverage_Reset( MovingAverage_t *maPtr );
extern  int             MovingAverage_DestroyMovingAverage( MovingAverage_t *maPtr );
extern  int             MovingAverage_Resize( MovingAverage_t *maPtr, int newMaxElements );
extern  int             MovingAverage_GetValues( MovingAverage_t *maPtr, double returnValues[] );
extern  void            MovingAverage_DebugDump( FILE *fp, MovingAverage_t *maPtr );




#ifdef	__cplusplus
}
#endif

#endif	/* MOVINGAVERAGE_H */

