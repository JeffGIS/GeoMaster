Readme file for 

    LizardTech MrSID Decode SDK For LiDAR
    Version 1.1.1

LizardTech, a Celartem Company
The Western Building, Suite 200, 1008 Western Avenue, Seattle WA 98104 USA

Copyright (c) 2010 LizardTech, Inc.
MrSID and LizardTech are registered trademarks of LizardTech, Inc.

Unauthorized use or distribution prohibited. Access to and use of this code is
permitted only under license from LizardTech Inc. Portions of the code are
protected by US and foreign patents and other filings.  All Rights Reserved.

 
INSTALLATION:

  No specific installation is required to use the LiDAR SDK beyond
  copying the SDK contents to your local machine.

  After installing, we suggest building and running the example program
  to assure correct installation and behaviour.

    api examples:  examples/src

  Contents include header files, libraries, documentation, examples and
  utilities.  Utilities include 
 
  - lidarinfo:  command line tool to get information (extent, number of points,
     etc) from LiDAR files.  Supported formats are MG4, LAS (1.0, 1.1 and 1.2)
     and TXT files.

  - lidardecode:  command line tool that transcodes LiDAR MG4 files to TXT or
     LAS.

  - DemoLidarViewer:  Win32/Win64 GUI application that renders LiDAR LAS and
     MG4 files.  This will require both DirectX9 with  and .NET 3.5 SP 1 (not
     included).  See Known Issues.

DOCUMENTATION:

  - User Manual: doc/UserManual/index.html
  - Reference Manual: doc/ReferenceManual/index.html

PROBLEM REPORTING:

  - Please see www.lizardtech.com/support for support options.  You will
    need to provide full SDK version and platform information; this can be
    found using the lidarinfo utility; pass the -version option.

KNOWN ISSUES:

- The Viewer utility is based on DirectX 9 ("DX9") and its Direct3D ("D3D")
   features.  The requires (a) current DX9 drivers (b) configuration of those
   drivers to enable D3D and (c) a compatible graphics card.  If any of these
   three requirements are not met, the viewer will not work.  So, if you have
   difficulties: (1) install DX9 / update drivers (2) run DXDIAG and ensure
   that Direct3D is enabled.  Known configurations that can not be supported
   are VMWare and Remote Desktop (aka Terminal Services).


LICENSES, COPYRIGHTS, TRADEMARKS and other acknowledgements:

  - See LICENSE.pdf for full licensing terms for this SDK.
  - Sample data is provided courtesy of The Sanborn Map Company Inc.

