=============================================================================
    APPLICATION : MrSid Viewer Example
=============================================================================

this application demonstrates how to read and display mrsid files with 
the decode SDK.  

USAGE INSTRUCTIONS:
	
	Zoom In:  Left Mouse Button
	Zoom Out: Middle Mouse Button
	Pan: Right Mouse Button (button-click becomes new center)

RELEVANT CODE LOCATIONS:
	
	The function "menu_Open_Click" contains code that creates a LTFileSpec from
   the image selected.  Creates the MrSIDImageReader from the FileSpec.
   Determines the best fit for the MrSID image given the constraints of the
   client size.  If the color space is not RGB, the colorspace is converted 
   to RGB with the LTI_ColorTransform class.  And finally, if the minimum 
   magnification of the image is not large enough to fit into our Client's area,
   we add LTIMultiResFilter to the pipeline to compensate.

	The function "updateView" contains code that decodes the latest scene 
   (given via user zoom/pan inputs and client size) onto a locked portion 
   of the System::Drawing::Bitmap which is then invalidated and redrawn.
   Note that the Bitmap wants its pixels in BGR format and that the
   MrSID image is stored in RGB.  We use LTI_BandSelectFilter to swap the 
   red and blue bands of the image.

/////////////////////////////////////////////////////////////////////////////
