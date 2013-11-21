//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// NNN     NN  PPPPPPP  EEEEEE
// NN NN   NN  PP    PP EE
// NN  NN  NN  PPPPPPP  EEEEE
// NN   NN NN  PP       EE
// NN     NNN  PP       EEEEEEE
//
// Copyright © 2008 by North Pole Engineering, Inc.  All rights reserved.
// Printed in the United States of America.  Except as permitted under the
// United States Copyright Act of 1976, no part of this software may be
// reproduced or distributed in any form or by any means, without the prior
// written permission of North Pole Engineering, Inc., unless such copying is
// expressly permitted by federal copyright law.
//
// Address copying inquires to:
// North Pole Engineering, Inc.
// Attn: Joe Meyer
// 221 North 1st Street Suite 310
// Minneapolis, Minnesota 55401
//
// Information contained in this software has been created or obtained by North
// Pole Engineering, Inc. from sources believed to be reliable. However, North
// Pole Engineering, Inc. does not guarantee the accuracy or completeness of the
// information published herein nor shall North Pole Engineering, Inc. be liable
// for any errors, omissions, or damages arising from the use of this software.
//
//
//	MODULE:
//    ClMalloc.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    05/22/2008
//
//  DESCRIPTION:
//    Memory allocation functions.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(DEBUG_MEM) && !defined(HUMMINBIRD)
#include <crtdbg.h>
#endif

#include "ClMalloc.h"
#include "ClAssert.h"
#include "error.h"

//**********************************************************************
// Parameters
//**********************************************************************
#define MIN_HEAP_SIZE ( sizeof(Header)<<1 )

#if defined( DEBUG_MEM )

#define LOC_NAME_LEN 16
#define PADDING_LEN 32
#define ALLOC_SIG_LEN 10
#define ALLOC_SIG "cl_malloc"

#endif // defined( DEBUG_MEM )

//**********************************************************************
// Memory debugging switches
//**********************************************************************
#if defined( DEBUG_MEM )

#if !defined(HUMMINBIRD)
#define ENABLE_DUMP_HEADER                0
#define ENABLE_DUMP_ALLOC_STATS           0
#else
// These must always be zero in the humminbird enviroment.
#define ENABLE_DUMP_HEADER                0
#define ENABLE_DUMP_ALLOC_STATS           0
#endif // HUMMINBIRD

#define ENABLE_FREE_HEAP_CHECK            1

#endif // defined( DEBUG_MEM )

//**********************************************************************
// Macros
//**********************************************************************
#define UNITS_2_BYTES( units ) ( (units) * sizeof(Header) )

//**********************************************************************
// Types
//**********************************************************************

typedef long Align; /* for alignment to long boundary */

union header { /* block header */

  struct {
    union header *ptr; /* next block if on free list */
    unsigned long size; /* size of this block */
#if defined(DEBUG_MEM)
    // Bytes allocated for the data and padding at the end.
    unsigned long allocLength;  
    char acSig[ALLOC_SIG_LEN];
#if !defined(HUMMINBIRD)
    char sFile[LOC_NAME_LEN];
    unsigned long nLine;
    char sFunc[LOC_NAME_LEN];
#endif // !defined(HUMMINBIRD)
    char pad[PADDING_LEN];
#endif // defined(DEBUG_MEM)
  } s;

  Align x; /* force alignment of blocks */
};

typedef union header Header;

//**********************************************************************
// Debug variables and macros.
//**********************************************************************
#if defined(DEBUG_MEM)

/*
  Remove the alias created by ClMalloc.h.
  The cl_xxx_debug functions call cl_malloc and cl_free.
*/
#undef cl_malloc
#undef cl_free

void *cl_malloc(unsigned long nbytes);
void cl_free(void *ap);

#if ENABLE_FREE_HEAP_CHECK
  static void ClCheckFreeHeap(char *op, char *psFile, int iLine, char *psFunc);
#else
#define ClCheckFreeHeap( op,  psFile, iLine, psFunc )
#endif

/*
  Each allocated block of memory is padded.  cl_free_debug()
  wil check to see if the padding was overwritten.  If it was
  DebugBreak() is called to cause execution to stop.
*/
static unsigned char ucBeginPaddingChar = 0xAA;
static unsigned char ucEndPaddingChar = 0xBB;
static unsigned char ucUsedBlockChar = 0xdd;
static unsigned char ucFreeBlockChar = 0xcc;

