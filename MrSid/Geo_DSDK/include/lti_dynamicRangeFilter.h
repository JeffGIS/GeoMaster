/* $Id$ */
/* //////////////////////////////////////////////////////////////////////////
//                                                                         //
// This code is Copyright (c) 2004 LizardTech, Inc, 1008 Western Avenue,   //
// Suite 200, Seattle, WA 98104.  Unauthorized use or distribution         //
// prohibited.  Access to and use of this code is permitted only under     //
// license from LizardTech, Inc.  Portions of the code are protected by    //
// US and foreign patents and other filings. All Rights Reserved.          //
//                                                                         //
////////////////////////////////////////////////////////////////////////// */
/* PUBLIC */

#ifndef LTI_DYNAMIC_RANGE_FILTER_H
#define LTI_DYNAMIC_RANGE_FILTER_H

// lt_lib_mrsid_core
#include "lti_pixel.h"
#include "lti_imageFilter.h"
#include "lti_imageStageOverrides.h"


LT_BEGIN_NAMESPACE(LizardTech)

class LTIReusableBSQBuffer;

/**
 * change dynamic range or datatype of the samples of the image
 *
 * Adjusts the sample values to fit the given dynamic range and datatype.
 */

#ifdef SWIG
class LTIDynamicRangeFilter : public LTIImageFilter
#else
class LTIDynamicRangeFilter : public LTIOverridePixelProps
                                     <LTIOverrideBackgroundPixel
                                     <LTIImageFilter> >
#endif
{
   LTI_REFERENCE_COUNTED_BOILERPLATE(LTIDynamicRangeFilter);
public:
   /**
    * initializer
    *
    * Creates an image stage with the sample data adjusted from the given
    * dynamic range values to the full dynamic range of the given datatype.
    * If not specified, the target datatype will be that of the source image.
    * If not specified, the filter will get the dynamic range from the source
    * image (i.e., it will use whatever is in the image metadata).
    *
    * @note the filter will stretch dynamic range on a per-band basis
    *
    * @param  srcImage       the base image
    * @param  srcDRMin       the minimum dynamic range value of the source data
    * @param  srcDRMax       the maximum dynamic range value of the source data
    * @param  dstDataType    the datatype of the new image stage
    */
   LT_STATUS initialize(LTIImageStage* srcImage,
                        const LTIPixel *srcDRMin = NULL,
                        const LTIPixel *srcDRMax = NULL,
                        LTIDataType dstDataType = LTI_DATATYPE_INVALID);

   /**
    * initializer  (for compatibly with LTIDataTypeTransformer)
    *
    * This initializer will cause the filter to do a straight datatype
    * conversion of the source image.  Advertised dynamic range in the source
    * image will be translated to the appropriate value for the new datatype.
    *
    * @param  srcImage     the base image
    * @param  dstDataType  the datatype of the new image stage
    */
   LT_STATUS initialize(LTIImageStage* srcImage,
                        LTIDataType dstDataType);

   /**
    * initializer
    *
    * Adjusts the sample data of the source image by scaling it from the
    * given source and destination dynamic range pixels.  Samples falling
    * outside of the given source range will be clipped to the range.
    *
    * @note the filter will stretch dynamic range on a per-band basis
    *
    * @param  srcImage    the base image
    * @param  srcMin      minimum of the dynamic range present in the source
    * @param  srcMax      maximum of the dynamic range present in the source
    * @param  dstMin      minimum bound of desired output dynamic range
    * @param  dstMax      maximum bound of desired output dynamic range
    */
   LT_STATUS initialize(LTIImageStage *srcImage,
                        const LTIPixel *srcMin, const LTIPixel *srcMax,
                        const LTIPixel &dstMin, const LTIPixel &dstMax);

   // LTIImageStage
   virtual lt_int64 getEncodingCost(const LTIScene& scene) const;
   virtual lt_uint32 getModifications(const LTIScene &scene) const;

   // LizardTech-internal only
   LT_STATUS setSrcMinMax(const LTIPixel& srcMin, const LTIPixel& srcMax);
   LT_STATUS setDstMinMax(const LTIPixel& srcMin, const LTIPixel& srcMax);

protected:
   LT_STATUS decodeStrip(LTISceneBuffer& stripBuffer,
                         const LTIScene& stripScene);

private:
   LT_STATUS transformBuffer(LTISceneBuffer &dstData,
                             LTISceneBuffer &srcData,
                             lt_uint32 numCols,
                             lt_uint32 numRows) const;
   LT_STATUS transformPixel(LTIPixel &newPixel,
                            const LTIPixel &oldPixel) const;

   const LTIPixel *m_srcMin;
   const LTIPixel *m_srcMax;
   LTIPixel m_dstMin;
   LTIPixel m_dstMax;

   LTIReusableBSQBuffer* m_buffer;
};


LT_END_NAMESPACE(LizardTech)

#endif // LTI_DYNAMIC_RANGE_FILTER_H
