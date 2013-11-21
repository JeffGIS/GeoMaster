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


// This demonstrates how to derive a simple reader.  This class reads raw
// files (BIP), with the colorspace, dimension, etc, passed in via the
// constructor.  To make the example simple, we do not supplort duplicate(),
// metadata, background/nodata pixels, we only support 8-bit samples, etc.


#include "support.h"

#include "lt_fileSpec.h"
#include "lt_ioFileStream.h"
#include "lti_imageReader.h"
#include "lti_pixel.h"
#include "lti_scene.h"
#include "lti_sceneBuffer.h"
#include "lti_bbbImageWriter.h"
#include "lti_utils.h"

LT_USE_NAMESPACE(LizardTech);

#ifdef WIN32_DLL_WORKAROUND
template <class T>
class LTIDLLFileStream : public T
{
public:
   LTIDLLFileStream() {}
   virtual ~LTIDLLFileStream() {}
};
#endif

//---------------------------------------------------------------------------
// reader for simple BIP raw files
//---------------------------------------------------------------------------

class MyReader : public LTIImageReader
{
   LTI_REFERENCE_COUNTED_BOILERPLATE(MyReader);
public:
   LT_STATUS initialize(const LTFileSpec& fileSpec,
                        const LTIPixel& pixelProps,
                        lt_uint32 width,
                        lt_uint32 height);
   lt_int64 getPhysicalFileSize(void) const;

      
   static LT_STATUS
   readLineBIP(lt_uint8* rowBuffer,
               lt_uint32 rowBytes,
               LTIOStreamInf* stream,
               LTISceneBuffer& bufferData,
               lt_int32 ulx,
               lt_int32 uly)
   {
      LT_STATUS sts = LT_STS_Uninit;
      
      const LTIPixel& pixel = bufferData.getPixelProps();
      const lt_uint32 sceneWidth = bufferData.getWindowNumCols();
      const lt_uint32 sceneHeight = bufferData.getWindowNumRows();
      const lt_uint32 bytesPerPixel = pixel.getNumBytes();
      const lt_uint32 sceneRowLen = sceneWidth * bytesPerPixel;
      const lt_uint16 numBands = bufferData.getNumBands();
      
      const lt_int64 startPos = (uly * rowBytes) + (ulx * bytesPerPixel);
      
      lt_int64 curPos = startPos;
      
      lt_uint32 y=0;
      for (y=0; y<sceneHeight; y++)
      {
         if (stream)
         {
            sts = stream->seek(curPos, LTIO_SEEK_DIR_BEG);
            if (!LT_SUCCESS(sts)) return sts;
            
            const lt_uint32 cnt = stream->read(rowBuffer, sceneRowLen);
            if (cnt != sceneRowLen) return stream->getLastError();
         }
         
         const lt_uint8* src = (lt_uint8*)rowBuffer;
         
         lt_uint32 col=0;
         lt_uint16 band=0;
         for (col=0; col<sceneWidth; col++)
         {
            for (band=0; band<numBands; band++)
            {
               // "getWindowSample" is not the most efficient way to do this:
               // better to get the data pointer, and move through it yourself
               lt_uint8 *dst = (lt_uint8*)bufferData.getWindowSample(col, y, band);
               *dst = *src;
               ++src;
            }
         }
         
         curPos += rowBytes;
      }
      
      return LT_STS_Success;
   }
   
private:
   virtual LT_STATUS decodeStrip(LTISceneBuffer& stripBuffer, const LTIScene& stripScene);
   
   virtual LT_STATUS
   decodeBegin(const LTIScene&)
   {
      return LT_STS_Success;
   }
   virtual LT_STATUS
   decodeEnd()
   {
      return LT_STS_Success;
   }

#ifdef WIN32_DLL_WORKAROUND
   LTIDLLFileStream<LTIOFileStream> *m_stream;
#else
   LTIOFileStream *m_stream;
#endif

   lt_uint32 m_rowBytes;
   lt_uint8* m_rowBuffer;
};

MyReader::MyReader() :
   LTIImageReader(),
   m_stream(NULL),
   m_rowBuffer(NULL),
   m_rowBytes(0)
{
}

MyReader::~MyReader()
{
   m_stream->close();
   delete m_stream;
   delete[] m_rowBuffer;
}

