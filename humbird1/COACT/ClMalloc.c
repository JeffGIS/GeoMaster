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

#if defined(DEBUG_MEM) && !defined(HUMMINBIRD)
#include <stdio.h>
#include <crtdbg.h>
#endif

#include "ClMalloc.h"
#include "ClAssert.h"

//**********************************************************************
// Debug variables and defines.
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

#endif //defined(DEBUG_MEM)

typedef long Align; /* for alignment to long boundary */
#define MIN_HEAP_SIZE ( sizeof(Header)<<1 )

#if defined(DEBUG_MEM)

#define LOC_NAME_LEN 16
#define PADDING_LEN 32
#define ALLOC_SIG_LEN 10
#define ALLOC_SIG "cl_malloc"

/*
  Each allocated block of memory is padded.  cl_free_debug()
  wil check to see if the padding was overwritten.  If it was
  DebugBreak() is called to cause execution to stop.
*/
unsigned char ucBeginPaddingChar = 0xfd;
unsigned char ucEndPaddingChar = 0xff;
unsigned char ucUsedBlockChar = 0xdd;
unsigned char ucFreeBlockChar = 0xcc;


#if defined(HUMMINBIRD)
#define DebugBreak() ;
#else

#include <windows.h>

#endif // defined(HUMMINBIRD)

#endif // defined(DEBUG_MEM)

//**********************************************************************
// Types
//**********************************************************************
union header { /* block header */

  struct {
    union header *ptr; /* next block if on free list */
    unsigned long size; /* size of this block */
#if defined(DEBUG_MEM)
    // Bytes allocated for the data and padding at the end.
    unsigned long allocLength;  
    char acSig[ALLOC_SIG_LEN];
    char sFile[LOC_NAME_LEN];
    unsigned long nLine;
    char sFunc[LOC_NAME_LEN];
    char pad[PADDING_LEN];
#endif // defined(DEBUG_MEM)
  } s;

  Align x; /* force alignment of blocks */
};

typedef union header Header;

//**********************************************************************
// Function prototypes
//**********************************************************************

#if !defined(HUMMINBIRD)

static void ClDumpHeader( Header *pHeader, char *psOp );

#endif // !defined(HUMMINBIRD)

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
unsigned long unNumAllocUnits = 0;

/*
  Stores the maximum amount of memory that is allocated.
  Used during debugging.  Can be compared to heapSize to see
  if there is enough headroom.
*/
unsigned long ulMaxUnitsAllocated = 0;


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

#if defined(DEBUG_MEM) && !defined(HUMMINBIRD)
  _RPT1( _CRT_WARN, "HeapSize: %lu\n", heapSize );
