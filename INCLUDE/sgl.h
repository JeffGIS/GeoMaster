/*****************************************************************************
 * SGL: A Scene Graph Library
 *
 * Copyright (C) 1997-2004  Scott McMillan   All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************
 *     File: sgl.h
 *  Project: 26 June 1997
 *  Summary: Top level include file with miscellaneous constants and macros,
 *         : and various hacks needed for cross platform compatibility.
 *****************************************************************************/

#ifndef __SGL_H
#define __SGL_H

#define SGL_MAJOR_VERSION 1
#define SGL_MINOR_VERSION 0

#if defined(WIN32) && defined(_DLL)
// The next define will come from the makefile for archive objects.
# ifdef sgl_DLL_FILE
#  define SGL_DLL_API  __declspec(dllexport)
#  define SGL_DLL_API2 __declspec(dllexport)
# else
#  define SGL_DLL_API  __declspec(dllimport)
#  define SGL_DLL_API2 __declspec(dllimport)
# endif
# if (_MSC_VER) && (_MSC_VER > 1200 ) // after VC6
#  undef  SGL_DLL_API2
#  define SGL_DLL_API2
# endif
#else
# define SGL_DLL_API
# define SGL_DLL_API2
#endif

// the following encapsulates a bunch of hacks to get around cross platform
// issues when Windoze is involved.
#include <sgl_windows.h>

// a bunch of hacks to select standard conforming iostream stuff if available
// on the platform

#if defined(WIN32) || (defined(sgi) && defined(_STANDARD_C_PLUS_PLUS)) || (defined(__GNUC__) && (((__GNUC__==2) && (__GNUC_MINOR__>=91)) || __GNUC__>2))
# include <iostream>
# include <iomanip>
# include <fstream>
#else
# include <iostream.h>
# include <iomanip.h>
# include <fstream.h>
#endif

#ifndef __gl_h_
# ifdef MAC_OSX
#  include <gl.h>
# else
#  include <GL/gl.h>
# endif
#endif


#include <stdlib.h>
#include <math.h>
#include <float.h>  // for {FLT,DBL}_{MIN,MAX}
#include <assert.h>
#include <string.h>

#include <vector>

#if 0
// use for testing traversal speed by not really drawing geometry
# define SGL_NO_VERTEX_ARRAYS
# define glVertex3fv glColor3fv
#endif

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

#ifndef NULL
# define NULL (void *)0
#endif

// ANSI value; 1E-7 should be fine too
const float sglFltEpsilon = (float) 1.0e-7;

// ANSI value, &1E-16 should be fine
const double sglDblEpsilon = 1.0e-16;

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
# define M_PI_2 1.57079632679489661923
#endif

#ifndef M_PI_4
# define M_PI_4 0.78539816339744830962
#endif

const double cTanPi8 = tan(M_PI/8.0);

#ifndef M_LOG2E
# define M_LOG2E 1.4426950408889634074
#endif

template <class T>
inline T SGL_ABS(T a)
{
   return (a >= 0 ? a : -a);
}

template <class T>
inline T SGL_MIN(T a, T b)
{
   return (a < b ? a : b);
}

template <class T>
inline T SGL_MAX(T a, T b)
{
   return (a > b ? a : b);
}

inline bool sglAlmostEqual(float a, float b)
{
   return SGL_ABS((a)-(b)) < sglFltEpsilon;
}

inline bool sglAlmostEqual(double a, double b)
{
   return SGL_ABS((a)-(b)) < sglDblEpsilon;
}

template <class T>
inline T sglRelativeError(T a, T b)
{
   return SGL_ABS(a-b)/SGL_MAX(SGL_ABS(a), SGL_ABS(b));
}

inline bool sglRelativelyEqual(
   float a, float b, float relative_error = 1.0e-6)
{
   return sglRelativeError(a, b) < relative_error;
}

inline bool sglRelativelyEqual(
   double a, double b, double relative_error = 1.0e-14)
{
   return sglRelativeError(a, b) < relative_error;
}

template <class T, class S>
inline bool SGL_ALMOST_EQUAL(T a, S b, double eps)
{
   return SGL_ABS(a-b) < eps;
}

#ifndef SGL_SWAP
# define SGL_SWAP(a,b,temp) ((temp)=(a),(a)=(b),(b)=(temp))
#endif


