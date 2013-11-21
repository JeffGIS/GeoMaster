// Prevent conflicts between liblas's stdint.hpp and <stdint.h>
#define  __STDC_CONSTANT_MACROS

#include "lidar/LASPointReader.h"
#include "lidar/core_status.h"
#include "lidar/formats_status.h"
#include "lidar/Error.h"
#include "lidar/Private.h"

#include "ogr_spatialref.h"
#include "liblas/capi/liblas.h"

#include <string.h>
#include <stdio.h>


LT_USE_LIDAR_NAMESPACE
#define LASF_Spec                  "LASF_Spec"
#define LASF_Projection            "LASF_Projection"

static const char *defaultClassId[] = {
  "Created, never classified",
  "Unclassified",
  "Ground",
  "Low Vegetation",
  "Medium Vegetation",
  "High Vegetation",
  "Building",
  "Low Point (noise)",
  "Model Key-point (mass point)",
  "Water",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Overlap Points",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition"
};

#define SIMPLE_READER(type, tag) \
static void read_##tag(const LASPointH pt, size_t idx, type *values) \
   { values[idx] = static_cast<type>(LASPoint_##tag(pt)); }

#define BOOL_READER(type, tag) \
   static void read_##tag(const LASPointH pt, size_t idx, type *values) \
   { values[idx] = LASPoint_##tag(pt) != 0; }

#define COLOR_READER(type, tag) \
   static void read_##tag(const LASPointH pt, size_t idx, type *values) \
   { values[idx] = LASColor_##tag(LASPoint_GetColor(pt)); }

// make sure the datatype are in sync with the m_channelInfo datatypes
SIMPLE_READER(double, GetX)
SIMPLE_READER(double, GetY)
SIMPLE_READER(double, GetZ)
SIMPLE_READER(lt_uint16, GetIntensity)
SIMPLE_READER(lt_uint8, GetReturnNumber)
SIMPLE_READER(lt_uint8, GetNumberOfReturns)
BOOL_READER(lt_uint8, GetScanDirection)
BOOL_READER(lt_uint8, GetFlightLineEdge)
SIMPLE_READER(lt_uint8, GetClassification)
SIMPLE_READER(lt_int8, GetScanAngleRank)
SIMPLE_READER(lt_uint8, GetUserData)
SIMPLE_READER(lt_uint16, GetPointSourceId)
SIMPLE_READER(double, GetTime);
COLOR_READER(lt_uint16, GetRed)
COLOR_READER(lt_uint16, GetGreen)
COLOR_READER(lt_uint16, GetBlue)

class LASPointReader::Iterator : public PointIterator
{
   CONCRETE_ITERATOR(Iterator);
protected:
   ~Iterator(void)
   {
      LASReader_Destroy(m_reader);
      DEALLOC(m_handler);
   }
   
public:
   Iterator(void) :
      m_reader(NULL),
      m_numHandlers(0),
      m_handler(NULL)
   {
   }
   
   void init(const Bounds &bounds,
             double fraction,
             const PointInfo &pointInfo,
             ProgressDelegate *delegate,
             const char *path)
   {
      assert(path != NULL && *path != '\0');
      PointIterator::init(bounds, fraction, pointInfo, delegate);
      if((m_reader = LASReader_Create(path)) == NULL)
         THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_LIBLAS_READER)
            ("Failed to create LASReader for path = '%s'", path);    

      m_numHandlers = pointInfo.getNumChannels();
      m_handler = ALLOC(Handler, sizeof(Handler) * m_numHandlers);
      for(size_t i = 0; i < m_numHandlers; i += 1)
      {
#define HANDLE(tag, func) \
   if(::strcmp(name, CHANNEL_NAME_##tag) == 0) \
      reader = reinterpret_cast<ReadFunc>(read_##func)

         const char *name = pointInfo.getChannel(i).getName();
         ReadFunc reader = NULL;
              HANDLE(X, GetX);
         else HANDLE(Y, GetY);
         else HANDLE(Z, GetZ);
         else HANDLE(Intensity, GetIntensity);
         else HANDLE(ReturnNum, GetReturnNumber);
         else HANDLE(NumReturns, GetNumberOfReturns);
         else HANDLE(ScanDir, GetScanDirection);
         else HANDLE(EdgeFlightLine, GetFlightLineEdge);
         else HANDLE(ClassId, GetClassification);
         else HANDLE(ScanAngle, GetScanAngleRank);
         else HANDLE(UserData, GetUserData);
         else HANDLE(SourceId, GetPointSourceId);
         else HANDLE(GPSTime, GetTime);
         else HANDLE(Red, GetRed);
         else HANDLE(Green, GetGreen);
         else HANDLE(Blue, GetBlue);
         else
            THROW_LIBRARY_ERROR(-1);
         
         m_handler[i].reader = reader;
         m_handler[i].data = NULL;
      }
   }
      
   size_t getNextPoints(PointData &points)
   {

      for(size_t i = 0; i < m_numHandlers; i += 1)
         m_handler[i].data = points.getChannel(i).getData();

      double *x = points.getX();
      const double *y = points.getY();
      const double *z = points.getZ();

      size_t count = 0;
      size_t totalCount = 0;
      size_t cancelCount = 0;
      LASPointH pt = NULL;
      Handler *stop = m_handler + m_numHandlers;
      while(count < points.getNumSamples() &&
            (pt = LASReader_GetNextPoint(m_reader)) != NULL)
      {
         totalCount += 1;
         for(Handler *handler = m_handler; handler < stop; handler += 1)
            handler->reader(pt, count, handler->data);

         if(useSample(x[count], y[count], z[count]))
            count += 1;

         cancelCount += 1;
         if(cancelCount == 4096)
         {
            if (m_delegate != NULL && m_delegate->getCancelled())
                THROW_LIBRARY_ERROR(LTL_STATUS_CORE_OPERATION_CANCELLED)
                   ("operation cancelled.");

            if(m_delegate != NULL)
               m_delegate->updateCompleted(static_cast<double>(cancelCount));

            cancelCount = 0;
         }
      }

      if(m_delegate != NULL)
         m_delegate->updateCompleted(static_cast<double>(cancelCount));

      return count;
   }
protected:

   typedef void (*ReadFunc)(const LASPointH pt, size_t idx, void *values);
   struct Handler
   {
      ReadFunc reader;
      void *data;
   };

   LASReaderH m_reader;
   size_t m_numHandlers;
   Handler *m_handler;
};

LASPointReader::LASPointReader(void) :
   m_path(NULL),
   //m_fileFormatString[32];
   m_classId(NULL),
   m_numClasses(0)
{
   ::memset(m_fileFormatString, 0, sizeof(m_fileFormatString));
}

LASPointReader::~LASPointReader(void)
{
   DEALLOC(m_path);
   if(m_classId != NULL && m_classId != defaultClassId)
   {
      for(size_t i = 0; i < m_numClasses; i += 1)
         DEALLOC(m_classId[i]);
      DEALLOC(m_classId);
   }
}

IMPLEMENT_OBJECT_CREATE(LASPointReader);

void
LASPointReader::loadHeaderProps(LASHeaderH header)
{
   ::snprintf(m_fileFormatString, sizeof(m_fileFormatString), "LAS %d.%d",
              LASHeader_GetVersionMajor(header),
              LASHeader_GetVersionMinor(header));

   int dataFormat = LASHeader_GetDataFormatId(header);
   size_t numChannels = 12;
   if(dataFormat == 1 || dataFormat == 3)
      numChannels += 1;
   if(dataFormat == 2 || dataFormat == 3)
      numChannels += 3;

   PointInfo pointInfo;
   pointInfo.init(numChannels);

   size_t i = 0;
   // make sure the datatypes are in sync with the read_*() functions
   pointInfo.getChannel(i++).init(CHANNEL_NAME_X, DATATYPE_FLOAT64, 32);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_Y, DATATYPE_FLOAT64, 32);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_Z, DATATYPE_FLOAT64, 32);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_Intensity, DATATYPE_UINT16, 16);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_ReturnNum, DATATYPE_UINT8, 3);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_NumReturns, DATATYPE_UINT8, 3);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_ScanDir, DATATYPE_UINT8, 1);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_EdgeFlightLine, DATATYPE_UINT8, 1);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_ClassId, DATATYPE_UINT8, 8);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_ScanAngle, DATATYPE_SINT8, 8);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_UserData, DATATYPE_UINT8, 8);
   pointInfo.getChannel(i++).init(CHANNEL_NAME_SourceId, DATATYPE_UINT16, 16);

   if(dataFormat == 1 || dataFormat == 3)
      pointInfo.getChannel(i++).init(CHANNEL_NAME_GPSTime, DATATYPE_FLOAT64, 64);

   if(dataFormat == 2 || dataFormat == 3)
   {
      pointInfo.getChannel(i++).init(CHANNEL_NAME_Red, DATATYPE_UINT16, 16);
      pointInfo.getChannel(i++).init(CHANNEL_NAME_Green, DATATYPE_UINT16, 16);
      pointInfo.getChannel(i++).init(CHANNEL_NAME_Blue, DATATYPE_UINT16, 16);
   }
   assert(i == numChannels);
   setPointInfo(pointInfo);
   setNumPoints(LASHeader_GetPointRecordsCount(header));

   Bounds bounds(LASHeader_GetMinX(header),
                 LASHeader_GetMaxX(header),
                 LASHeader_GetMinY(header),
                 LASHeader_GetMaxY(header),
                 LASHeader_GetMinZ(header),
                 LASHeader_GetMaxZ(header));
   setBounds(bounds);

   double scale[3], offset[3];
   scale[0] = LASHeader_GetScaleX(header);
   scale[1] = LASHeader_GetScaleY(header);
   scale[2] = LASHeader_GetScaleZ(header);
   offset[0] = LASHeader_GetOffsetX(header);
   offset[1] = LASHeader_GetOffsetY(header);
   offset[2] = LASHeader_GetOffsetZ(header);
   setQuantization(scale, offset);

   
   LASSRSH srs = LASHeader_GetSRS(header);
   char *wkt = LASSRS_GetWKT(srs);
   if(wkt != NULL)
   {
      // check for this string which is specific to gdal 1.6.0
      if(strcmp(wkt, "LOCAL_CS[\"unnamed\",UNIT[\"unknown\",1]]") != 0 )
      {
         setWKT(wkt);
      }
      LASString_Free(wkt);
   }
   LASSRS_Destroy(srs);
}