LT_STATUS
MyReader::initialize(const LTFileSpec& fileSpec,
                     const LTIPixel& pixelProps,
                     lt_uint32 width,
                     lt_uint32 height)
{
   LT_STATUS sts = LTIImageReader::init();
   if (!LT_SUCCESS(sts))
      return sts;
      
   LTIMetadataDatabase *metadata = &LTIOverrideMetadataData::getMetadata();
   
   // only support LTI_DATATYPE_UINT8 or LTI_DATATYPE_UINT16
   if (pixelProps.getDataType() != LTI_DATATYPE_UINT8 &&
       pixelProps.getDataType() != LTI_DATATYPE_UINT16)
   {
      return LT_STS_Failure;
   }
#ifdef WIN32_DLL_WORKAROUND
   m_stream = new LTIDLLFileStream<LTIOFileStream>;
#else
   m_stream = new LTIOFileStream;
#endif
   m_stream->initialize(fileSpec, "rb");
   if (!m_stream)
   {
      return LT_STS_Failure;
   }
   sts = m_stream->open();
   if (!LT_SUCCESS(sts)) 
      return sts;

   sts = setPixelProps(pixelProps);
   if (!LT_SUCCESS(sts)) 
      return sts;
   
   sts = setDimensions(width, height);
   if (!LT_SUCCESS(sts)) 
      return sts;
   
   sts = setDefaultGeoCoord(*this);
   if (!LT_SUCCESS(sts)) 
      return sts;
       
   m_rowBytes = pixelProps.getNumBytes() * getWidth();
   m_rowBuffer = new lt_uint8[m_rowBytes];

   return LT_STS_Success;
}

lt_int64
MyReader::getPhysicalFileSize(void) const
{
   const lt_int64 pos = m_stream->tell();
   m_stream->seek(0,LTIO_SEEK_DIR_END);
   const lt_int64 siz = m_stream->tell();
   m_stream->seek(pos,LTIO_SEEK_DIR_BEG);
   return siz;
}

LT_STATUS
MyReader::decodeStrip(LTISceneBuffer& stripBuffer, 
                      const LTIScene& stripScene)
{
   const lt_int32 ulx = stripScene.getUpperLeftCol();
   const lt_int32 uly = stripScene.getUpperLeftRow();
   const lt_int32 sceneWidth = stripScene.getNumCols();
   const lt_int32 sceneHeight = stripScene.getNumRows();

   const lt_int32 imageHeight = getHeight();

   ASSERT(sceneWidth == stripBuffer.getWindowNumCols());
   ASSERT(sceneHeight == stripBuffer.getWindowNumRows());

   LT_STATUS sts = readLineBIP(m_rowBuffer, m_rowBytes, 
                               m_stream, stripBuffer, ulx, uly);
   if (!LT_SUCCESS(sts)) 
      return sts;

   if(LTIUtils::needsSwapping(getDataType(), LTI_ENDIAN_LITTLE))
      stripBuffer.byteSwap();

   return LT_STS_Success;
}

MyReader* 
MyReader::create()
{
   return new MyReader;
}
//---------------------------------------------------------------------------

void
DerivedImageReader()
{
   LT_STATUS sts = LT_STS_Uninit;

   // read in a raw rgb image, and write it out as grayscale

   // make the reader
   const LTFileSpec fileSpec("data/meg.bip");
   const LTIPixel pixel(LTI_COLORSPACE_RGB, 3, LTI_DATATYPE_UINT8);
   MyReader *reader = MyReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec, pixel, 640, 480);
   ASSERT(LT_SUCCESS(sts));

   // make the BBB writer
   LTIBBBImageWriter writer;
   sts = writer.initialize(reader);
   ASSERT(LT_SUCCESS(sts));

   // set up the output file
   sts = writer.setOutputFileSpec("meg.bip");
   ASSERT(LT_SUCCESS(sts));
   
   const LTIScene scene(0, 0, 640, 480, 1.0);

   // write the scene to the file   
   sts = writer.write(scene);
   ASSERT(LT_SUCCESS(sts));

   // verify we got the right output
   ASSERT(Compare("meg.bip", "data/meg.bip"));
   Remove("meg.bip");
   Remove("meg.hdr");

   reader->release();
   reader = NULL;

   return;
}
