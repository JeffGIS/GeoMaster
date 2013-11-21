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

#include "lt_base.h"
#include "lti_pixel.h"
#include "lti_scene.h"
#include "lti_sceneBuffer.h"
#include "MrSIDImageReader.h"
#include "lt_fileSpec.h" 
#include "lti_navigator.h"
#include "DlgAbout.h"
#include "DlgKeyComm.h"
#include "lti_bandSelectFilter.h"
#include "lti_colorTransformer.h"
#include "lti_multiresFilter.h"
#include "lti_viewerImageFilter.h"

LT_USE_NAMESPACE(LizardTech);	
bool flag = false;

#define ASSERT(b) Assert(__FILE__, __LINE__, #b, (b))
void Assert(const char* file, long line, const char* str, int cond);


namespace MrSidViewerExample
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
   using namespace System::Drawing::Imaging;
	using namespace System::IO;

	/// <summary> 
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public __gc class Form1 : public System::Windows::Forms::Form
	{	
	public:
		Form1(void)
		{
			InitializeComponent();
		}
  
	protected:
		void Dispose(Boolean disposing)
		{
			if (disposing && components)
			{
				components->Dispose();
			}
			__super::Dispose(disposing);
		}
	private: System::Windows::Forms::PictureBox *  pictureBox1;
	
	protected:
		LTIImageStage    *m_reader;	// Our sid image file reader
		LT_STATUS         m_sts;		// status variable, 0 = success
		bool              m_imageOpen;// flag: 1 if file has been opened
		Bitmap           *m_bmp;      // we write image to this bitmap 
		LTINavigator     *m_navv;		// navigator: does panning and zooming
									         // for us
		lt_uint8         *m_memBuf;   // buffer, reader writes scenes to 
									         // memory here
		double            m_oldMag;   // track the zoom level (powers of 2)
		double            m_imgCntrX; // x position of the center of the image
		double            m_imgCntrY; // y position of the center of the image
		lt_uint32         m_clWidth;  // width of the image rendering box
		lt_uint32         m_clHeight; // height of the image rendering box
		double            m_imgWidth; // width of the image
		double            m_imgHeight;// height of the image
		lt_uint32         m_bufWidth; // width of the buffer 
									         //(which receives decoded image)
		lt_uint32         m_bufHeight;// height of the buffer 
									         //(which receives decoded image)
		String           *m_str;

   private: System::Windows::Forms::MainMenu *  mainMenu1;
   private: System::Windows::Forms::MenuItem *  menu_File;
   private: System::Windows::Forms::MenuItem *  menu_Open;
   private: System::Windows::Forms::MenuItem *  menu_Close;
   private: System::Windows::Forms::MenuItem *  menu_Exit;
   private: System::Windows::Forms::MenuItem *  menu_KeyCommands;
   private: System::Windows::Forms::MenuItem *  menu_AboutMrSid;
   private: System::Windows::Forms::MenuItem *  menu_Help;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container * components;

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
         this->pictureBox1 = new System::Windows::Forms::PictureBox();
         this->mainMenu1 = new System::Windows::Forms::MainMenu();
         this->menu_File = new System::Windows::Forms::MenuItem();
         this->menu_Open = new System::Windows::Forms::MenuItem();
         this->menu_Close = new System::Windows::Forms::MenuItem();
         this->menu_Exit = new System::Windows::Forms::MenuItem();
         this->menu_Help = new System::Windows::Forms::MenuItem();
         this->menu_KeyCommands = new System::Windows::Forms::MenuItem();
         this->menu_AboutMrSid = new System::Windows::Forms::MenuItem();
         this->SuspendLayout();
         // 
         // pictureBox1
         // 
         this->pictureBox1->Anchor = (System::Windows::Forms::AnchorStyles)(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
            | System::Windows::Forms::AnchorStyles::Left) 
            | System::Windows::Forms::AnchorStyles::Right);
         this->pictureBox1->BackColor = System::Drawing::Color::Black;
         this->pictureBox1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
         this->pictureBox1->Location = System::Drawing::Point(0, 0);
         this->pictureBox1->Name = S"pictureBox1";
         this->pictureBox1->Size = System::Drawing::Size(592, 440);
         this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::CenterImage;
         this->pictureBox1->TabIndex = 0;
         this->pictureBox1->TabStop = false;
         this->pictureBox1->Paint += new System::Windows::Forms::PaintEventHandler(this, &MrSidViewerExample::Form1::pictureBox1_Paint);
         this->pictureBox1->MouseDown += new System::Windows::Forms::MouseEventHandler(this, &MrSidViewerExample::Form1::pictureBox1_MouseDown);
         // 
         // mainMenu1
         // 
         System::Windows::Forms::MenuItem* __mcTemp__1[] = new System::Windows::Forms::MenuItem*[2];
         __mcTemp__1[0] = this->menu_File;
         __mcTemp__1[1] = this->menu_Help;
         this->mainMenu1->MenuItems->AddRange(__mcTemp__1);
         // 
         // menu_File
         // 
         this->menu_File->Index = 0;
         System::Windows::Forms::MenuItem* __mcTemp__2[] = new System::Windows::Forms::MenuItem*[3];
         __mcTemp__2[0] = this->menu_Open;
         __mcTemp__2[1] = this->menu_Close;
         __mcTemp__2[2] = this->menu_Exit;
         this->menu_File->MenuItems->AddRange(__mcTemp__2);
         this->menu_File->Text = S"File";
         // 
         // menu_Open
         // 
         this->menu_Open->Index = 0;
         this->menu_Open->Shortcut = System::Windows::Forms::Shortcut::CtrlO;
         this->menu_Open->Text = S"Open";
         this->menu_Open->Click += new System::EventHandler(this, &MrSidViewerExample::Form1::menu_Open_Click);
         // 
         // menu_Close
         // 
         this->menu_Close->Index = 1;
         this->menu_Close->Shortcut = System::Windows::Forms::Shortcut::CtrlC;
         this->menu_Close->Text = S"Close";
         this->menu_Close->Click += new System::EventHandler(this, &MrSidViewerExample::Form1::menu_Close_Click);
         // 
         // menu_Exit
         // 
         this->menu_Exit->Index = 2;
         this->menu_Exit->Shortcut = System::Windows::Forms::Shortcut::CtrlX;
         this->menu_Exit->Text = S"Exit";
         this->menu_Exit->Click += new System::EventHandler(this, &MrSidViewerExample::Form1::menu_Exit_Click);
         // 
         // menu_Help
         // 
         this->menu_Help->Index = 1;
         System::Windows::Forms::MenuItem* __mcTemp__3[] = new System::Windows::Forms::MenuItem*[2];
         __mcTemp__3[0] = this->menu_KeyCommands;
         __mcTemp__3[1] = this->menu_AboutMrSid;
         this->menu_Help->MenuItems->AddRange(__mcTemp__3);
         this->menu_Help->Text = S"Help";
         // 
         // menu_KeyCommands
         // 
         this->menu_KeyCommands->Index = 0;
         this->menu_KeyCommands->Shortcut = System::Windows::Forms::Shortcut::CtrlK;
         this->menu_KeyCommands->Text = S"Key Commands";
         this->menu_KeyCommands->Click += new System::EventHandler(this, &MrSidViewerExample::Form1::menu_KeyCommands_Click);
         // 
         // menu_AboutMrSid
         // 
         this->menu_AboutMrSid->Index = 1;
         this->menu_AboutMrSid->Shortcut = System::Windows::Forms::Shortcut::CtrlA;
         this->menu_AboutMrSid->Text = S"About MrSidView...";
         this->menu_AboutMrSid->Click += new System::EventHandler(this, &MrSidViewerExample::Form1::menu_AboutMrSid_Click);
         // 
         // Form1
         // 
         this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
         this->ClientSize = System::Drawing::Size(592, 441);
         this->Controls->Add(this->pictureBox1);
         this->Menu = this->mainMenu1;
         this->Name = S"Form1";
         this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
         this->Text = S"MrSid Viewer Application";
         this->Resize += new System::EventHandler(this, &MrSidViewerExample::Form1::Form1_Resize);
         this->Load += new System::EventHandler(this, &MrSidViewerExample::Form1::Form1_Load);
         this->Closed += new System::EventHandler(this, &MrSidViewerExample::Form1::Form1_Closed);
         this->ResumeLayout(false);

      }	


	private: System::Void Form1_Resize(System::Object *  sender, System::EventArgs *  e)
	{
      m_clWidth = pictureBox1->Width;
      m_clHeight = pictureBox1->Height;
      
      if( m_imageOpen && m_clHeight > 0)
      {
         updateView(m_oldMag);
      }		
   }

	private: System::Void Form1_Load(System::Object *  sender, System::EventArgs *  e)
	{	
      m_sts = LT_STS_Uninit;
      m_memBuf = NULL;
      m_reader = NULL;
      m_navv = NULL;
      m_oldMag = 1.0;
      m_clWidth = pictureBox1->Width;
      m_clHeight = pictureBox1->Height;
      menu_Close->set_Enabled(false);
   }

	private: System::Void pictureBox1_Paint(System::Object *  sender, System::Windows::Forms::PaintEventArgs *  e)
	{
      if( m_imageOpen )
      {			
         Graphics* g = e->Graphics; 

			// we must specify the rect in which we wish to draw
			// our bitmap
         int newX = m_clWidth/2 - (int)m_imgWidth/2;
         int newY = m_clHeight/2 - (int)m_imgHeight/2;
         int ulx = LT_MAX( 0, newX );
         int uly = LT_MAX( 0, newY );

			Rectangle rect;
         rect.set_X(ulx);
         rect.set_Y(uly);
         rect.set_Width(m_bufWidth);
         rect.set_Height(m_bufHeight);

			// draw our bitmap in the specified rect
         g->DrawImage( m_bmp, rect );
      }	    
   }

	private: System::Void pictureBox1_MouseDown(System::Object *  sender, System::Windows::Forms::MouseEventArgs *  e)
	{		
      if( !m_imageOpen )
      {
         return;
      }

		switch( e->Button )
      {
         case MouseButtons::Left:
         {
            zoomIn();
            break;
         }
         case MouseButtons::Right:
         {
            Point p( e->X, e->Y );
            PanImageTo( p );
            break;
			}
			case MouseButtons::Middle:
         {
            zoomOut();
            break;
         }
      }
   }

	private: String* OpenFile() 
	{
      // Displays an OpenFileDialog and shows the read/only files.
      OpenFileDialog* dlgOpenFile = new OpenFileDialog();
      dlgOpenFile->InitialDirectory = "data\\";    
      dlgOpenFile->Filter = "MrSID files (*.sid)|*.sid" ;

      if (dlgOpenFile->ShowDialog() == DialogResult::OK)
      {
         String *pString = dlgOpenFile->FileName ;	
         return pString;		 
      }
      return 0;
   }
   private: void updateView(double newMag)
	{
      LT_STATUS sts = LT_STS_Uninit;
      // delete the original view	
      delete [] m_memBuf;
      m_memBuf = NULL;

      // It's good practice to manually dispose of gc classes
      if( m_bmp ){
         m_bmp->Dispose();
      }

      // get the dimensions at the new magnification
      lt_uint32 w,h; 		
      sts = m_reader->getDimsAtMag( newMag, w, h );
      m_imgWidth = w;
      m_imgHeight = h;		
      
      // set our buffer's X and Y to client X/Y
      // or Image X/Y (whichever is smaller)		
      m_bufWidth = (int)LT_MIN( m_clWidth, m_imgWidth );
      m_bufHeight = (int)LT_MIN( m_clHeight, m_imgHeight );	
      
      // Create a temporary scene to run some bounds checking tests
      LTINavigator tmpNavv( *m_reader, m_navv->getScene() );
      tmpNavv.setSceneAsCWH(	m_imgCntrX, m_imgCntrY, 
                              m_bufWidth, m_bufHeight, 
                              newMag );
      
      // Do some bounds checking
      {
         LTIScene scene = tmpNavv.getScene();	
         double ulx = scene.getLowerLeftX();
         double uly = scene.getUpperLeftY();	
         double lrx = scene.getLowerRightX();
         double lry = scene.getLowerRightY();	

         // bounds checking on Left and Right 	
         if( ulx < 0 ){	
            m_imgCntrX -= ulx;
         }
         else if( lrx > m_imgWidth ){
            m_imgCntrX -= (lrx - m_imgWidth);
         }		

         // bounds checking on top and Bottom 
         if( uly < 0 ){
            m_imgCntrY -= uly;
         }
         else if( lry > m_imgHeight ){
            m_imgCntrY -= (lry - m_imgHeight);
         }	
      }	
      
      // The scene will now set to be no larger
      // than the client area and also be completely contained
      // within the image bounds at the current magnification	
      sts = m_navv->setSceneAsCWH(	m_imgCntrX, m_imgCntrY, 
                                    m_bufWidth, m_bufHeight, newMag);
      ASSERT(LT_SUCCESS(sts));  

      
      // We will now create a bitmap with dimensions to contain our image.
      // Then, we lock the entire bitmap in order to write directly to memory.
      m_bmp = new Bitmap( m_bufWidth, m_bufHeight, PixelFormat::Format32bppRgb );

      // all pixels within this rectangle will be locked for writing purposes
      Drawing::Rectangle rect = 
               Drawing::Rectangle(0,0,m_bufWidth,m_bufHeight);      
      BitmapData *bitmapData = m_bmp->LockBits( rect,
                                                ImageLockMode::ReadOnly, 
                                                m_bmp->PixelFormat); 

      // pixels is a pointer to the topleft pixel in our Locked Rectangle (0,0)
      void* pixels = (void*)bitmapData->Scan0.ToPointer();
      LTISceneBuffer bufData(	m_reader->getPixelProps(), 
                              m_bufWidth, m_bufHeight, 
                              NULL);  

      sts = m_reader->read(m_navv->getScene(), bufData);
      ASSERT(LT_SUCCESS(sts));

      // we use LTIScenebuffer::exportData() to specify how we want our image
      // data formatted. pixels points to the locked pixels in Drawing::Bitmap
      // this will be displayed in BGR, BIP format. Note that Drawing::Bitmap's 
      // pixels are 4 bytes
      bufData.exportData( pixels, 4, 4*m_bufWidth,  1 );     

      
      // Commit the changes, and unlock the bitmap.        
      m_bmp->UnlockBits(bitmapData);
   }

	private: void zoomIn()
	{			
		// double the zoom level
		double newMag = m_oldMag*2;  
		double maxMag = m_reader->getMaxMagnification();
		if( newMag < maxMag )
		{	
         // compensate for double zoom
         m_imgCntrX *=2;  
         m_imgCntrY *=2;			

         updateView(newMag);
         m_oldMag = newMag;
         
         pictureBox1->Invalidate();
      }			
   }
	private: void zoomOut()
	{
      // halve the zoom level
      double newMag = m_oldMag/2;
      if( newMag >= m_reader->getMinMagnification() )
      {			
         // compensate for halved zoom
         m_imgCntrX /= 2; 
         m_imgCntrY /= 2;
         
         updateView(newMag);
         m_oldMag = newMag;

         pictureBox1->Invalidate();
      }	
   }
	private: void PanImageTo( Point &p )
	{		
      int dx = (p.X - m_clWidth/2);
      int dy = (p.Y - m_clHeight/2);
      m_imgCntrX += dx;
      m_imgCntrY += dy;	
      
      updateView(m_oldMag);
      pictureBox1->Invalidate();
   }

	private: System::Void Form1_Closed(System::Object *  sender, System::EventArgs *  e)
	{				
      delete m_navv;
      m_navv = NULL;
      
      ASSERT( m_reader != NULL );
      m_reader->release();
      m_reader = NULL;

      delete [] m_memBuf;
      m_memBuf = NULL;
   }


   private: System::Void menu_Open_Click(System::Object *  sender, System::EventArgs *  e)
   {      
      menu_Close->set_Enabled(true);

      LT_STATUS sts = LT_STS_Uninit;

      // try to open the file, if no file opened, exit function
      String *stFileName = OpenFile();		
      if( !stFileName ){
         return;
      }

      // delete any previous image state stuff		
      delete m_navv;
      m_navv = NULL;

      if(m_reader != NULL)
         m_reader->release();
      m_reader = NULL;

      // create the image reader
      {
         // Convert the incoming string to an LTFileSpec
         const wchar_t __pin* pFileName = PtrToStringChars(stFileName);
         LTFileSpec fileSpec(pFileName);
         pFileName = NULL;

         lt_uint8 generation;
         bool raster;
         sts = MrSIDImageReader::getMrSIDGeneration(fileSpec, generation, raster);
         ASSERT(LT_SUCCESS(sts));
         ASSERT(raster && "this is a LiDAR file - you need to use the LiDAR SDK to work with it");

         MrSIDImageReader *reader = MrSIDImageReader::create();
         ASSERT(reader != NULL);

         sts = reader->initialize(fileSpec);
         ASSERT(LT_SUCCESS(sts));

         m_reader = reader;
      }
     
      // It's possible that the minimum magnification on some images
      // isn't small enough to fit into our client area.
      // for this, we use LTIMultiResFilter to force a smaller magnification
      {
         LTIMultiResFilter *filter = LTIMultiResFilter::create();
         ASSERT(filter != NULL);

         // passing m_reader to initialize() will increment m_reader's reference count...
         sts = filter->initialize(m_reader, true);
         ASSERT(LT_SUCCESS(sts));

         m_reader->release(); // decrement reference count here.
         m_reader = filter;
      }

      // add a viewer filter to the pipeline.  This will do some handy conversions to our images
      // to ensure that we can display them.  (ie. swap the blue and red bands -- RGB to BGR)
      {  
         LTIViewerImageFilter *viewerFilter = LTIViewerImageFilter::create();
         ASSERT(viewerFilter != NULL);

         sts = viewerFilter->initialize(m_reader, true, true);
         ASSERT(LT_SUCCESS(sts));

         m_reader->release();
         m_reader = viewerFilter;
      }

      // our image is now either RGB or Grayscale: for ease of display later on,
      // we will canonicalize things to RGB here
      ASSERT((m_reader->getColorSpace() == LTI_COLORSPACE_GRAYSCALE &&
              m_reader->getNumBands() == 1) ||
             (m_reader->getColorSpace() == LTI_COLORSPACE_RGB &&
              m_reader->getNumBands() == 3));
      if (m_reader->getColorSpace() == LTI_COLORSPACE_GRAYSCALE)
      {
         const lt_uint16 bands[] = {0,0,0}; // map the single band into the R and G and B bands
         LTIBandSelectFilter *bandFilter = LTIBandSelectFilter::create();
         ASSERT(bandFilter != NULL);

         sts = bandFilter->initialize(m_reader, bands, 3, LTI_COLORSPACE_RGB);
         ASSERT(LT_SUCCESS(sts));

         m_reader->release();
         m_reader = bandFilter;
      }

      // create a scene and a navigator.  Using navigator,
      // find the best fit scene for our sid image.  
      LTIScene scene;
      m_navv = new LTINavigator(*m_reader);	
      sts = m_navv->bestFit( m_clWidth, m_clHeight, scene );
      ASSERT(LT_SUCCESS(sts));

      m_oldMag = scene.getMag();
      lt_uint32 w,h;  
      sts = m_reader->getDimsAtMag( m_oldMag, w, h );
      m_imgWidth = w;
      m_imgHeight = h;
      ASSERT(LT_SUCCESS(sts));

      m_imgCntrX = m_imgWidth/2;
      m_imgCntrY = m_imgHeight/2;
      updateView(m_oldMag);

      // set flag: image needs to be rendered     
      m_imageOpen = true;
      pictureBox1->Invalidate();
   }

   private: System::Void menu_Close_Click(System::Object *  sender, System::EventArgs *  e)
   {
      if( m_bmp )
      {
         m_bmp->Dispose();
      }

      ASSERT(m_reader != NULL);
      m_reader->release();
      m_reader = NULL;

      delete m_navv;
      m_navv = NULL;

      delete [] m_memBuf;
      m_memBuf = NULL;

      menu_Close->set_Enabled(false);
      m_imageOpen = false;

      pictureBox1->Invalidate();

   }

   private: System::Void menu_Exit_Click(System::Object *  sender, System::EventArgs *  e)
   {
      this->Close();
   }

   private: System::Void menu_KeyCommands_Click(System::Object *  sender, System::EventArgs *  e)
   {
      CDlgKeyComm *about = new CDlgKeyComm();
	   about->ShowDialog();
   }

   private: System::Void menu_AboutMrSid_Click(System::Object *  sender, System::EventArgs *  e)
   {
      CDlgAbout *about = new CDlgAbout();
	   about->ShowDialog();
   }

};
}


void Assert(const char* file, long line, const char* str, int cond)
{
   using namespace System;
   if (cond) 
      return;

   char msg[1024];
   sprintf(msg, "Assertion Failed \"%s\" at line [%d]", str, line);
   System::Windows::Forms::MessageBox::Show( msg );
   
   exit(1);
}

