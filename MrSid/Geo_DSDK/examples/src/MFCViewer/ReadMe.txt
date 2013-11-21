========================================================================
    WIN32 APPLICATION : viewer Project Overview
========================================================================

simple SID viewing example in win32 api.

FUNCTION:
  bool OpenImage()
    open the SID image and setup your pipeline.

FUNCTION:
  void UpdateView()
    Here we will setup our new scene and export that scene to BIP
    format and forward that into a DIB.  Our magnification is 
    stored in g_mag.