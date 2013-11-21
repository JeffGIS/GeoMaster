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
 *     File: sgl_windows.h
 *  Project: 16 April 2001
 *  Summary: various hacks needed for cross platform compatibility, and an
 *           attempt at removing windows.h from most dependencies lists by
 *           forward declaring some macros here.
 *****************************************************************************/

#ifndef __SGL_WINDOWS_H
#define __SGL_WINDOWS_H

#if !defined(WIN32)
  #ifndef APIENTRY
  #define APIENTRY
  #endif
#else

 #pragma warning (disable : 4786 4251 4661)
 #pragma warning (disable : 4244)

 #ifdef SGL_INCLUDE_WINDOWS
  #include <windows.h>
 #else
  // the following section gleaned from OSG
  // Under windows avoid including <windows.h>
  // to avoid name space pollution, but Win32's <GL/gl.h>
  // needs APIENTRY and WINGDIAPI defined properly.

  // XXX This is from Win32's <windef.h>
  #ifndef APIENTRY
  # if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
  #  define APIENTRY    __stdcall
  # else
  #  define APIENTRY
  # endif
  #endif
     // XXX This is from Win32's <winnt.h>
  #ifndef CALLBACK
  # if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
  #  define CALLBACK __stdcall
  # else
  #  define CALLBACK
  # endif
  #endif
     // XXX This is from Win32's <wingdi.h> and <winnt.h>
  #ifndef WINGDIAPI
  # define WINGDIAPI __declspec(dllimport)
  #endif
     // XXX This is from Win32's <ctype.h>
  #ifndef _WCHAR_T_DEFINED
  typedef unsigned short wchar_t;
  # define _WCHAR_T_DEFINED
  #endif
 #endif /* SGL_INCLUDE_WINDOWS */
#endif

#endif /* SGL_WINDOWS_H */