void
LASPointReader::loadClassNames(LASHeaderH header)
{
   uint32_t numVLR = LASHeader_GetRecordsCount(header);

   for(uint32_t i = 0; i < numVLR; i += 1)
   {
      LASVLRH pVLR = LASHeader_GetVLR(header, i);
      if(pVLR == NULL) 
         THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_LIBLAS_VLRHEADER)
            ("Unable to fetch VLR record %d of %d", i, numVLR);
      
      const char *recordName = LASVLR_GetUserId(pVLR);
      uint16_t recordId = LASVLR_GetRecordId(pVLR);

      if((::strcmp(LASF_Spec, recordName) == 0 && recordId == 0))
      {
         uint16_t recordLength = LASVLR_GetRecordLength(pVLR);
         m_numClasses = recordLength / 16; // should be 256
         m_classId = ALLOC(char *, sizeof(char *) * m_numClasses);
         ::memset(m_classId, 0, sizeof(char *) * m_numClasses);
         
         struct CLASSIFICATION
         {
            unsigned char id;
            char desc[15];
         };

         CLASSIFICATION *table = ALLOC(CLASSIFICATION, recordLength);
         LASVLR_GetData(pVLR, reinterpret_cast<uint8_t *>(table));
         for(size_t i = 0; i < m_numClasses ; i++)
            m_classId[table[i].id] = STRDUP(table[i].desc, 15);
         DEALLOC(table);
      }
      LASVLR_Destroy(pVLR);
   }
   if(m_classId == NULL)
   {
       m_classId = const_cast<char **>(defaultClassId);
       m_numClasses = sizeof(defaultClassId) / sizeof(*defaultClassId);
   }
}

