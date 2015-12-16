/* 
 * File:   main.c
 * Author: pconroy
 *
 * Created on January 15, 2015, 2:50 PM
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "movingaverage.h"


static  unsigned    long    globalSequenceCounter = 0UL;

//------------------------------------------------------------------------------
//
//  Call this function first to obtain a pointer to a Moving Average structure
//  The buffer will be ready to take new values after this call
MovingAverage_t    *MovingAverage_CreateMovingAverage (int maxElements)
{
    MovingAverage_t    *maPtr = malloc( sizeof( MovingAverage_t ));
    if (maPtr != (MovingAverage_t *) 0) {
        maPtr->head = (BufferElement_t *) 0;
        maPtr->elementCount = 0;
        maPtr->index = 0;
        maPtr->runningSum = 0.0;
        maPtr->currentAverage = 0.0;
        
        maPtr->maxElements = maxElements;               // size of the ring buffer
    }
    
    return maPtr;
}

//------------------------------------------------------------------------------
//
//  Add a new value (of type double) to the list. This will force a recalculation
//  of the average
int     MovingAverage_AddValue (MovingAverage_t *maPtr, double value)
{
    if (maPtr != (MovingAverage_t *) 0) {
        BufferElement_t     *newElement = malloc( sizeof( BufferElement_t ));
        if (newElement != (BufferElement_t *) 0 ) {
            newElement->value = value;
            newElement->sequence = globalSequenceCounter;
            globalSequenceCounter += 1UL;

            //
            // If there's still room in the buffer, if we haven't wrapped around yet
            if (maPtr->elementCount < maPtr->maxElements) {
                LL_APPEND( maPtr->head, newElement );
                maPtr->elementCount += 1;
                maPtr->index += 1;

            } else {
                //
                // Buffer is full, time to wrap around and over write the value at "index"
                //   Where will this new element go? Here:
                int newElementSpot = (maPtr->index % maPtr->maxElements);
                
                // First - advance to the element and get value that's already there
                BufferElement_t     *oldElement = (maPtr->head);
                for (int i = 0 ; i < newElementSpot; i ++)
                    oldElement = oldElement->next;
                
                //
                // Remove the value from the running total
                maPtr->runningSum -= oldElement->value;
                
                //
                // Swap out the old element with the new
                LL_REPLACE_ELEM( maPtr->head, oldElement, newElement );
                
                //
                //  Update the index, but it needs to wrap around
                maPtr->index = ((maPtr->index + 1) % maPtr->maxElements);
                
                //
                // Free up the memory used by the old, discarded element
                free( oldElement );
            }
            
            maPtr->runningSum += value;
            assert( maPtr->elementCount != 0 );
            maPtr->currentAverage = ( maPtr->runningSum / maPtr->elementCount );
                        
        } else {
            return MAFAILURE;
        }
    }
    
    return MASUCCESS;
}

//------------------------------------------------------------------------------
//
//  Return the average of all of the values in the buffer
double  MovingAverage_GetAverage (MovingAverage_t *maPtr)
{
    assert( maPtr->elementCount != 0 );
    return maPtr->currentAverage;
}

//------------------------------------------------------------------------------
//
//  Return how many values are currently in the buffer
int  MovingAverage_GetElementCount (MovingAverage_t *maPtr)
{
    return maPtr->elementCount;
}

//------------------------------------------------------------------------------
//
//  Clear the contents of the list of values, free the memory, reset all sums and counts
//  to zero.  Do NOT free the head of the list, do NOT resize the list
int  MovingAverage_Reset (MovingAverage_t *maPtr)
{
    BufferElement_t *ptr1;
    BufferElement_t *ptr2;
    
    LL_FOREACH_SAFE( maPtr->head, ptr1, ptr2 ) {
        free( ptr1 );
    }
    
    maPtr->elementCount = 0;
    maPtr->currentAverage = 0.0;
    maPtr->index = 0;
    maPtr->runningSum = 0.0;
    
    return MASUCCESS;
}

//------------------------------------------------------------------------------
//
//  Call Reset() to clear and free memory, then free the head ptr and set the list size to
//  zero.  The list should not be reused after this
int     MovingAverage_DestroyMovingAverage (MovingAverage_t *maPtr)
{
    MovingAverage_Reset( maPtr );

    free( maPtr->head );
    maPtr->maxElements = 0;
    
    return MASUCCESS;
}

// -----------------------------------------------------------------------------
int     MovingAverage_Resize (MovingAverage_t *maPtr, int newMaxElements)
{
    //
    // For now - we're only going to allow larger, not smaller
    assert( newMaxElements > maPtr->maxElements );
    
    
    //
    //  If we're increasing the size of the list - that's easy to do!
    if (newMaxElements > maPtr->maxElements) {
        maPtr->maxElements = newMaxElements;
        //
        // We're Not going to adjust the current index
        
    } else {
        //
        // Shrinking is harder! -- Save the "newMaxElement" values, and drop the rest
        //
        
        int numElementsInThere = maPtr->elementCount;
        if (numElementsInThere < newMaxElements) {
            // Good - we haven't filled up past the new max, just set the new Max value
            maPtr->maxElements = newMaxElements;
        } else {
            //
            // We've got to save some of the values and truncate the others
            int numberElementsToSave = newMaxElements;
            
            //
            // We should save the 'last n' values where the last is found by subtracting index
            //
            //
            // An example, say we were storing 5 and now are resizing down to 4
            //  we're at index = 1 
            //  Index = 1 means that's the spot to use for the next store
            //  Which means the last good one is in spot [0]
            //  Counting backwards wee need to save elements at [0], [3], [2] and [1]
            //
            //  If index = 5 then we'd save [4], [3], [2], [1]
            //
            //
            //  Say we're storing 10 and resizing to 4 - index = 1, we save
            //      [0], [9], [8], [7]
            //  If index was 3, we'd save: [2], [1], [0], [9]
            //
            // val = (oldSize - (newSize - 3))
            //          10    -  ( 4 - 3)) = 1
            //          5       (4 -1))) = 2
            
            int startingIndex = 0;
            if (maPtr->index >= newMaxElements)
                startingIndex = maPtr->index - newMaxElements;
            else
                startingIndex = (maPtr->maxElements - (newMaxElements - maPtr->index));

            //printf( "We're Shrinking the Buffer.  New Size: %d, Old Size: %d, Old Index: %d, New Index: %d\n",
            //        newMaxElements, maPtr->maxElements, maPtr->index, startingIndex );
            //printf( "So we're saving " );
            //for (int i = 0; i < newMaxElements; i += 1)
            //    printf( " [%d]  ", ((startingIndex + i) % newMaxElements) );
            //printf( "\n\n" );
            //int foo = 0;
        }
    }
    
    return MASUCCESS;
}

//------------------------------------------------------------------------------
int     MovingAverage_GetValues (MovingAverage_t *maPtr, double returnValues[])
{
    BufferElement_t *ptr;
    int             index = 0;
    
    LL_FOREACH( maPtr->head, ptr ) {
        returnValues[ index ] = ptr->value;
        index += 1;
    }
    
    return maPtr->elementCount;
}

//------------------------------------------------------------------------------
void    MovingAverage_DebugDump (FILE *fp, MovingAverage_t *maPtr)
{
    fprintf( fp, "Current Structure Values\n" );
    fprintf( fp, "  Element Count: %d\n", maPtr->elementCount );
    fprintf( fp, "  Max Elements : %d\n", maPtr->maxElements );
    fprintf( fp, "  Index        : %d\n", maPtr->index );
    fprintf( fp, "  Average      : %f\n", maPtr->currentAverage );
    fprintf( fp, "  Running Sum  : %f\n", maPtr->runningSum );

    BufferElement_t *ptr;
    
    LL_FOREACH( maPtr->head, ptr ) {
        fprintf( fp, "      Seq: %lu  Value: %f\n", ptr->sequence, ptr->value );    
    }
    fprintf( fp, "Global Sequence Counter: %lu\n", globalSequenceCounter );
}



#if 0
int main(int argc, char** argv) 
{
    MovingAverage_t    *aMovingAverage = MovingAverage_CreateMovingAverage( 3 );
    if (aMovingAverage != (MovingAverage_t *) 0) {
        for (int i = 0; i < 10; i += 1) {
            (void) MovingAverage_AddValue( aMovingAverage, (double) i );
        }
    }
    MovingAverage_DebugDump( aMovingAverage );
    
    /*
    double  values[ 10 ];
    int count = MovingAverage_GetValues( aMovingAverage, &values[ 0 ] );
    for (int i = 0; i < count; i += 1)
        printf( "--> [%f]\n", values[ i ] );
    printf( "The moving average is: %f\n", MovingAverage_GetAverage( aMovingAverage ) );
    */
    
    
    
    //
    // Make it bigger
    printf( "Make the buffer bigger.\n" );
    MovingAverage_Resize( aMovingAverage, 20 );
    for (int i = 100; i < 110; i += 1) {
        (void) MovingAverage_AddValue( aMovingAverage, (double) i );
    }
    
    printf( "After making the buffer larger.\n" );
    MovingAverage_DebugDump( aMovingAverage );



    


    //
    // Make it smaller
    printf( "Make the buffer smaller.\n" );
    MovingAverage_Resize( aMovingAverage, 4 );
    for (int i = 0; i < 10; i += 1) {
        (void) MovingAverage_AddValue( aMovingAverage, (double) i );
    }
    printf( "After making the buffer smaller.\n" );
    MovingAverage_DebugDump( aMovingAverage );
    
    
    
    MovingAverage_DestroyMovingAverage( aMovingAverage );
    return (EXIT_SUCCESS);
}
#endif