#endif

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
//**********************************************************************
CL_BOOL ClReleaseHeap( void **ppHeap )
{
  CL_BOOL bResult = CL_FALSE;

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
//    
//**********************************************************************
void *cl_malloc(unsigned long nbytes)
{
  Header *p, *prevp;
  unsigned long nunits;

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
        prevp->s.ptr = p->s.ptr;
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
//    
//**********************************************************************
void cl_free(void *ap)
{
  Header *bp, *p;
  CL_BOOL bIsFree = CL_FALSE;

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
    if (bp + bp->s.size == p->s.ptr) { /* join to upper nbr */
      bp->s.size += p->s.ptr->s.size;
      bp->s.ptr = p->s.ptr->s.ptr;
    }
    else
      bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) { /* join to lower nbr */
      p->s.size += bp->s.size;
      p->s.ptr = bp->s.ptr;
    }
    else
      p->s.ptr = bp;

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
  char msg[256];

  if( NULL != header )
  {
    OutputDebugStringA( "\n----------------------------------------\n" );

    if( NULL != psOp )
    {
      sprintf( msg, "%s\n", psOp );
      OutputDebugStringA( msg );
    }

    sprintf( msg, "File: %s\n", header->s.sFile );
    OutputDebugStringA( msg );

    sprintf( msg, "Line: %ld\n", header->s.nLine );
    OutputDebugStringA( msg );

    sprintf( msg, "Function: %s\n", header->s.sFunc );
    OutputDebugStringA( msg );

    sprintf( msg, "Units: %ld\n", header->s.size );
    OutputDebugStringA( msg );

    sprintf( msg, "Allocation Size: %ld\n", header->s.allocLength );
    OutputDebugStringA( msg );

    sprintf( msg, "Data address: %p\n", header + 1 );
    OutputDebugStringA( msg );

    sprintf( msg, "Allocated Units:\t%ld\n", unNumAllocUnits );
    OutputDebugStringA( msg );

    sprintf( msg, "Free Units:\t%ld\n", heapSize -  unNumAllocUnits );
    OutputDebugStringA( msg );

    sprintf( msg, "Total Units:\t%ld\n", heapSize );
    OutputDebugStringA( msg );

  }
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
  sprintf( msg, "Heap Size: %ld\n", heapSize );
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
    sprintf( msg, "Header Index:\t%ld\nStart Address:\t%p\nLast Address:\t%p\nsize:  \t%ld\nNext:   \t%p\n\n", 
      unHeaderCount,
      pHeader,
      (char *)(pHeader + pHeader->s.size) - ( (pHeader->s.size) ? 1 : 0),
      pHeader->s.size,
      pHeader->s.ptr
    );
    OutputDebugStringA( msg );

    pHeader = pHeader->s.ptr;

  } while( pHeader != &base );

}


#endif // defined(DEBUG_MEM) && !defined(HUMMINBIRD)

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
void *cl_malloc_debug(unsigned long unByteCount, char *psFile, int nLine, char *psFunc )
{
  unsigned char *pMem = NULL; // Returns a point to allocated memory.
  Header *header = NULL;
  char *psFileName = psFile;
  unsigned long allocLength;

  allocLength = unByteCount + PADDING_LEN;
  pMem = cl_malloc( allocLength );

  if( NULL != pMem )
  {    

    /*
      Store the location of the allocation.
    */
    header = (Header *)pMem - 1;

    header->s.allocLength = allocLength;

    strncpy( (char *)header->s.acSig, ALLOC_SIG, ALLOC_SIG_LEN );
    header->s.acSig[ ALLOC_SIG_LEN - 1 ] = '\0';

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

    header->s.nLine = nLine;
      
    if( NULL != psFunc )
    {
      strncpy( header->s.sFunc, psFunc, LOC_NAME_LEN );
      header->s.sFunc[LOC_NAME_LEN - 1] = '\0';
    }

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
#endif 

  }

  return pMem;
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
//
//**********************************************************************
#if defined(DEBUG_MEM)
void cl_free_debug( void *p )
{
  unsigned char *pStart = NULL;
  unsigned char *pEnd = NULL;
  unsigned char *pByte = NULL;
  Header *header = NULL;

  if( NULL != p )
  {
    header = (Header *)p - 1;

    if( 0 != strncmp(header->s.acSig, ALLOC_SIG, ALLOC_SIG_LEN ) )
       DebugBreak();

    /*
      Check the begin padding for overwrites.
    */
    pStart = header->s.pad;
    pEnd = header->s.pad + PADDING_LEN;
    for( pByte = pStart; pByte < pEnd; ++pByte )
      if( ucBeginPaddingChar != *pByte )
        DebugBreak();
        

    /*
      Check the end padding for overwrites.
    */
    pEnd = (unsigned char*)(header + 1 ) + header->s.allocLength;
    pStart = pEnd - PADDING_LEN;
    for( pByte = pStart; pByte < pEnd; ++pByte )
      if( ucEndPaddingChar != *pByte )
        DebugBreak();
  }

  cl_free( p );

#if !defined(HUMMINBIRD)
    if( NULL != header )
      ClDumpHeader( header , "cl_free" );
#endif

}


#endif // defined(DEBUG_MEM)