void
LASPointReader::init(const char *path)
{
   if(path == NULL || *path == '\0')
      THROW_LIBRARY_ERROR(LTL_STATUS_CORE_INVALID_PARAM)
         ("path not given");

   m_path = STRDUP(path);

   LASReaderH reader = NULL;
   LASHeaderH header = NULL;
   try
   {
      if((reader = LASReader_Create(m_path)) == NULL)
         THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_LIBLAS_READER)
            ("Failed to create LASReader for path = '%s'", m_path);    

      if((header = LASReader_GetHeader(reader)) == NULL)
         THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_LIBLAS_HEADER)
            ("Failed to get a LASHeader from a reader", m_path);

      loadHeaderProps(header);
      loadClassNames(header);

      LASHeader_Destroy(header);
      LASReader_Destroy(reader);
   }
   catch(...)
   {
      LASHeader_Destroy(header);
      LASReader_Destroy(reader);
      throw;
   }
}

static double GregorianToJulianDayNumber(int year, int month, int day)
{
   if(month < 3)
   {
      month += 12;
      year -= 1;
   }

   const double c = 2 - floor(year / 100.0) + floor(year / 400.0);

   return floor(1461.0 * (year + 4716.0) / 4.0) + c +
          floor(153.0 * (month + 1) / 5.0) +
          day - 1524.5;
}