// Number of blocks in the free block arena chain.
static long freeCount = 0;

/*
  Number of times cl_malloc_debug or cl_free_debug are
  called.  A conditional break point can be set on the call
  counts to debug a specific allocation or free.
*/
static int mallocDebugCallCnt = 0;
static int freeDebugCallCnt = 0;

#if defined(HUMMINBIRD)

void DebugFailed( char *msg, char *op, char *file, int iLine, char *psFunc );


#define DEBUG_FAILED( msg, op, file, line, function ) DebugFailed( msg, op, file, line, function )
#define DEBUG_WARNING( msg, op, file, line, function ) DebugFailed( msg, op, file, line, function )

#else // defined(HUMMINBIRD)

#include <windows.h>
#include <crtdbg.h>

/*
  Stops the program if there is a problem.
*/
#define DEBUG_FAILED( msg, op, file, line, function ) DebugBreak()
/*
  Prints a warning.
*/
#define DEBUG_WARNING( msg, op, file, line, function )

static void ClDumpHeader( Header *pHeader, char *psOp );
static void ClDumpAllocStats( void );

#endif // defined(HUMMINBIRD)


#endif // defined(DEBUG_MEM)


//**********************************************************************
// Function prototypes
//**********************************************************************
static void InitFreeList(void);

//**********************************************************************
// Variables.
//**********************************************************************
/*
  Pointer to heap memory used by cl_malloc and cl_free.
  If the entire head is allocated cl_malloc returns NULL.
*/
static Header *heap = NULL;
/*
  Number of units in the heap.  Each unit is the size
  of a header.
*/
static unsigned long heapSize = 0;


static Header base; /* empty list to get started */
/*
  freep points to the last header checked during the last call
  to cl_malloc;

  The free list is a circular linked list.  The first
  item in the list is Header which is a dummy zero byte free block.

*/
static Header *freep = NULL; /* start of free list */

/*
  Number of allocated units of memory. Each unit is the
  size of a header.  unNumAllocUnits includes the size of allocated
  data and the headers at the beginning of each allocated block.
*/
static unsigned long unNumAllocUnits = 0;

/*
  Stores the maximum amount of memory that is allocated.
  Used during debugging.  Can be compared to heapSize to see
  if there is enough headroom.
*/
static unsigned long ulMaxUnitsAllocated = 0;


//**********************************************************************
// Functions
//**********************************************************************


//**********************************************************************
//  FUNCTION:
//    ClSetHeap
//
//  DESCRIPTION:	
//    Sets the address and the size of the heap.  This function must
//    be called before cl_malloc().  ClSetHeap() will only set the heap
//    the first time it is called.
//
//  Inputs:
//    void *pHeap
//      Location of memory to use for the heap.
//    unsigned long unHeapSize
//      The number of bytes in the heap.
//
//  Outputs:
//    None
//  Returns:
//    CL_TRUE
//      The heap adderess and size were set.
//    CL_FALSE
//      The heap has already been set by a previous call to this function.
//
//  GLOBALS:
//    heap
//    heapSize
//**********************************************************************
CL_BOOL ClSetHeap( void *pHeap, unsigned long unHeapSize )
{
  if( NULL == pHeap || unHeapSize < MIN_HEAP_SIZE || NULL != heap )
   return CL_FALSE;

  heap=(Header *)pHeap;
  heapSize = unHeapSize/sizeof(Header);

  heap->s.size = heapSize;

#if defined(DEBUG_MEM)
  /*
    cl_free() decrements freeCount and call DebugBreak()
    if freeCount is less than 0.  Trick cl_free() into
    thinking it is freeing the last block.
  */
  freeCount = 0;
#endif
  /*
    Add the heap to the free list.
  */
  cl_free((void *)(heap+1));

  /*
    cl_free() will set unNumAllocUnits to heap->s.size.
    unNumAllocUnits must be 0 because nothing is allocated
    at this point.
  */
  unNumAllocUnits = 0;


#if defined(DEBUG_MEM)

#if defined(HUMMINBIRD)
  {
    char msg[80];
    sprintf( msg, "Heap Pointer:\t%p\tHeap Size:\t%-.10lu units\t%-.10lu bytes\n", heap, heapSize, UNITS_2_BYTES(heapSize) );
    msg[sizeof(msg) - 1] = '\0';
    WriteInformationToErrorLog( msg );
  }
#else
  _RPT2( _CRT_WARN, "Heap Size: %lu\t%lu bytes\n", heapSize, UNITS_2_BYTES(heapSize) );
#endif // defined(HUMMINBIRD)

#endif // defined(DEBUG_MEM)

  return CL_TRUE;

}