// Use the std namespace. To do this we must first guarantee that it exists.
#if defined(__sgi) || defined(__WIN32_) || defined(WIN32) || defined (__ICC) ||(defined(__GNUC__) && __GNUC__>2)
namespace std {}
using namespace std;
#endif


class sglLock;


/** NEED CLASS DESCRIPTION HERE.

    Several environment variable can affect the runtime behavior of sgl.
    SGL_DATA_PATH - Defines the search path that sgl will use by default
        when loading data files.
    SGL_DISABLE_HARDWARE_MIPMAP - When set forces sgl to use a software
        algorithm for computing texture mipmaps.  This may be necessary
        on machines with a broken implementation of the
        GL_EXT_generate_mipmap extension.
    SGL_DISABLE_VERTEX_PREFETCH - When set prevents sgl from using the
        glDrawRangeElements functions in the draw routines.
    SGL_NUM_TEXTURE_UNITS - When set sgl will use the lesser of the
        number automatically detected texture units and the value of
        the environment variable.
*/

// this has to come before the sgl class because they are used in it
#ifndef GL_ARB_shader_objects
typedef char GLcharARB;		/* native character */
typedef unsigned int GLhandleARB;	/* shader object handle */
#endif

class SGL_DLL_API sgl
{
public:
   /** This function should be called before any drawing is done with sgl.
       NOTE: a valid graphics context _must_ be made current before calling
       this function.
       @param default_stream the ostream to use with sglPrint() output
   */
   static void initialize(ostream* default_stream = &cout);


   /** This function will allocate a new display list for use.
       NOTE: a valid graphics context _must_ be made current before calling
       this function.
       @return a new display list
   */
   static GLuint getDisplayList();

   /** This function will schedule the given display list for deletion.
       @param dlist display list to delete
   */
   static void releaseDisplayList(GLuint dlist);

   /** This function should be called every once in a while to delete
       unused display lists.  The preDraw() method of sglScene calls
       this function. NOTE: a valid graphics context _must_ be made current
       before calling this function.
   */
   static void deleteUnusedDisplayLists();

   /** This function will allocate a new texture object for use.
       NOTE: a valid graphics context _must_ be made current before calling
       this function.
       @return a new texture object
   */
   static GLuint getTextureObj();

   /** This function will schedule the given texture object for deletion.
       @param tex_obj texture object to delete
   */
   static void releaseTextureObj(GLuint tex_obj);

   /** This function should be called every once in a while to delete
       unused texture objects.  The preDraw() method of sglScene calls
       this function.  NOTE: a valid graphics context _must_ be made
       current before calling this function.
   */
   static void deleteUnusedTextureObjs();

   /** Gets the power of 2 that is greater or equal to the given value.
       @param val input value
       @return a power of 2 that is greater or equal to val
   */
   static unsigned int nextPower2(unsigned int val);

   /** Loads a dynamically loadable library with the given name.  Note that
       the true name of the dll actually loaded is system dependent:
       libdll_name.so on unix vs dll_name.dll on WIN32.
       @param dll_name name of dll to load
       @return a handle to the dll, or NULL if dll is not found
   */
   static void *getDLLHandle(const char *dll_name);

   /** Gets a pointer to a function in the given dll.
       @param dll handle to the dll to load from
       @param func_name name of the function to find
       @return function pointer, or NULL if function is not found
   */
   static void *getFuncPtr(void *dll, const char *func_name);

   /** Gets a pointer to a function in the opengl library.
       @param func_name name of the function to find
       @return function pointer, or NULL if function is not found
   */
   static void *getGLFuncPtr(const char *func_name);

protected:
   /** Not implemented.  Don't use. */
   sgl();
   /** Not implemented.  Don't use. */
   ~sgl();

private:
   /** Not implemented.  Don't use. */
   sgl(const sgl &);
   /** Not implemented.  Don't use. */
   sgl &operator=(const sgl &);

public:
   static bool            s_initialized;

   static GLint           s_max_prefetch_indices;
   static GLint           s_max_prefetch_vertices;
   static void (APIENTRY *drawRangeElements)(GLenum, GLuint, GLuint,
                                             GLsizei, GLenum, const GLvoid *);

   static GLint           s_max_texture_size;
   static GLint           s_clamp_to_edge;
   static GLint           s_clamp_to_border;
   static GLenum          s_rescale_normal;
   static float           s_max_texture_max_anisotropy;
   static GLint           s_num_texture_units;