static void JulianDayNumberToGregorian(double jd, int &year, int &mouth, int &day)
{
   const double p = floor(jd + 0.5);
   const double s1 = p + 68569;
   const double n = floor(4.0 * s1 / 146097.0);
   const double s2 = s1 - floor((146097.0 * n + 3.0) / 4.0);
   const double i = floor(4000.0 * (s2 + 1) / 1461001.0);
   const double s3 = s2 - floor(1461.0 * i / 4.0) + 31.0;
   const double q = floor(80.0 * s3 / 2447.0);
   const double e = s3 - floor(2447.0 * q / 80.0);
   const double s4 = floor(q / 11.0);
   year = static_cast<int>(100 * (n - 49) + i + s4);
   mouth = static_cast<int>(q + 2 - 12 * s4);
   day = static_cast<int>(e + jd - p + 0.5);
}


void
LASPointReader::loadMetadata(LASHeaderH header,
                             Metadata &metadata,
                             bool sanitize)
{
   char str[256];

   int sourceId = LASHeader_GetFileSourceId(header);
   if(sourceId != 0)
   {
      snprintf(str, sizeof(str), "%d", sourceId);
      metadata.add(METADATA_KEY_FileSourceID, NULL,
                   METADATA_DATATYPE_STRING, str, 0);
   }

   char *projectId = LASHeader_GetProjectId(header);
   if(projectId != NULL)
   {
      if(*projectId != '\0')
         metadata.add(METADATA_KEY_ProjectID, NULL,
                      METADATA_DATATYPE_STRING, projectId, 0);
      LASString_Free(projectId);
   }

   char *systemIdentifier = LASHeader_GetSystemId(header);
   if(systemIdentifier != NULL)
   {
      if(*systemIdentifier != '\0')
         metadata.add(METADATA_KEY_SystemID, NULL,
                      METADATA_DATATYPE_STRING, systemIdentifier, 0);
      LASString_Free(systemIdentifier);
   }

   char *generatingSoftware = LASHeader_GetSoftwareId(header);
   if(generatingSoftware != NULL)
   {
      if(*generatingSoftware != '\0')
         metadata.add(METADATA_KEY_GeneratingSoftware, NULL,
                      METADATA_DATATYPE_STRING, generatingSoftware, 0);
      LASString_Free(generatingSoftware);
   }

   lt_uint16 creationYear = LASHeader_GetCreationYear(header);
   lt_uint16 creationDay = LASHeader_GetCreationDOY(header);
   if(creationYear >= 1900 && creationDay >= 1)
   {
      double jd = GregorianToJulianDayNumber(creationYear, 1, 1) + creationDay - 1;
      int year, mouth, day;
      JulianDayNumberToGregorian(jd, year, mouth, day);

      char creationDate[32];
      ::snprintf(creationDate, sizeof(creationDate), "%04d-%02d-%02d", year, mouth, day);
      metadata.add(METADATA_KEY_FileCreationDate, NULL,
                   METADATA_DATATYPE_STRING, creationDate, 0);
   }

   {
      double returnCounts[5];
      bool haveCounts = false;
      for(int i = 0; i < 5; i += 1)
      {
         returnCounts[i] = LASHeader_GetPointRecordsByReturnCount(header, i);
         if(returnCounts[i] != 0)
            haveCounts = true;
      }
      if(haveCounts)
         metadata.add(METADATA_KEY_PointRecordsByReturnCount, NULL,
                      METADATA_DATATYPE_REAL_ARRAY, returnCounts, 5);
   }
   
   {
      double bbox[6];
      bbox[0] = LASHeader_GetMinX(header); bbox[1] = LASHeader_GetMaxX(header); 
      bbox[2] = LASHeader_GetMinY(header); bbox[3] = LASHeader_GetMaxY(header); 
      bbox[4] = LASHeader_GetMinZ(header); bbox[5] = LASHeader_GetMaxZ(header); 
      metadata.add(METADATA_KEY_LASBBox, NULL,
                      METADATA_DATATYPE_REAL_ARRAY, bbox, 6);
   }
   uint32_t numVLR = LASHeader_GetRecordsCount(header);
   for(uint32_t i = 0; i < numVLR; i += 1)
   {
      LASVLRH pVLR = LASHeader_GetVLR(header, i);
      if(pVLR == NULL) 
          THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_LIBLAS_VLRHEADER)
            ("Unable to fetch VLR record %d of %d", i, numVLR);

      const char * pszVLRUser = LASVLR_GetUserId(pVLR);
      const char * pszVLRDescription = LASVLR_GetDescription(pVLR);
      uint16_t nVLRLength = LASVLR_GetRecordLength(pVLR);
      uint16_t nVLRRecordId = LASVLR_GetRecordId(pVLR);

      if(::strcmp(LASF_Spec, pszVLRUser) == 0 &&
         nVLRRecordId  == 0)
      {
         LASVLR_Destroy(pVLR);
         continue; // handled by ClassIds
      }
  
      // If we are sanitizing this, then only load the VLR records if they
      // are defined in the LAS Spec.  Otherwise get them all.
      
      if (!sanitize || ::strcmp(LASF_Spec, pszVLRUser) == 0 || ::strcmp(LASF_Projection, pszVLRUser) == 0)
      {
         sprintf(str, "%s::%d", pszVLRUser, nVLRRecordId);

         void *blob = ALLOC(void, nVLRLength);
         LASVLR_GetData(pVLR, static_cast<uint8_t *>(blob));
  
         metadata.add(str, pszVLRDescription,
                      METADATA_DATATYPE_BLOB, blob, nVLRLength);
         DEALLOC(blob);
      }
      LASVLR_Destroy(pVLR);
   }
}

