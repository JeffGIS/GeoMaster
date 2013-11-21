/* //////////////////////////////////////////////////////////////////////////
//                                                                         //
// This code is Copyright (c) 2009-2009 LizardTech, Inc, 1008 Western      //
// Avenue, Suite 200, Seattle, WA 98104.  Unauthorized use or distribution //
// prohibited.  Access to and use of this code is permitted only under     //
// license from LizardTech, Inc.  Portions of the code are protected by    //
// US and foreign patents and other filings. All Rights Reserved.          //
//                                                                         //
////////////////////////////////////////////////////////////////////////// */     
/* PUBLIC */

#ifndef __LIDAR_LAS_SINK_H__
#define __LIDAR_LAS_SINK_H__

#include "lidar/SimplePointWriter.h"

// liblas CAPI
struct LASWriterHS;
struct LASHeaderHS;
struct LASPointHS; 
struct LASColorHS; 

LT_BEGIN_LIDAR_NAMESPACE

class LASPointWriter : public SimplePointWriter
{
   CONCRETE_OBJECT(LASPointWriter);
public:
   
   enum FileVersion
   {
      VERSION_ANY = -1, // choose the lowest version that supports the data
      VERSION_1_0 = 10, // LAS 1.0
      VERSION_1_1 = 11, // LAS 1.1
      VERSION_1_2 = 12  // LAS 1.2
   };
   
   void init(const PointSource *src, const char *path,
             FileVersion fileVersion);

   void writeBegin(const PointInfo &pointInfo);
   void writePoints(const PointData &points,
                    size_t numPoints,
                    ProgressDelegate *delegate);
   void writeEnd(PointSource::count_type numPoints,
                 const Bounds &bounds);

   static FileVersion getFileVersion(const PointInfo &pointInfo);
   
protected:
   static int getRecordFormat(const PointInfo &pointInfo);
   
   char *m_output;
   FileVersion m_fileVersion;

   LASHeaderHS *m_header;
   LASWriterHS *m_writer;

   struct Handler;
   Handler *m_handler;
   size_t m_numHandlers;
   PointSource::count_type m_total;
   double LASMax[3];
   double LASMin[3];
   
};

LT_END_LIDAR_NAMESPACE  
#endif // __LIDAR_LAS_SINK_H__