   static void (APIENTRY *setActiveTextureUnit)(GLenum);
   static void (APIENTRY *setClientActiveTextureUnit)(GLenum);
   typedef void (APIENTRY *multiTexCoordFunc)(GLenum, const GLfloat*);
   static multiTexCoordFunc getMultiTexFunc(const char *type);
   static multiTexCoordFunc multiTexCoord1sv;
   static multiTexCoordFunc multiTexCoord2sv;
   static multiTexCoordFunc multiTexCoord3sv;
   static multiTexCoordFunc multiTexCoord4sv;
   static multiTexCoordFunc multiTexCoord1iv;
   static multiTexCoordFunc multiTexCoord2iv;
   static multiTexCoordFunc multiTexCoord3iv;
   static multiTexCoordFunc multiTexCoord4iv;
   static multiTexCoordFunc multiTexCoord1fv;
   static multiTexCoordFunc multiTexCoord2fv;
   static multiTexCoordFunc multiTexCoord3fv;
   static multiTexCoordFunc multiTexCoord4fv;
   static multiTexCoordFunc multiTexCoord1dv;
   static multiTexCoordFunc multiTexCoord2dv;
   static multiTexCoordFunc multiTexCoord3dv;
   static multiTexCoordFunc multiTexCoord4dv;

   // GLSL Shaders

   static bool s_shader_capable;
   static void ( APIENTRY * deleteObjectARB)(GLhandleARB);
   static GLhandleARB (APIENTRY * getHandleARB)(GLenum);
   static void ( APIENTRY * detachObjectARB)(GLhandleARB, GLhandleARB);
   static GLhandleARB (APIENTRY * createShaderObjectARB)(GLenum);
   static void ( APIENTRY * shaderSourceARB)(GLhandleARB, GLsizei, 
                                          const GLcharARB* *, const GLint *);
   static void ( APIENTRY * compileShaderARB)(GLhandleARB);
   static GLhandleARB (APIENTRY * createProgramObjectARB)(void);
   static void ( APIENTRY * attachObjectARB)(GLhandleARB, GLhandleARB);
   static void ( APIENTRY * linkProgramARB)(GLhandleARB);
   static void ( APIENTRY * useProgramObjectARB)(GLhandleARB);
   static void ( APIENTRY * validateProgramARB)(GLhandleARB);
   static void ( APIENTRY * uniform1fARB)(GLint, GLfloat);
   static void ( APIENTRY * uniform2fARB)(GLint, GLfloat, GLfloat);
   static void ( APIENTRY * uniform3fARB)(GLint, GLfloat, GLfloat, GLfloat);
   static void ( APIENTRY * uniform4fARB)(GLint, GLfloat, GLfloat, GLfloat, 
                                                                  GLfloat);
   static void ( APIENTRY * uniform1iARB)(GLint, GLint);
   static void ( APIENTRY * uniform2iARB)(GLint, GLint, GLint);
   static void ( APIENTRY * uniform3iARB)(GLint, GLint, GLint, GLint);
   static void ( APIENTRY * uniform4iARB)(GLint, GLint, GLint, GLint, GLint);
   static void ( APIENTRY * uniform1fvARB)(GLint, GLsizei, const GLfloat *);
   static void ( APIENTRY * uniform2fvARB)(GLint, GLsizei, const GLfloat *);
   static void ( APIENTRY * uniform3fvARB)(GLint, GLsizei, const GLfloat *);
   static void ( APIENTRY * uniform4fvARB)(GLint, GLsizei, const GLfloat *);
   static void ( APIENTRY * uniform1ivARB)(GLint, GLsizei, const GLint *);
   static void ( APIENTRY * uniform2ivARB)(GLint, GLsizei, const GLint *);
   static void ( APIENTRY * uniform3ivARB)(GLint, GLsizei, const GLint *);
   static void ( APIENTRY * uniform4ivARB)(GLint, GLsizei, const GLint *);
   static void ( APIENTRY * uniformMatrix2fvARB)(GLint, GLsizei, GLboolean, 
                                                            const GLfloat *);
   static void ( APIENTRY * uniformMatrix3fvARB)(GLint, GLsizei, GLboolean, 
                                                            const GLfloat *);
   static void ( APIENTRY * uniformMatrix4fvARB)(GLint, GLsizei, GLboolean, 
                                                            const GLfloat *);
   static void ( APIENTRY * getObjectParameterfvARB)(GLhandleARB, GLenum, 
                                                            GLfloat *);
   static void ( APIENTRY * getObjectParameterivARB)(GLhandleARB, GLenum, 
                                                            GLint *);
   static void ( APIENTRY * getInfoLogARB)(GLhandleARB, GLsizei, GLsizei *, 
                                                            GLcharARB *);
   static void ( APIENTRY * getAttachedObjectsARB)(GLhandleARB, GLsizei, 
                                                  GLsizei *, GLhandleARB *);
   static GLint (APIENTRY * getUniformLocationARB)(GLhandleARB, 
                                                         const GLcharARB *);
   static void ( APIENTRY * getActiveUniformARB)(GLhandleARB, GLuint, GLsizei,
                                   GLsizei *, GLint *, GLenum *, GLcharARB *);
   static void ( APIENTRY * getUniformfvARB)(GLhandleARB, GLint, GLfloat *);
   static void ( APIENTRY * getUniformivARB)(GLhandleARB, GLint, GLint *);
   static void ( APIENTRY * getShaderSourceARB)(GLhandleARB, GLsizei, 
                                                      GLsizei *, GLcharARB *);