//**********************************************************************
//  FUNCTION:
//    ClReleaseHeap
//
//  DESCRIPTION:	
//    Returns a pointer to heap memory so that the heap can be freed.
//    
//
//  Inputs:
//    None
//
//  Outputs:
//   void **ppHeap
//    Returns the pointer previously passed to ClSetHeap().  If there is
//    any memory allocated in the heap or ClSetHeap() was not called then
//    ppHeap will return NULL.
//
//  Returns:
//    CL_TRUE
//      No memory is allocated in the heap.  ppHeap returns a pointer to
//      that heap.
//
//    CL_FALSE
//      Memory is allocated from the heap.  ppHeap returns NULL.
//
//  GLOBALS:
//    heap
//    heapSize
//    freep;
//    unNumAllocUnits;
//    freeCount;
//**********************************************************************
CL_BOOL ClReleaseHeap( void **ppHeap )
{
  CL_BOOL bResult = CL_FALSE;

#if defined(DEBUG_MEM)

#if defined(HUMMINBIRD) 
  {
    char msg[80];
    sprintf( msg, "Heap Released.  Pointer:\t%p\tSize:\t%-.10lu units\t%-.10lu bytes\n", heap, heapSize, UNITS_2_BYTES(heapSize) );
    msg[sizeof(msg) - 1] = '\0';
    WriteInformationToErrorLog( msg );
  }
#else
  _RPT2( _CRT_WARN, "Heap Size: %lu\t%lu bytes\n", heapSize, UNITS_2_BYTES(heapSize) );
#endif // defined(HUMMINBIRD)

#endif //defined(DEBUG_MEM)

/*if( NULL == ppHeap )
    return CL_FALSE;

  if( 0 == unNumAllocUnits )
  { */
    /*
      No nemory is allocated in the heap.  It can be freed.
    */
    *ppHeap = heap;

    /*
      Initial the module.
    */
    heap = NULL;
    heapSize = 0;
    freep = NULL;
    unNumAllocUnits = 0;
#if defined(DEBUG_MEM)
    freeCount = 0;
#endif


    InitFreeList();


    bResult = CL_TRUE;
/*}
  else
  {
    *ppHeap = NULL;
    bResult = CL_FALSE;
  } */

  return bResult;

}

//**********************************************************************
//  FUNCTION:
//    ClFreeMemorySize
//
//  DESCRIPTION:	
//    Returns the number of unallocated bytes in the heap.
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//  Returns:
//    The number of free bytes.
//
//  GLOBALS:
//    unNumAllocUnits
//    heapSize
//**********************************************************************
unsigned long ClFreeMemorySize( void )
{
  ClAssert( heapSize >= ( unNumAllocUnits + 1 ) );

  return ( heapSize - unNumAllocUnits - 1 ) * sizeof(Header);
}

//**********************************************************************
//  FUNCTION:
//    InitFreeList
//
//  DESCRIPTION:	
//    Intializes this module's local variables.  Must be called at the
//    beginning of every routine that acceses module variables.
//
//  Inputs:
//    None:
//
//  Outputs:
//    None
//
//  Returns:
//    None
//
//  GLOBALS:
//    base
//    freep
//**********************************************************************
static void InitFreeList(void)
{
  if( NULL == freep ) { /* no free list yet */
    base.s.ptr = freep = &base;
    base.s.size = 0;
  }

}

int cl_chkmalloc (unsigned long nbytes )
{
	return 1;
}

