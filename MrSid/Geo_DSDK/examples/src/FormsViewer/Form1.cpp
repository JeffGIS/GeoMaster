#include "stdafx.h"
#include "Form1.h"
#include <windows.h>

using namespace MrSidViewerExample;

//[STAThreadAttribute]
// Since this attribute is .NET 2.0 specific set it in the project Properties
// Linker Command Line Additional Options for all configurations by adding
// the following: /CLRTHREADATTRIBUTE:STA
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	System::Threading::Thread::CurrentThread->ApartmentState = System::Threading::ApartmentState::STA;
	Application::Run(new Form1());
	return 0;
}
