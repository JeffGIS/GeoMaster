// Prevent conflicts between liblas's stdint.hpp and <stdint.h>
#define __STDC_CONSTANT_MACROS

//self
#include "lidar/LASPointWriter.h"
#include "lidar/formats_status.h"
#include "lidar/core_status.h"
#include "lidar/Version.h"
#include "lidar/FileIO.h"
#include "lidar/Error.h"
#include "lidar/Private.h"

//liblas
#include "liblas/capi/liblas.h"
#include "liblas/lasvariablerecord.hpp"
#include "liblas/lasspatialreference.hpp"
#include "ogr_spatialref.h"
#include "cpl_conv.h"

#include <time.h>

LT_USE_LIDAR_NAMESPACE

#define LASF_Projection            "LASF_Projection"



typedef void (*WriteFunc)(LASPointH pt, LASColorH color,
                          size_t idx, const void *values);
struct LASPointWriter::Handler
{
   WriteFunc writer;
   const void *data;
};

LASPointWriter::LASPointWriter(void) :
   m_output(NULL),
   m_fileVersion(VERSION_ANY),
   m_header(NULL),
   m_writer(NULL),
   m_handler(NULL),
   m_numHandlers(0),
   m_total(0)
{
   memset (LASMax, 0, sizeof(LASMax));
   memset (LASMin, 0, sizeof(LASMax));
}

LASPointWriter::~LASPointWriter(void)
{
   if(m_writer != NULL)
      LASWriter_Destroy(m_writer);
   if(m_header != NULL)
      LASHeader_Destroy(m_header);

   DEALLOC(m_output);
}

IMPLEMENT_OBJECT_CREATE(LASPointWriter);

int
LASPointWriter::getRecordFormat(const PointInfo &pointInfo)
{
   int format = 0;
   if(pointInfo.hasChannel(CHANNEL_NAME_GPSTime))
      format += 1;

   if(pointInfo.hasChannel(CHANNEL_NAME_Red))
      format += 2;

   return format;
}

LASPointWriter::FileVersion
LASPointWriter::getFileVersion(const PointInfo &pointInfo)
{
   if(getRecordFormat(pointInfo) >= 2)
      return VERSION_1_2;
   else
      return VERSION_1_1;
}


void
LASPointWriter::init(const PointSource *src,
                     const char *path,
                     FileVersion fileVersion)
{
   assert(path != NULL);
   SimplePointWriter::init(src);
   m_output = STRDUP(path);
   m_fileVersion = fileVersion;
}