void
LASPointReader::loadMetadata(Metadata &metadata, bool sanitize) const
{
   LASReaderH reader = NULL;
   LASHeaderH header = NULL;
   try
   {
      if((reader = LASReader_Create(m_path)) == NULL)
         THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_LIBLAS_READER)
            ("Failed to create LASReader for path = '%s'", m_path);    

      if((header = LASReader_GetHeader(reader)) == NULL)
         THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_LIBLAS_HEADER)
            ("Failed to get a LASHeader from a reader", m_path);

      loadMetadata(header, metadata, sanitize);

      LASHeader_Destroy(header);
      LASReader_Destroy(reader);
   }
   catch(...)
   {
      LASHeader_Destroy(header);
      LASReader_Destroy(reader);
      throw;
   }
}


char const * const *
LASPointReader::getClassIdNames(void) const
{
   return m_classId;
}

size_t
LASPointReader::getNumClassIdNames(void) const
{
   return m_numClasses;
}

const char *
LASPointReader::getFileFormatString(void) const
{
   return m_fileFormatString;
}

PointIterator *
LASPointReader::createIterator(const Bounds &bounds,
                               double fraction,
                               const PointInfo &pointInfo,
                               ProgressDelegate *delegate) const
{
   Scoped<Iterator> iter;
   iter->init(bounds, fraction, pointInfo, delegate,
              m_path);
   iter->retain();
   return iter;
}
