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
#include ".\dlgkeycomm.h"
#using <mscorlib.dll>

CDlgKeyComm::CDlgKeyComm(void)
{
	this->Text = "MrSid Viewer Key Commands";
	this->MinimizeBox = false;
	this->MaximizeBox = false;
	this->StartPosition = FormStartPosition::CenterParent;
	this->FormBorderStyle = FormBorderStyle::FixedDialog;
   this->Height = 150;
   this->Width = 300;
   this->Paint += new System::Windows::Forms::PaintEventHandler(this, &CDlgKeyComm::CDlgKeyComm_Paint);
        
}

CDlgKeyComm::~CDlgKeyComm(void)
{
}