//**********************************************************************
//  FUNCTION:
//    cl_malloc
//
//  DESCRIPTION:	
//    Allocates memory from the heap.
//
//  Inputs:
//    unsigned long nbytes
//      Number of bytes to allocate.
//
//  Outputs:
//    None
//
//  Returns:
//    Success - Returns the pointer to allocated memory.
//    Failure - Returns NULL.
//
//  GLOBALS:
//    freep
//    freeCount
//**********************************************************************
void *cl_malloc(unsigned long nbytes)
{
  Header *p, *prevp;
  unsigned long nunits;
return malloc (nbytes);
  /*
    Calculate the number of allocation units.
    Round up if nunits is not disvisble by the sizeof(Align);
    Add 1 to account for the header at the beginning of a block.
  */
  nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1;

  if( 0 == nbytes || NULL == heap || nunits > heapSize )
    // No heap of nbytes bigger than heap.
    return NULL;

  InitFreeList();

  prevp = freep;

  /*
    Find a free block.
  */
  for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {

    if (p->s.size >= nunits) { /* big enough */
      if (p->s.size == nunits) /* exactly */
      {
#if defined(DEBUG_MEM)
        --freeCount;
#endif
        prevp->s.ptr = p->s.ptr;
      }
      else { /* allocate tail end */
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      /*
        The next call to malloc will start searching from
        the block after prevp;
      */
      freep = prevp;

      unNumAllocUnits += p->s.size;

      ClAssert( unNumAllocUnits <= heapSize );

      ulMaxUnitsAllocated = CL_MAX( ulMaxUnitsAllocated, unNumAllocUnits );

      return (void *)(p+1);
    }

    if (p == freep) /* wrapped around free list */
    {
      return NULL; /* none left */
    }
  }

  
}


//**********************************************************************
//  FUNCTION:
//    cl_free
//
//  DESCRIPTION:	
//    Frees allocated memory.
//
//  Inputs:
//    void *ap
//      Pointer to memory that was returned by cl_malloc().
//
//  Outputs:
//    None
//
//  Returns:
//   None
//
//  GLOBALS:
//    freep
//    freeCount
//**********************************************************************
void cl_free(void *ap)
{
  Header *bp, *p;
  CL_BOOL bIsFree = CL_FALSE;
  {
	  static int n=0;
	char msg[32];
	sprintf (msg,"free %x %i",(long)ap,n++);
	LKMTrace (msg);
  }


  return free (ap);
  bp = (Header *)ap - 1; /* point to block header */

  if( NULL == ap || NULL == heap )
    return;

  InitFreeList();

  /*
    detect overflows and underflows in unNumAllocUnits.
  */
  ClAssert( unNumAllocUnits <= heapSize );

  unNumAllocUnits -= bp->s.size;

  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
  {
    /*
      Abort if a point is being freed twice.  If the pointer
      is in the middle of the free block then this function will
      try to free it.
    */
    if( p == bp )
    {
      bIsFree = CL_TRUE;
      break;
    }

    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break; /* freed block at start or end of arena */
  }

  if( !bIsFree )
  {
#if defined(DEBUG_MEM)
    CL_BOOL mergeLowerBlock = CL_FALSE;
    CL_BOOL mergeUpperBlock = CL_FALSE;
#endif  
    if (bp + bp->s.size == p->s.ptr) /* join to upper nbr */
    {
#if defined(DEBUG_MEM)
      mergeUpperBlock = CL_TRUE;
#endif
      bp->s.size += p->s.ptr->s.size;
      bp->s.ptr = p->s.ptr->s.ptr;
    }
    else
      bp->s.ptr = p->s.ptr;

    if (p + p->s.size == bp) /* join to lower nbr */
    {
#if defined(DEBUG_MEM)
      mergeLowerBlock = CL_TRUE;
#endif
      p->s.size += bp->s.size;
      p->s.ptr = bp->s.ptr;
    }
    else
      p->s.ptr = bp;

#if defined(DEBUG_MEM)
    if( mergeLowerBlock && mergeUpperBlock )
      --freeCount;
    else if( !mergeLowerBlock && !mergeUpperBlock )
      ++freeCount;
#endif
    freep = p;

  }

}


//**********************************************************************
//  FUNCTION:
//    ClDumpHeader
//
//  DESCRIPTION:	
//    Dumps a memory block header's contents to the debug output window.
//    The header can be for a free or allocated block.
//
//  Inputs:
//    Header *header
//      The header to dump.
//     char *psOp
//      A string describing the programs operation when ClDumpHeader is 
//      called.  The operation is shown in the debugger windows.
//      No operation is shown when the argument is NULL.
//
//  Outputs:
//    None
//
//  Returns:
//   None
//
//  GLOBALS:
//    heapSize
//    unNumAllocUnits
//**********************************************************************
#if defined(DEBUG_MEM) && !defined(HUMMINBIRD)
static void ClDumpHeader( Header *header, char *psOp )
{
#if ENABLE_DUMP_HEADER
  char msg[256];

  if( NULL != header )
  {
    OutputDebugStringA( "\n----------------------------------------\n" );

    if( NULL != psOp )
    {
      sprintf( msg, "%s\n", psOp );
      OutputDebugStringA( msg );

      if( 0 == strcmp( "cl_malloc_debug", psOp ) )
        sprintf( msg, "cl_malloc_debug call count: %ld\n", mallocDebugCallCnt );
      else if( 0 == strcmp( "cl_free_debug", psOp ) )
        sprintf( msg, "cl_free_debug call count: %ld\n", freeDebugCallCnt );
      OutputDebugStringA( msg );
    }

    sprintf( msg, "File: %s\n", header->s.sFile );
    OutputDebugStringA( msg );

    sprintf( msg, "Line: %ld\n", header->s.nLine );
    OutputDebugStringA( msg );

    sprintf( msg, "Function: %s\n", header->s.sFunc );
    OutputDebugStringA( msg );

    sprintf( msg, "Size of unit: %ld bytes\n", UNITS_2_BYTES( 1 ) );
    OutputDebugStringA( msg );

    sprintf( msg, "Units: %ld\n", header->s.size );
    OutputDebugStringA( msg );

    sprintf( msg, "Allocation Size( data and padding): %ld bytes\n", header->s.allocLength );
    OutputDebugStringA( msg );

    sprintf( msg, "Allocation Size ( data only ): %ld bytes\n", header->s.allocLength - PADDING_LEN );
    OutputDebugStringA( msg );

    sprintf( msg, "Header address: %p\n", header );
    OutputDebugStringA( msg );

    sprintf( msg, "Data address: %p\n", header + 1 );
    OutputDebugStringA( msg );

  }
#endif
}

#endif // defined(DEBUG_MEM) && !defined(HUMMINBIRD)

//**********************************************************************
//  FUNCTION:
//    ClDumpAllocStats
//
//  DESCRIPTION:	
//    Dumps stats on the amount of free and allocated memory to the 
//    debug output window.
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//   None
//
//  GLOBALS:
//    heapSize
//    unNumAllocUnits
//**********************************************************************
#if defined(DEBUG_MEM) && !defined(HUMMINBIRD)
static void ClDumpAllocStats( void )
{
#if ENABLE_DUMP_ALLOC_STATS
  char msg[256];

  sprintf(
    msg, 
    "Allocated Units:\t%ld (units)\t%ld (bytes)\n",
    unNumAllocUnits, 
    UNITS_2_BYTES( unNumAllocUnits )
  );
  OutputDebugStringA( msg );

  sprintf(
    msg,
    "Free Units:\t%ld (units)\t%ld (bytes)\n",
    heapSize -  unNumAllocUnits, 
    UNITS_2_BYTES( heapSize -  unNumAllocUnits )
  );
  OutputDebugStringA( msg );

  sprintf(
    msg,
    "Total Units:\t%ld (units)\t%ld (bytes)\n",
    heapSize,
    UNITS_2_BYTES( heapSize )
  );
  OutputDebugStringA( msg );

  sprintf(
    msg,
    "Free Chain Block Count:\t%ld\n",
    freeCount
  );
  OutputDebugStringA( msg );
#endif // #if ENABLE_DUMP_ALLOC_STATS
}
#endif // defined(DEBUG_MEM) && !defined(HUMMINBIRD)

//**********************************************************************
//  FUNCTION:
//    ClDumpFreeList
//
//  DESCRIPTION:	
//    Dumps all of the headers in the free chain to the debug output
//    window.
//
//  Inputs:
//    Header *header
//      The header to dump.
//     char *psOp
//      A string describing the programs operation when ClDumpHeader is 
//      called.  The operation is shown in the debugger windows.
//      No operation is shown when the argument is NULL.
//
//  Outputs:
//    None
//
//  Returns:
//   None
//
//  GLOBALS:
//    freep
//    base
//**********************************************************************
#if defined(DEBUG_MEM) && !defined(HUMMINBIRD)
static void ClDumpFreeList(void)
{
  Header *pHeader = NULL;
  unsigned long unHeaderCount = 0;
  char msg[128];


  InitFreeList();

  if( NULL == freep )
  {
    sprintf( msg, "Free list does not exist.\n" );
    OutputDebugStringA( msg );
  }
  if( NULL == heap )
  {
    sprintf( msg, "Heap does not exist.\n" );
    OutputDebugStringA( msg );
  }
  sprintf( msg, "Header Size: %ld\n",sizeof( Header) );
  OutputDebugStringA( msg );
  sprintf( msg, "Heap Size: %ld units\t%ld bytes\n", heapSize, UNITS_2_BYTES(heapSize) );
  OutputDebugStringA( msg );
  sprintf( msg, "Heap Start: %p\n", heap );
  OutputDebugStringA( msg );
  sprintf( msg, "Heap End: %p\n", (char *)(heap + heapSize) - ( (heapSize) ? 1 : 0) );
  OutputDebugStringA( msg );
  sprintf( msg, "cl_malloc Search Start:\t%p\n", freep->s.ptr );
  OutputDebugStringA( msg );
  sprintf( msg, "cl_malloc Search End:\t%p\n", freep );
  OutputDebugStringA( msg );

  sprintf( msg, "Free List\n----------\n\n" );
  OutputDebugStringA( msg );

  pHeader = &base;
  do 
  {
    ++unHeaderCount;
    sprintf( msg, "Header Index:\t%ld\nStart Address:\t%p\nLast Address:\t%p\nsize:  \t%ld units\t%ld bytes\nNext:   \t%p\n\n", 
      unHeaderCount,
      pHeader,
      (char *)(pHeader + pHeader->s.size) - ( (pHeader->s.size) ? 1 : 0),
      pHeader->s.size,
      UNITS_2_BYTES(pHeader->s.size),
      pHeader->s.ptr
    );
    OutputDebugStringA( msg );

    pHeader = pHeader->s.ptr;

  } while( pHeader != &base );

}


#endif // defined(DEBUG_MEM) && !defined(HUMMINBIRD)

#if defined(DEBUG_MEM) && defined(HUMMINBIRD)
void DebugFailed( char *msg, char *op, char *file, int iLine, char *psFunc )
{
  char sStr[80];

  sprintf( sStr, "%s: %s\n", op, msg ); 
  sStr[sizeof(sStr) - 1] = '\0';

  WriteToErrorLog( sStr , (file), (iLine) );

  sprintf(
    sStr, 
    "Allocated Units:\t%ld (units)\t%ld (bytes)\n",
    unNumAllocUnits, 
    UNITS_2_BYTES( unNumAllocUnits )
  );
  WriteInformationToErrorLog( sStr );


  sprintf(
    sStr,
    "Free Units:\t%ld (units)\t%ld (bytes)\n",
    heapSize -  unNumAllocUnits, 
    UNITS_2_BYTES( heapSize -  unNumAllocUnits )
  );
  WriteInformationToErrorLog( sStr );

  sprintf(
    sStr,
    "Total Units:\t%ld (units)\t%ld (bytes)\n",
    heapSize,
    UNITS_2_BYTES( heapSize )
  );
  WriteInformationToErrorLog( sStr );

  sprintf(
    sStr,
    "Free Chain Block Count:\t%ld\n",
    freeCount
  );
  WriteInformationToErrorLog( sStr );
}
#endif // defined(DEBUG_MEM) && defined(HUMMINBIRD)
//**********************************************************************
//  FUNCTION:
//    cl_malloc_debug
//
//  DESCRIPTION:	
//    Debug wrapper that calls cl_malloc.  Padding is added before and 
//    after the requested block of memory.  Byte patterns are written to
//    the padding to detect invalid memory access.  The location of the 
//    allocation is written to the beginning of the buffer.
//
//  Inputs:
//    unsigned long unByteCount
//      Number of bytes to allocate.
//    char *psFile
//      The file that this function was called from.  Can be NULL.
//    int nLine
//      The line that this function was called from.
//    char *psFunc 
//      The function that called cl_malloc.  Can be NULL.
//
//  Outputs:
//    None
//
//  Returns:
//   None
//
//  GLOBALS:
//    ALLOC_SIG_LEN
//    ALLOC_SIG
//    LOC_NAME_LEN
//    PADDING_LEN
//    unNumAllocUnits
//**********************************************************************
#if defined(DEBUG_MEM)
void *cl_malloc_debug(unsigned long unByteCount, char *psFile, int iLine, char *psFunc )
{
  unsigned char *pMem = NULL; // Returns a point to allocated memory.
  Header *header = NULL;
  char *psFileName = psFile;
  unsigned long allocLength;

  ++mallocDebugCallCnt;

  allocLength = unByteCount + PADDING_LEN;

#if defined(DEBUG_MEM)

  /*
    The heap may not be initialized yet. cl_malloc() will initialize
    the heap.  Check the heap if it has been initialized.
  */
  if( NULL != freep )
    ClCheckFreeHeap( "cl_malloc_debug", psFile, iLine, psFunc);
#endif

  pMem = cl_malloc( allocLength );

#if defined(DEBUG_MEM)
    if( NULL == freep )
      DEBUG_FAILED( "Heap not initialized.", "cl_malloc_debug", psFile, iLine, psFunc );
#endif

  if( NULL == pMem )
    DEBUG_WARNING( "Memory allocation failed.", "cl_malloc_debug", psFile, iLine, psFunc );
  else
  {    

    /*
      Store the location of the allocation.
    */
    header = (Header *)pMem - 1;

    header->s.allocLength = allocLength;

    strncpy( (char *)header->s.acSig, ALLOC_SIG, ALLOC_SIG_LEN );
    header->s.acSig[ ALLOC_SIG_LEN - 1 ] = '\0';
#if !defined(HUMMINBIRD)
    if( NULL != psFile )
    {
      psFileName = strrchr(psFile, '\\' );
      if( NULL == psFileName )
        psFileName = psFile;
      else
        ++psFileName;

      strncpy( header->s.sFile, psFileName, LOC_NAME_LEN );
      header->s.sFile[LOC_NAME_LEN - 1] = '\0';
    }

    header->s.nLine = iLine;
      
    if( NULL != psFunc )
    {
      strncpy( header->s.sFunc, psFunc, LOC_NAME_LEN );
      header->s.sFunc[LOC_NAME_LEN - 1] = '\0';
    }
#endif //!defined(HUMMINBIRD)
    /*
      Put fill patterns in, before, and after the data area.
      The fill patterns and the beginning and end of the allocated memory
      are checked by cl_free().  The data area can be viewed in the debugger
      to see if it has ever been written to.
    */ 
    memset( header->s.pad, ucBeginPaddingChar, PADDING_LEN );
    memset( pMem, ucUsedBlockChar, unByteCount );
    memset( pMem + unByteCount, ucEndPaddingChar, PADDING_LEN );

#if !defined(HUMMINBIRD)
    ClDumpHeader( header, __FUNCTION__ );
    ClDumpAllocStats();
#endif 

  }

  return pMem;
}
#endif // defined(DEBUG_MEM)

//**********************************************************************
//  FUNCTION:
//    ClCheckFreeHeap
//
//  DESCRIPTION:	
//    Scans the free memory arena chain.  The chain must be a circualar
//    loop that begins and ends at base.  The function will call DebugBreak
//    if the chain is broken.
//
//  Inputs:
//    char *psOp
//      cl_malloc or cl_free.
//    char *psFile
//      The file where cl_malloc or cl_free was called.
//    int iLine
//      The line number where cl_malloc or cl_free was called.
//    char *psFunc
//      The name of routine that called cl_malloc or cl_free.
//
//  Outputs:
//    None
//
//  Returns:
//   None
//
//  GLOBALS:
//    base
//    freeCount
//
//**********************************************************************
#if defined(DEBUG_MEM) && ENABLE_FREE_HEAP_CHECK
static void ClCheckFreeHeap(char *psOp, char *psFile, int iLine, char *psFunc)
{
  Header *pHeader = NULL;
  long uni;

  if( freeCount && &base == base.s.ptr ||
      !freeCount && &base != base.s.ptr )
  {
    /*
      freeCount is not zero and the heap is empty.
      or
      freeCount is zero and heap has free blocks
    */
    DEBUG_FAILED( "ClCheckFreeHeap() failed.", psOp, psFile, iLine, psFunc );
  }
  else
  {   
    /*
      Scan the entire free memory arena chain.
      Start from the first free block and stop
      when the base is reached.  freeCount prevents 
      the loop from executing forever when the chain
      is corrupt.
    */
    for( 
      uni = 0, pHeader = base.s.ptr; 
      uni < freeCount && pHeader != &base; 
      ++uni, pHeader = pHeader->s.ptr
    )
    {
      if( pHeader->s.ptr != &base && 
          (pHeader + pHeader->s.size > pHeader->s.ptr)
      )
      {
#if !defined(HUMMINBIRD)
        ClDumpFreeList();
#endif
        DEBUG_FAILED( "ClCheckFreeHeap() failed.", psOp, psFile, iLine, psFunc );
      }
    }

    if( uni != freeCount || pHeader != &base )
    {
#if !defined(HUMMINBIRD)
      ClDumpFreeList();
#endif
      // freeCount is to big or too small == corrupt arena chain.
      DEBUG_FAILED( "ClCheckFreeHeap() failed.", psOp, psFile, iLine, psFunc );
    }
  }

  
}
#endif // defined(DEBUG_MEM)

//**********************************************************************
//  FUNCTION:
//    cl_free_debug
//
//  DESCRIPTION:	
//    Debug wrapper that calls cl_free.  Padding at the beginning and end
//    of the allocated memory is checked for overwrites.  DebugBreak()is 
//    called if there was an overwrite.
//
//  Inputs:
//    void *p
//      Memory address previously returned by cl_malloc_debug.
//
//  Outputs:
//    None
//
//  Returns:
//   None
//
//  GLOBALS:
//    ALLOC_SIG_LEN
//    ALLOC_SIG
//    LOC_NAME_LEN
//    PADDING_LEN
//    freeCount
//**********************************************************************
#if defined(DEBUG_MEM)
void cl_free_debug( void *p, char *psFile, int iLine, char *psFunc )
{
  unsigned char *pStart = NULL;
  unsigned char *pEnd = NULL;
  unsigned char *pByte = NULL;
  Header *header = NULL;

  ++freeDebugCallCnt;

  if( NULL != p )
  {

    header = (Header *)p - 1;

#if !defined(HUMMINBIRD)
    ClDumpHeader( header , __FUNCTION__ );
#endif

    if( 0 != strncmp(header->s.acSig, ALLOC_SIG, ALLOC_SIG_LEN ) )
       DEBUG_FAILED( "Bad header signature", "cl_free_debug", psFile, iLine, psFunc );

    /*
      Check the begin padding for overwrites.
    */
    pStart = header->s.pad;
    pEnd = header->s.pad + PADDING_LEN;
    for( pByte = pStart; pByte < pEnd; ++pByte )
      if( ucBeginPaddingChar != *pByte )
      {
        DEBUG_FAILED( "Buffer underflow." ,"cl_free_debug", psFile, iLine, psFunc );
        break;
      }
        

    /*
      Check the end padding for overwrites.
    */
    pEnd = (unsigned char*)(header + 1 ) + header->s.allocLength;
    pStart = pEnd - PADDING_LEN;
    for( pByte = pStart; pByte < pEnd; ++pByte )
      if( ucEndPaddingChar != *pByte )
      {
        DEBUG_FAILED( "Buffer overflow.", "cl_free_debug", psFile, iLine, psFunc );
        break;
      }
  }

#if defined(DEBUG_MEM)
  if( NULL == freep )
    DEBUG_FAILED( "Heap not initialized.", "cl_free_debug", psFile, iLine, psFunc );
  else
    ClCheckFreeHeap( "cl_free_debug", psFile, iLine, psFunc);
#endif

  cl_free( p );

#if !defined(HUMMINBIRD)
  ClDumpAllocStats();
#endif

}


#endif // defined(DEBUG_MEM)