   static unsigned int    s_num_statelet_types;

   static GLenum          s_stencil_inc_wrap;
   static GLenum          s_stencil_dec_wrap;
   static GLint           s_num_stencil_bits;

   static bool            s_multisample_capable;
   static bool            s_use_msalpha;
   static void (APIENTRY *sampleCoverage)(GLclampf, GLboolean);

   static bool            s_texture_env_add_capable;
   static bool            s_texture_env_combine_capable;
   static bool            s_texture_env_dot3_capable;

   static bool            s_occlusion_capable;
   static GLenum          s_occlusion_enable_flag;
   static GLenum          s_occlusion_result_flag;

   static bool            s_fragment_program_capable;
   static GLenum          s_fragment_program_enum;
   static bool            s_vertex_program_capable;
   static GLenum          s_vertex_program_enum;

   static bool            s_point_sprite_capable;

   static bool            s_depth_texture_capable;
   static bool            s_shadow_capable;
   static bool            s_shadow_ambient_capable;
   static bool            s_shadow_funcs_capable;

   // texture compression
   enum CompressionVendor
   {
      eCOMPRESSION_VENDOR_ARB,
      eCOMPRESSION_VENDOR_S3
   };
   static bool            s_texture_compression_capable;
   static CompressionVendor s_texture_compression_vendor;
   static void (APIENTRY *compressedTexImage2D)(
      GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
   static void (APIENTRY *getCompressedTexImage)(GLenum, GLint, GLvoid *);

   // auto-generate mipmap
   static bool            s_auto_mipmap_capable;
   static GLenum          s_auto_mipmap_enable_flag;

   // texture LOD bias
   static bool            s_texture_lod_bias_capable;
   static float           s_max_texture_lod_bias;

   // individual texture LOD
   static bool            s_texture_lod_capable;

   // fog distance equation support
   static bool            s_fog_distance_capable;

   // point parameters extension
   static bool            s_point_parameters_capable;
   static float           s_point_default_size_min;
   static float           s_point_default_size_max;
   static float           s_point_default_fade_threshold;
   static void (APIENTRY *pointParameterf)( GLenum, GLfloat );
   static void (APIENTRY *pointParameterfv)( GLenum, GLfloat * );

   // nv occlusion query
   static bool s_nv_occlusion_capable;
   static void (APIENTRY *genOcclusionQueriesNV)(GLsizei, GLuint *);
   static void (APIENTRY *deleteOcclusionQueriesNV)(GLsizei, const GLuint *);
   static void (APIENTRY *beginOcclusionQueryNV)(GLuint);
   static void (APIENTRY *endOcclusionQueryNV)(void);
   static void (APIENTRY *getOcclusionQueryuivNV)(GLuint, GLenum, GLuint *);

	 // ATI PN triangles extension
   static bool            s_pn_triangles_capable;
   static GLint           s_pn_triangles_max_tess_level;
   static void (APIENTRY *pnTrianglesATI)(GLenum, GLint);

	 // ATI envmap bumpmap
   static bool    s_envmap_bumpmap_capable;
   static GLint * s_envmap_bumpmap_units;
   static GLint   s_envmap_bumpmap_num_units;
   static GLint   s_envmap_bumpmap_matrix_size;
   static GLfloat*s_envmap_bumpmap_default_matrix;
   static void (APIENTRY *texBumpParameterfvATI)(GLenum, GLfloat *);
   static void (APIENTRY *getTexBumpParameterivATI)(GLenum, GLint *);
   static void (APIENTRY *getTexBumpParameterfvATI)(GLenum, GLfloat *);

   // cube map extension
   static bool            s_texture_cube_map_capable;
   static GLint           s_max_cube_map_texture_size;

   // number of lights supported by OGL
   static GLint           s_max_lights;

private:
   static vector<GLuint>  s_unused_display_lists;
   static sglLock         s_dlist_lock;

   static vector<GLuint>  s_unused_texture_objs;
   static sglLock         s_tex_lock;
};


#if (defined(MESA_MAJOR_VERSION) && ((MESA_MAJOR_VERSION<3) || ((MESA_MAJOR_VERSION==3) && (MESA_MINOR_VERSION<1))))
# define SGL_NO_VERTEX_ARRAYS
#endif

#if !defined(WIN32) && !(defined(__GNUC__) && (((__GNUC__==2) && (__GNUC_MINOR__ <= 8)) || (__GNUC__<2)))
# define TEMPLATE_OSTREAM_HACK
#endif


#ifndef GL_TEXTURE_COMPRESSION_HINT_ARB
#define GL_TEXTURE_COMPRESSION_HINT_ARB 0x84ef
#endif

#ifndef GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB 0x86a0
#endif

#ifndef GL_TEXTURE_COMPRESSED_ARB
#define GL_TEXTURE_COMPRESSED_ARB 0x86a1
#endif

#ifndef GL_TEXTURE_FILTER_CONTROL_EXT
#define GL_TEXTURE_FILTER_CONTROL_EXT 0x8500
#endif

#ifndef GL_TEXTURE_LOD_BIAS_EXT
#define GL_TEXTURE_LOD_BIAS_EXT 0x8501
#endif

#ifndef GL_MAX_TEXTURE_LOD_BIAS_EXT
#define GL_MAX_TEXTURE_LOD_BIAS_EXT 0x84fd
#endif

#ifndef GL_TEXTURE_MIN_LOD_SGIS
#define GL_TEXTURE_MIN_LOD_SGIS 0x813a
#endif

#ifndef GL_TEXTURE_MAX_LOD_SGIS
#define GL_TEXTURE_MAX_LOD_SGIS 0x813b
#endif

#ifndef GL_TEXTURE_BASE_LEVEL_SGIS
#define GL_TEXTURE_BASE_LEVEL_SGIS 0x813c
#endif

#ifndef GL_TEXTURE_MAX_LEVEL_SGIS
#define GL_TEXTURE_MAX_LEVEL_SGIS 0x813d
#endif

// cube map tokens GL_ARB_texture_cube_map extension
#ifndef GL_NORMAL_MAP_ARB
#define GL_NORMAL_MAP_ARB 0x8511
#endif

#ifndef GL_REFLECTION_MAP_ARB
#define GL_REFLECTION_MAP_ARB 0x8512
#endif

#ifndef GL_TEXTURE_CUBE_MAP_ARB
#define GL_TEXTURE_CUBE_MAP_ARB 0x8513
#endif

#ifndef GL_TEXTURE_BINDING_CUBE_MAP_ARB
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB 0x8514
#endif

#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#endif

#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#endif

#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#endif

#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#endif

#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#endif

#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851a
#endif

#ifndef GL_PROXY_TEXTURE_CUBE_MAP_ARB
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB 0x851b
#endif

#ifndef GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB 0x851c
#endif

// fog function tokens
#ifndef GL_FOG_DISTANCE_MODE_NV
#define GL_FOG_DISTANCE_MODE_NV 0x855a
#endif

#ifndef GL_EYE_RADIAL_NV
#define GL_EYE_RADIAL_NV 0x855b
#endif

#ifndef GL_EYE_PLANE_ABSOLUTE_NV
#define GL_EYE_PLANE_ABSOLUTE_NV 0x855c
#endif

#ifndef GL_POINT_SIZE_MIN_EXT
#define GL_POINT_SIZE_MIN_EXT 0x8126
#endif

#ifndef GL_POINT_SIZE_MAX_EXT
#define GL_POINT_SIZE_MAX_EXT 0x8127
#endif

#ifndef GL_POINT_FADE_THRESHOLD_SIZE_EXT
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT 0x8128
#endif

#ifndef GL_DISTANCE_ATTENUATION_EXT
#define GL_DISTANCE_ATTENUATION_EXT 0x8129
#endif

#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif

#ifndef GL_COMBINE
#define GL_COMBINE 0x8570
#endif

#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB 0x8571
#endif

#ifndef GL_COMBINE_ALPHA
#define GL_COMBINE_ALPHA 0x8572
#endif

#ifndef GL_SOURCE0_RGB
#define GL_SOURCE0_RGB 0x8580
#endif

#ifndef GL_SOURCE0_ALPHA
#define GL_SOURCE0_ALPHA 0x8588
#endif

#ifndef GL_OPERAND0_RGB
#define GL_OPERAND0_RGB 0x8590
#endif

#ifndef GL_OPERAND0_ALPHA
#define GL_OPERAND0_ALPHA 0x8598
#endif

#ifndef GL_RGB_SCALE
#define GL_RGB_SCALE 0x8573
#endif

#ifndef GL_ADD_SIGNED
#define GL_ADD_SIGNED 0x8574
#endif

#ifndef GL_INTERPOLATE
#define GL_INTERPOLATE 0x8575
#endif

#ifndef GL_SUBTRACT
#define GL_SUBTRACT 0x84E7
#endif

#ifndef GL_CONSTANT
#define GL_CONSTANT 0x8576
#endif

#ifndef GL_PRIMARY_COLOR
#define GL_PRIMARY_COLOR 0x8577
#endif

#ifndef GL_PREVIOUS
#define GL_PREVIOUS 0x8578
#endif

#ifndef GL_DOT3_RGB
#define GL_DOT3_RGB 0x86AE
#endif

#ifndef GL_DOT3_RGBA
#define GL_DOT3_RGBA 0x86AF
#endif

#ifndef GL_PIXEL_COUNTER_BITS_NV
#define GL_PIXEL_COUNTER_BITS_NV 0x8864
#endif

#ifndef GL_CURRENT_OCCLUSION_QUERY_ID_NV
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV 0x8865
#endif

#ifndef GL_PIXEL_COUNT_NV
#define GL_PIXEL_COUNT_NV 0x8866
#endif

#ifndef GL_PIXEL_COUNT_AVAILABLE_NV
#define GL_PIXEL_COUNT_AVAILABLE_NV 0x8867
#endif

#ifndef GL_DEPTH_COMPONENT16_ARB
#define GL_DEPTH_COMPONENT16_ARB 0x81a5
#endif

#ifndef GL_DEPTH_COMPONENT24_ARB
#define GL_DEPTH_COMPONENT24_ARB 0x81a6
#endif

#ifndef GL_DEPTH_COMPONENT32_ARB
#define GL_DEPTH_COMPONENT32_ARB 0x81a7
#endif

#ifndef GL_DEPTH_TEXTURE_MODE_ARB
#define GL_DEPTH_TEXTURE_MODE_ARB 0x884b
#endif

#ifndef GL_TEXTURE_COMPARE_MODE_ARB
#define GL_TEXTURE_COMPARE_MODE_ARB 0x884c
#endif

#ifndef GL_TEXTURE_COMPARE_FUNC_ARB
#define GL_TEXTURE_COMPARE_FUNC_ARB 0x884d
#endif

#ifndef GL_COMPARE_R_TO_TEXTURE_ARB
#define GL_COMPARE_R_TO_TEXTURE_ARB 0x884e
#endif

#ifndef GL_TEXTURE_COMPARE_FAIL_VALUE_ARB
#define GL_TEXTURE_COMPARE_FAIL_VALUE_ARB 0x80bf
#endif

#ifndef GL_CLAMP_TO_BORDER_ARB
#define GL_CLAMP_TO_BORDER_ARB 0x812D
#endif

#ifndef GL_POINT_SPRITE_ARB
#define GL_POINT_SPRITE_ARB 0x8861
#endif

#ifndef GL_COORD_REPLACE_ARB
#define GL_COORD_REPLACE_ARB 0x8862
#endif

#ifndef GL_PN_TRIANGLES_ATI
#define GL_PN_TRIANGLES_ATI 0x87f0
#endif

#ifndef GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87f1
#endif

#ifndef GL_PN_TRIANGLES_POINT_MODE_ATI
#define GL_PN_TRIANGLES_POINT_MODE_ATI 0x87f2
#endif

#ifndef GL_PN_TRIANGLES_NORMAL_MODE_ATI
#define GL_PN_TRIANGLES_NORMAL_MODE_ATI 0x87f3
#endif

#ifndef GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87f4
#endif

#ifndef GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI 0x87f5
#endif

#ifndef GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI 0x87f6
#endif

#ifndef GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI 0x87f7
#endif

#ifndef GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI 0x87f8
#endif

#ifndef GL_SAMPLE_ALPHA_TO_COVERAGE_ARB
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB 0x809e
#endif

#ifndef GL_SAMPLE_ALPHA_TO_ONE_ARB
#define GL_SAMPLE_ALPHA_TO_ONE_ARB 0x809f
#endif

#ifndef GL_SAMPLE_COVERAGE_ARB
#define GL_SAMPLE_COVERAGE_ARB 0x80a0
#endif

#ifndef GL_MULTISAMPLE_ARB
#define GL_MULTISAMPLE_ARB 0x809d
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80e1
#endif

#ifndef GL_BGR_EXT
#define GL_BGR_EXT 0x80e0
#endif

#ifndef GL_BUMP_ROT_MATRIX_ATI
#define GL_BUMP_ROT_MATRIX_ATI 0x8775
#endif

#ifndef GL_BUMP_ROT_MATRIX_SIZE_ATI
#define GL_BUMP_ROT_MATRIX_SIZE_ATI 0x8776
#endif

#ifndef GL_BUMP_NUM_TEX_UNITS_ATI
#define GL_BUMP_NUM_TEX_UNITS_ATI 0x8777
#endif

#ifndef GL_BUMP_TEX_UNITS_ATI
#define GL_BUMP_TEX_UNITS_ATI 0x8778
#endif

#ifndef GL_DUDV_ATI
#define GL_DUDV_ATI 0x8779
#endif

#ifndef GL_DU8DV8_ATI
#define GL_DU8DV8_ATI 0x877a
#endif

#ifndef GL_BUMP_ENVMAP_ATI
#define GL_BUMP_ENVMAP_ATI 0x877b
#endif

#ifndef GL_BUMP_TARGET_ATI
#define GL_BUMP_TARGET_ATI 0x877c
#endif

#ifndef GL_TEXTURE_DEPTH_SIZE
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#endif

// copied from the glext.h

#ifndef GL_ARB_shader_objects

//typedef char GLcharARB;		/* native character */
//typedef unsigned int GLhandleARB;	/* shader object handle */

#define GL_PROGRAM_OBJECT_ARB             0x8B40
#define GL_SHADER_OBJECT_ARB              0x8B48
#define GL_OBJECT_TYPE_ARB                0x8B4E
#define GL_OBJECT_SUBTYPE_ARB             0x8B4F
#define GL_FLOAT_VEC2_ARB                 0x8B50
#define GL_FLOAT_VEC3_ARB                 0x8B51
#define GL_FLOAT_VEC4_ARB                 0x8B52
#define GL_INT_VEC2_ARB                   0x8B53
#define GL_INT_VEC3_ARB                   0x8B54
#define GL_INT_VEC4_ARB                   0x8B55
#define GL_BOOL_ARB                       0x8B56
#define GL_BOOL_VEC2_ARB                  0x8B57
#define GL_BOOL_VEC3_ARB                  0x8B58
#define GL_BOOL_VEC4_ARB                  0x8B59
#define GL_FLOAT_MAT2_ARB                 0x8B5A
#define GL_FLOAT_MAT3_ARB                 0x8B5B
#define GL_FLOAT_MAT4_ARB                 0x8B5C
#define GL_OBJECT_DELETE_STATUS_ARB       0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB     0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB     0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB    0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB     0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB 0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB 0x8B88
#endif

#ifndef GL_ARB_vertex_shader
#define GL_VERTEX_SHADER_ARB              0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB 0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB         0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB 0x8B4D
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB   0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB 0x8B8A
#endif

#ifndef GL_ARB_fragment_shader
#define GL_FRAGMENT_SHADER_ARB            0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB 0x8B49
#endif

#endif