#define SIMPLE_WRITER(type, tag, func, cast, value) \
   static void writer_##func##_##tag(LASPointH pt, LASColorH /*color*/, \
   size_t idx, const type *values) \
{ LASPoint_##func(pt, static_cast<cast>(value)); }

#define SIMPLE_COLOR_WRITER(type, tag, func, cast, value) \
   static void writer_##func##_##tag(LASPointH /*pt*/, LASColorH color, \
   size_t idx, const type *values) \
{ LASColor_##func(color, static_cast<cast>(value)); }

#define FLOAT_WRITER(func, cast) \
   SIMPLE_WRITER(float, FLOAT32, func, cast, values[idx]) \
   SIMPLE_WRITER(double, FLOAT64, func, cast, values[idx])

#define INT_WRITER(func, cast) \
   SIMPLE_WRITER(lt_int8, SINT8, func, cast, values[idx]) \
   SIMPLE_WRITER(lt_uint8, UINT8, func, cast, values[idx]) \
   SIMPLE_WRITER(lt_int16, SINT16, func, cast, values[idx]) \
   SIMPLE_WRITER(lt_uint16, UINT16, func, cast, values[idx]) \
   SIMPLE_WRITER(lt_int32, SINT32, func, cast, values[idx]) \
   SIMPLE_WRITER(lt_uint32, UINT32, func, cast, values[idx]) \
   SIMPLE_WRITER(lt_int64, SINT64, func, cast, values[idx]) \
   SIMPLE_WRITER(lt_uint64, UINT64, func, cast, values[idx])

#define BOOL_WRITER(func, cast) \
   SIMPLE_WRITER(lt_int8, SINT8, func, cast, values[idx] ? 1 : 0) \
   SIMPLE_WRITER(lt_uint8, UINT8, func, cast, values[idx] ? 1 : 0) \
   SIMPLE_WRITER(lt_int16, SINT16, func, cast, values[idx] ? 1 : 0) \
   SIMPLE_WRITER(lt_uint16, UINT16, func, cast, values[idx] ? 1 : 0) \
   SIMPLE_WRITER(lt_int32, SINT32, func, cast, values[idx] ? 1 : 0) \
   SIMPLE_WRITER(lt_uint32, UINT32, func, cast, values[idx] ? 1 : 0) \
   SIMPLE_WRITER(lt_int64, SINT64, func, cast, values[idx] ? 1 : 0) \
   SIMPLE_WRITER(lt_uint64, UINT64, func, cast, values[idx] ? 1 : 0)

#define COLOR_WRITER(func, cast) \
   SIMPLE_COLOR_WRITER(lt_int8, SINT8, func, cast, values[idx]) \
   SIMPLE_COLOR_WRITER(lt_uint8, UINT8, func, cast, values[idx]) \
   SIMPLE_COLOR_WRITER(lt_int16, SINT16, func, cast, values[idx]) \
   SIMPLE_COLOR_WRITER(lt_uint16, UINT16, func, cast, values[idx]) \
   SIMPLE_COLOR_WRITER(lt_int32, SINT32, func, cast, values[idx]) \
   SIMPLE_COLOR_WRITER(lt_uint32, UINT32, func, cast, values[idx]) \
   SIMPLE_COLOR_WRITER(lt_int64, SINT64, func, cast, values[idx]) \
   SIMPLE_COLOR_WRITER(lt_uint64, UINT64, func, cast, values[idx])

FLOAT_WRITER(SetX, double)
FLOAT_WRITER(SetY, double)
FLOAT_WRITER(SetZ, double)
INT_WRITER(SetIntensity, uint16_t)
INT_WRITER(SetReturnNumber, uint16_t)
INT_WRITER(SetNumberOfReturns, uint16_t)
BOOL_WRITER(SetScanDirection, lt_uint64)
BOOL_WRITER(SetFlightLineEdge, lt_uint64)
INT_WRITER(SetClassification, uint8_t)
INT_WRITER(SetScanAngleRank, int8_t)
INT_WRITER(SetUserData, uint8_t)
INT_WRITER(SetPointSourceId, uint16_t)
FLOAT_WRITER(SetTime, double)
COLOR_WRITER(SetRed, uint16_t)
COLOR_WRITER(SetGreen, uint16_t)
COLOR_WRITER(SetBlue, uint16_t)

void
LASPointWriter::writeBegin(const PointInfo &pointInfo)
{
   m_total = 0;
   int recordFormat = getRecordFormat(pointInfo);
   int fileVersion = m_fileVersion;
   if(fileVersion == VERSION_ANY)
      fileVersion = getFileVersion(pointInfo);

   // writer should write out intersection of requested fields
   // and available fields

   m_header = LASHeader_Create();
   char strSoftwareID[32];
   // Generating Software:  software package and version was used during LAS
   // file creation 
   sprintf(strSoftwareID, "LizardTech %s", Version::getSDKVersionString());
   LASHeader_SetSoftwareId(m_header, strSoftwareID);
   LASHeader_SetSystemId(m_header, "");
   time_t theTime = time(0);
   tm *t = gmtime(&theTime);
   // tm_yday is days since January 1 (0-365)
   // but LAS Spec says 1 January is day 1
   LASHeader_SetCreationDOY(m_header, static_cast<uint16_t>(t->tm_yday + 1));
   LASHeader_SetCreationYear(m_header, static_cast<uint16_t>(t->tm_year + 1900));
   LASHeader_SetVersionMajor(m_header, static_cast<uint8_t>(fileVersion / 10));
   LASHeader_SetVersionMinor(m_header, static_cast<uint8_t>(fileVersion % 10));
   LASHeader_SetDataFormatId(m_header, static_cast<uint8_t>(recordFormat));
   LASHeader_SetScale(m_header, m_scale[0], m_scale[1], m_scale[2]);
   LASHeader_SetOffset(m_header, m_offsets[0], m_offsets[1], m_offsets[2]);

   bool hasGeoKeyDirectory = false;
   LASSRSH hSRS = LASHeader_GetSRS(m_header);

   const Metadata &metadata = getMetadata();
   
   for(size_t idx = 0; idx < metadata.getNumRecords(); idx += 1)
   {
      const char *key;
      const char *description;
      MetadataDataType datatype;
      const void *value;
      size_t length;

      metadata.get(idx, key, description, datatype, value, length);

      if (!strcmp(METADATA_KEY_SystemID, key))
         LASHeader_SetSystemId(m_header, static_cast<const char *>(value));
      else if (!strcmp(METADATA_KEY_ProjectID, key))
         LASHeader_SetProjectId(m_header, static_cast<const char *>(value));
      else if (!strcmp(METADATA_KEY_PointRecordsByReturnCount, key))
      {
         const double *p = static_cast<const double *>(value);
         for(int i = 0; i < 5; i += 1)
            LASHeader_SetPointRecordsByReturnCount(m_header, i,
            static_cast<uint32_t>(p[i]));
      }
      else if (!strcmp(METADATA_KEY_FileSourceID, key))
      {
         int fileSourceId;
         sscanf(static_cast<const char *>(value), "%d", &fileSourceId);
         LASHeader_SetFileSourceId(m_header,
            static_cast<uint16_t>(fileSourceId));
      }
      else if (!strcmp(METADATA_KEY_FileCreationDate, key))
         LASHeader_SetProjectId(m_header, static_cast<const char *>(value));
      else if(strcmp(METADATA_KEY_GeneratingSoftware, key) == 0)
         // Always "LizardTech V-X.Y"
         continue;
      else if(strcmp(METADATA_KEY_LASBBox, key) == 0)
      {
         LASMin[0] = static_cast<const double*>(value)[0];
         LASMin[1] = static_cast<const double*>(value)[2];
         LASMin[2] = static_cast<const double*>(value)[4];
         LASMax[0] = static_cast<const double*>(value)[1];
         LASMax[1] = static_cast<const double*>(value)[3];
         LASMax[2] = static_cast<const double*>(value)[5];
         continue;
      }   
      else
      {
         const char *e = strstr(key, "::");
         if(e == NULL)
            continue ; // unrecognized metadata.
         char userId[17];
         memset(userId, 0, sizeof(userId));
         memcpy(userId, key, MIN(e - key, 16));

         int recordId;
         sscanf(e + 2, "%d", &recordId);

         LASVLRH hVLR = LASVLR_Create();
         LASVLR_SetUserId(hVLR, userId);
         LASVLR_SetRecordId(hVLR, static_cast<uint16_t>(recordId));
         if(description == NULL)
            description = "";
         LASVLR_SetDescription(hVLR, description);

         LASVLR_SetData(hVLR,
            const_cast<uint8_t *>(static_cast<const uint8_t *>(value)),
            static_cast<uint16_t>(length));
         LASVLR_SetRecordLength(hVLR, static_cast<uint16_t>(length));
         if(strcmp(LASF_Projection, userId) == 0 &&
            (recordId == 34735 || recordId == 34736 || recordId == 34737))
         {
            hasGeoKeyDirectory = true;
            ((liblas::LASSpatialReference*)hSRS)->AddVLR(*((liblas::LASVariableRecord*)hVLR));
         }
         else
         {
            LASHeader_AddVLR(m_header, hVLR);
            LASVLR_Destroy(hVLR);
         }
      }
   }
   if (hasGeoKeyDirectory)
   {
      LASHeader_SetSRS(m_header, hSRS);
   }
   else
   {
      const char *wkt = getSrc()->getWKT();
      if(wkt != NULL)
      {
         LASError err = LASSRS_SetWKT(hSRS, wkt);
         if (!err)
            LASHeader_SetSRS(m_header, hSRS);
      }
   }
   LASSRS_Destroy(hSRS);

   m_numHandlers = pointInfo.getNumChannels();
   m_handler = ALLOC(Handler, sizeof(Handler) * m_numHandlers);
   for(size_t i = 0; i < m_numHandlers; i += 1)
   {
#define CASE(func, type) \
   case DATATYPE_##type: \
      writer = reinterpret_cast<WriteFunc>(writer_##func##_##type); \
      break

#define HANDLE_FLOAT(tag, func) \
   if(::strcmp(name, CHANNEL_NAME_##tag) == 0) \
   { \
      switch(datatype) \
      { \
         CASE(func, FLOAT32); \
         CASE(func, FLOAT64); \
         default: \
            THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_INVALID_PARAM) \
               ("invalid floating point number provided"); \
      } \
   }

#define HANDLE_INT(tag, func) \
   if(::strcmp(name, CHANNEL_NAME_##tag) == 0) \
   { \
      switch(datatype) \
      { \
         CASE(func, SINT8); \
         CASE(func, UINT8); \
         CASE(func, SINT16); \
         CASE(func, UINT16); \
         CASE(func, SINT32); \
         CASE(func, UINT32); \
         CASE(func, SINT64); \
         CASE(func, UINT64); \
         default: \
            THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_INVALID_PARAM) \
               ("invalid integer provided"); \
      } \
   }

      const ChannelInfo &channelInfo = pointInfo.getChannel(i);
      const char *name = channelInfo.getName();
      DataType datatype = channelInfo.getDataType();
      WriteFunc writer = NULL;
      HANDLE_FLOAT(X, SetX)
      else HANDLE_FLOAT(Y, SetY)
      else HANDLE_FLOAT(Z, SetZ)
      else HANDLE_INT(Intensity, SetIntensity)
      else HANDLE_INT(ReturnNum, SetReturnNumber)
      else HANDLE_INT(NumReturns, SetNumberOfReturns)
      else HANDLE_INT(ScanDir, SetScanDirection)
      else HANDLE_INT(EdgeFlightLine, SetFlightLineEdge)
      else HANDLE_INT(ClassId, SetClassification)
      else HANDLE_INT(ScanAngle, SetScanAngleRank)
      else HANDLE_INT(UserData, SetUserData)
      else HANDLE_INT(SourceId, SetPointSourceId)
      else HANDLE_FLOAT(GPSTime, SetTime)
      else HANDLE_INT(Red, SetRed)
      else HANDLE_INT(Green, SetGreen)
      else HANDLE_INT(Blue, SetBlue)
      else
         THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_INVALID_PARAM)
            ("invalid LAS field: %s", name);

      m_handler[i].writer = writer;
      m_handler[i].data = NULL;
   }

   m_writer = LASWriter_Create(m_output, m_header, LAS_MODE_WRITE);
   if(m_writer == NULL)
   {
      THROW_OS_ERROR()
         ("Unable to create output %s (%s).", m_output, LASError_GetLastErrorMsg());
   }
}

void
LASPointWriter::writePoints(const PointData &points,
                            size_t numPoints,
                            ProgressDelegate *delegate)
{
   if(delegate != NULL && delegate->getCancelled())
   {
      THROW_LIBRARY_ERROR(LTL_STATUS_CORE_OPERATION_CANCELLED)
         ("LAS Write operation cancelled.");
   }
   m_total += numPoints;
   if (m_total > MAX_VALUE<lt_uint32>())
      THROW_LIBRARY_ERROR(LTL_STATUS_FORMATS_INVALID_PARAM)
      ("Attempt to write 4 * 2 ^ 32 or more points to a LAS file.  That's not allowed by the LAS spec.");

   for(size_t i = 0; i < m_numHandlers; i += 1)
      m_handler[i].data = points.getChannel(i).getData();

   Handler *stop = m_handler + m_numHandlers;
   for(size_t i = 0; i < numPoints; i += 1)
   {
      LASPointH pt = LASPoint_Create();
      LASColorH color = LASColor_Create();
      for(Handler *handler = m_handler; handler < stop; handler += 1)
         handler->writer(pt, color, i, handler->data);

      LASPoint_SetColor(pt, color);
      LASColor_Destroy(color);
      if (LASWriter_WritePoint(m_writer, pt))
         THROW_LIBRARY_ERROR((LTL_STATUS_FORMATS_LIBLAS_WRITER))
         ("Unable to write point (%s)", LASError_GetLastErrorMsg());

      LASPoint_Destroy(pt);
   }

   if(delegate != NULL)
      delegate->updateCompleted(static_cast<double>(numPoints));
}

void
LASPointWriter::writeEnd(PointSource::count_type numPoints,
                         const Bounds &bounds)
{
   bool failed = numPoints == 0 && bounds == Bounds::Huge();
   if(m_writer != NULL)
   {
      if(!failed)
      {
         if (!(LASMax[0] == 0.0 && LASMax[1] == 0.0 && LASMax[2] == 0.0 &&
             LASMin[0] == 0.0 && LASMin[1] == 0.0 && LASMin[2] == 0.0))
         {
            LASHeader_SetMax(m_header, LASMax[0], LASMax[1], LASMax[2]);
            LASHeader_SetMin(m_header, LASMin[0], LASMin[1], LASMin[2]);
         }
         else
         {
            if (LASHeader_SetMax(m_header, bounds.x.max, bounds.y.max, bounds.z.max))
               THROW_LIBRARY_ERROR((LTL_STATUS_FORMATS_LIBLAS_HEADER))
               ("Unable to set max into header (%s)", LASError_GetLastErrorMsg());         
            if (LASHeader_SetMin(m_header, bounds.x.min, bounds.y.min, bounds.z.min))
               THROW_LIBRARY_ERROR((LTL_STATUS_FORMATS_LIBLAS_HEADER))
               ("Unable to set min into header (%s)", LASError_GetLastErrorMsg());
          }
         if (LASWriter_WriteHeader(m_writer, m_header))
            THROW_LIBRARY_ERROR((LTL_STATUS_FORMATS_LIBLAS_WRITER))
            ("Unable to update header (%s)", LASError_GetLastErrorMsg());
      }
      LASWriter_Destroy(m_writer); // liblas will update the pointcount for us here.
      m_writer = NULL;
      LASHeader_Destroy(m_header);
      m_header = NULL;
   }
   if(failed && m_output != NULL)
      FileIO::deleteFile(m_output); 
}
