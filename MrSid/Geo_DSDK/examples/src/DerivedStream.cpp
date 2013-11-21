/* $Id$ */
/* //////////////////////////////////////////////////////////////////////////
//                                                                         //
// This code is Copyright (c) 2008 LizardTech, Inc, 1008 Western Avenue,   //
// Suite 200, Seattle, WA 98104.  Unauthorized use or distribution         //
// prohibited.  Access to and use of this code is permitted only under     //
// license from LizardTech, Inc.  Portions of the code are protected by    //
// US and foreign patents and other filings. All Rights Reserved.          //
//                                                                         //
////////////////////////////////////////////////////////////////////////// */
/* PUBLIC */


// This demonstrates how to derive your own stream class.  This example
// stream takes a standard FILE* and wraps the LTIOStreamInf stream interface
// around it.


#include "support.h"

#include "lt_base.h"
#include "lt_ioStreamInf.h"

LT_USE_NAMESPACE(LizardTech);


//---------------------------------------------------------------------------
// simple stdio/FILE* stream
//---------------------------------------------------------------------------


class MyStream : public LTIOStreamInf
{
public:
   MyStream();
   LT_STATUS initialize(FILE* fp);
   ~MyStream();

	bool isEOF();
	bool isOpen();
	
	LT_STATUS open();
	LT_STATUS close();
   
   lt_uint32 read(lt_uint8 *pDest, lt_uint32 numBytes);
   lt_uint32 write(const lt_uint8 *pSrc, lt_uint32 numBytes);
   
   LT_STATUS seek(lt_int64 offset, LTIOSeekDir origin);
   lt_int64 tell();
	
	LTIOStreamInf* duplicate();
	
	LT_STATUS getLastError() const;
	
	const char* getID() const;

	// these are only for stdio streams, not from LTIOStreamInf
   int stdio_fflush();
   int stdio_ferror();
   void stdio_clearerr();
   
private:
	FILE* m_file;
};


MyStream::MyStream() :
   m_file(NULL)
{
}


MyStream::~MyStream()
{
}


LT_STATUS
MyStream::initialize(FILE* fp)
{
   m_file = fp;
   return fp ? LT_STS_Success : LT_STS_Failure;
}


bool 
MyStream::isEOF()
{
   return 0 != feof(m_file);
}


bool 
MyStream::isOpen()
{
   return (m_file != NULL);
}


LT_STATUS 
MyStream::open()
{
   return m_file ? LT_STS_Success : LT_STS_Failure;
}


LT_STATUS 
MyStream::close()
{
   return LT_STS_Success;
}


lt_uint32 
MyStream::read( lt_uint8 *pDest, lt_uint32 numBytes )
{
   return (lt_uint32)fread(pDest, sizeof(lt_uint8), numBytes, m_file);
}


lt_uint32 
MyStream::write( const lt_uint8 *pSrc, lt_uint32 numBytes )
{
   return (lt_uint32)fwrite(pSrc, sizeof(lt_uint8), numBytes, m_file);
}


LT_STATUS 
MyStream::seek( lt_int64 offset, LTIOSeekDir origin )
{
   int stdOrigin;
   switch (origin)
   {
      case (LTIO_SEEK_DIR_BEG):
         stdOrigin = SEEK_SET;
         break;
      
      case (LTIO_SEEK_DIR_CUR):
         stdOrigin =  SEEK_CUR;
         break;
      
      case (LTIO_SEEK_DIR_END):
         stdOrigin = SEEK_END;
         break;
      
      default:
         return LT_STS_Failure;
   }
   
   // do the seek
   if (offset > LT_LONG_MAX ||
       offset < LT_LONG_MIN)
   {
      // stdio doesn't support 64-bit files
      return LT_STS_Failure;
   }
   long loffset = (long)offset;

   int stat = ::fseek(m_file, loffset, stdOrigin);

   // return
   if (stat == 0)
   {
      return LT_STS_Success;
   }

   return LT_STS_Failure;
}


lt_int64 
MyStream::tell()
{
   return ftell(m_file);
}


LTIOStreamInf* 
MyStream::duplicate()
{
	// not supported by this class
   return NULL;
}


LT_STATUS
MyStream::getLastError() const
{
   return ferror(m_file);
}


const char*
MyStream::getID() const
{
   return "my_stdio_stream:";
}


int 
MyStream::stdio_fflush()
{
   return fflush(m_file);
}


int
MyStream::stdio_ferror()
{
   return ferror(m_file);
}


void
MyStream::stdio_clearerr()
{
   clearerr(m_file);
}


//---------------------------------------------------------------------------

void
DerivedStream()
{
   LT_STATUS sts = LT_STS_Uninit;

   // read in a file using the stdio stream, then write it
   // to disk the same way
   
#if defined(LT_OS_WIN32) || defined(LT_OS_WIN64)
   const lt_uint32 len=44;
#else
   const lt_uint32 len=40;
#endif
   lt_uint8* buf = new lt_uint8[len];

   {
      FILE* inFP = fopen("data/meg.hdr", "rb");
      MyStream inStream;
      sts = inStream.initialize(inFP);
      ASSERT(LT_SUCCESS(sts));
      sts = inStream.open();
      ASSERT(LT_SUCCESS(sts));
      const lt_uint32 cnt = inStream.read(buf, len);
      ASSERT(cnt==len);
      inStream.close();
      fclose(inFP);
   }

   {
      FILE* outFP = fopen("foo", "wb");
      MyStream outStream;
      sts = outStream.initialize(outFP);
      ASSERT(LT_SUCCESS(sts));
      sts = outStream.open();
      ASSERT(LT_SUCCESS(sts));
      const lt_uint32 cnt = outStream.write(buf, len);
      ASSERT(cnt==len);
      outStream.close();
      fclose(outFP);
   }

   delete[] buf;

   // verify we got the right output
   ASSERT(Compare("foo", "data/meg.hdr"));
   Remove("foo");

   return;
}
