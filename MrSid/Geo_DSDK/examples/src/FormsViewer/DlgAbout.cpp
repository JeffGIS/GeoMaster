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
#include "StdAfx.h"
#include ".\dlgabout.h"
#using <mscorlib.dll>

CDlgAbout::CDlgAbout(void)
{
	this->Text = "About MrSid Viewer Example Application";
	this->MinimizeBox = false;
	this->MaximizeBox = false;
	this->StartPosition = FormStartPosition::CenterParent;
	this->FormBorderStyle = FormBorderStyle::FixedDialog;
   this->Height = 250;
   this->Width = 400;
   this->Paint += new System::Windows::Forms::PaintEventHandler(this, &CDlgAbout::CDlgAbout_Paint);
   this->MouseDown += new System::Windows::Forms::MouseEventHandler(this, &CDlgAbout::CDlgAbout_MouseDown);
        
}

CDlgAbout::~CDlgAbout(void)
{
}
