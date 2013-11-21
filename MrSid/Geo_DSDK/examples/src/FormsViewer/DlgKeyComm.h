#pragma once

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;

__gc class CDlgKeyComm : public Form
{
public:
   CDlgKeyComm(void);
   ~CDlgKeyComm(void);

   private: System::Void CDlgKeyComm_Paint(System::Object *  sender, System::Windows::Forms::PaintEventArgs *  e)
   {
      Graphics *g = e->Graphics;

      g->DrawString( new String( "Zoom In: Left Mouse Button" ), 
                     new Drawing::Font( FontFamily::GenericSansSerif, 10 ), 
                     new SolidBrush( Color::Black ),    
                     25,25 );
      g->DrawString( new String( "Zoom Out: Middle Mouse Button" ), 
                     new Drawing::Font( FontFamily::GenericSansSerif, 10 ), 
                     new SolidBrush( Color::Black ),    
                     25,50 );
      g->DrawString( new String( "Pan: Right Mouse Button" ), 
                     new Drawing::Font( FontFamily::GenericSansSerif, 10 ), 
                     new SolidBrush( Color::Black ),    
                     25,75 );
   }
};
