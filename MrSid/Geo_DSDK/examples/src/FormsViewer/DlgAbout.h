/* $Id$ */
/* //////////////////////////////////////////////////////////////////////////
//                                                                         //
// This code is Copyright (c) 2005 LizardTech, Inc, 1008 Western Avenue,   //
// Suite 200, Seattle, WA 98104.  Unauthorized use or distribution         //
// prohibited.  Access to and use of this code is permitted only under     //
// license from LizardTech, Inc.  Portions of the code are protected by    //
// US and foreign patents and other filings. All Rights Reserved.          //
//                                                                         //
////////////////////////////////////////////////////////////////////////// */
/* PUBLIC */
#pragma once

	using namespace System;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;

__gc class CDlgAbout : public Form
{
public:
   CDlgAbout(void);
   ~CDlgAbout(void);

   private: System::Void CDlgAbout_Paint(System::Object *  sender, System::Windows::Forms::PaintEventArgs *  e)
   {
      Graphics *g = e->Graphics;

      g->DrawString( new String( "MrSid Example Viewer" ), 
                     new Drawing::Font( FontFamily::GenericSansSerif, 10 ), 
                     new SolidBrush( Color::Black ),    
                     25,25 );
      g->DrawString( new String( "Copyright (c) 2005 LizardTech, Inc." ),
                     new Drawing::Font( FontFamily::GenericSansSerif, 10 ), 
                     new SolidBrush( Color::Black ),    
                     25,50 );  
      g->DrawString( new String( "1008 Western Avenue, Suite 200, Seattle, WA. 98104" ),
                     new Drawing::Font( FontFamily::GenericSansSerif, 10 ), 
                     new SolidBrush( Color::Black ),    
                     25,75 );
   }
   private: System::Void CDlgAbout_MouseDown(System::Object *  sender, System::Windows::Forms::MouseEventArgs *  e)
	{
   }

};


