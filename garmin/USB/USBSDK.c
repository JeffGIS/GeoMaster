
// interface with Garmin USB units. This should not be viewed as a
// full implementation of PC to unit communication. It does not include
// error checking and other elements of a full implementation.
// Also, there are notes in the code suggesting other elements that
// might be necessary.

#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <lmerr.h>

#include <initguid.h>
#include <setupapi.h> // You may need to explicitly link with setupapi.lib
#include <winioctl.h>
#if XPORABOVE
DEFINE_GUID(GUID_DEVINTERFACE_GRMNUSB, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0);

#define IOCTL_ASYNC_IN        CTL_CODE (FILE_DEVICE_UNKNOWN, 0x850, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USB_PACKET_SIZE CTL_CODE (FILE_DEVICE_UNKNOWN, 0x851, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MAX_BUFFER_SIZE 4096
#define ASYNC_DATA_SIZE 64
#define MaxWait	5
//-----------------------------------------------------------------------------
static	HANDLE gHandle=0;
static	BOOL	FirstSinceSend;

char	str[1024];
extern	HWND	hWndMain;

DWORD gUSBPacketSize;
BOOL CloseGarminUSB (LPSTR NullArg);

//-----------------------------------------------------------------------------
#pragma pack( push, 1)
typedef struct
    {
    unsigned char  mPacketType;
    unsigned char  mReserved1;
    unsigned short mReserved2;
    unsigned short mPacketId;
    unsigned short mReserved3;
    unsigned long  mDataSize;
    BYTE           mData[1];
    } Packet_t;
#pragma pack( pop)
void MessageBox2 (HWND hWnd,LPSTR lpText,LPSTR lpCaption,UINT uType)
{
	//MessageBox (hWnd,lpText,lpCaption,uType);
	return;
}

void DisplayError(
    DWORD dwLastError
    )
{
    HMODULE hModule = NULL; // default to system source
    LPSTR MessageBuffer;
    DWORD dwBufferLength;

    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM ;

    //
    // If dwLastError is in the network range, 
    //  load the message source.
    //

    if(dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
        hModule = LoadLibraryEx(
            TEXT("netmsg.dll"),
            NULL,
            LOAD_LIBRARY_AS_DATAFILE
            );

        if(hModule != NULL)
            dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    //
    // Call FormatMessage() to allow for message 
    //  text to be acquired from the system 
    //  or from the supplied module handle.
    //

    if(dwBufferLength = FormatMessageA(
        dwFormatFlags,
        hModule, // module to get message from (NULL == system)
        dwLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
        (LPSTR) &MessageBuffer,
        0,
        NULL
        ))
    {

        //
        // Output message string on stderr.
        //
       /*WriteFile(
            GetStdHandle(STD_ERROR_HANDLE),
            MessageBuffer,
            dwBufferLength,
            &dwBytesWritten,
            NULL
            );*/
		MessageBox2 (0,MessageBuffer,NULL,MB_ICONEXCLAMATION);

        //
        // Free the buffer allocated by the system.
        //
        LocalFree(MessageBuffer);
    }

    //
    // If we loaded a message source, unload it.
    //
    if(hModule != NULL)
        FreeLibrary(hModule);
}

//-----------------------------------------------------------------------------
BOOL
SendPacket_USB
    (
    Packet_t *aPacket
    )
{
DWORD theBytesToWrite = sizeof( Packet_t) - 1 + aPacket->mDataSize;
DWORD theBytesReturned = 0;

//sprintf (str,"Writing %ld bytes to %ld",theBytesToWrite,(long)gHandle);
//MessageBox2 (0,str,NULL,MB_OK);
	sprintf (str,"write to %ld-%i-%i-%i-%i-%i",(long)gHandle,theBytesToWrite,aPacket->mPacketType,
											   aPacket->mPacketId,
							                   aPacket->mDataSize,
											   aPacket->mData[0]);
	MessageBox2 (0,str,"",MB_OK);
if (!WriteFile( gHandle,
           aPacket,
           theBytesToWrite,
           &theBytesReturned,
           NULL ))
{
	DWORD	err=GetLastError();
	MessageBox2 (0,"Got write error",NULL,MB_OK);
	DisplayError (err);
	return FALSE;
}
if (theBytesToWrite != theBytesReturned)
{
	MessageBox2 (0,"BytesToWrite not written",NULL,MB_OK);
	return FALSE;
}

// If the packet size was an exact multiple of the USB packet
// size, we must make a final write call with no data
if( theBytesToWrite % gUSBPacketSize == 0 )
    {
	MessageBox2 (0,"Writing 0 byte",NULL,MB_OK);
    WriteFile( gHandle,
               0,
               0,
               &theBytesReturned,
               NULL );
    }
	FirstSinceSend=TRUE;
	return TRUE;
}

BOOL SendPacketP (LPSTR pPacket)
{

	if (!gHandle)
		return FALSE;
	return SendPacket_USB ((Packet_t *)pPacket);
}

//-----------------------------------------------------------------------------
// Gets a single packet. Since packets may come simultaneously through
// asynchrous reads and normal (ReadFile) reads, a full implementation
// may require a packet queue and multiple threads.
Packet_t*
GetPacket_USB
    (
    )
{
Packet_t* thePacket = 0;
DWORD theBufferSize = 0;
BYTE* theBuffer = 0;
DWORD theBytesReturned,TotBytes=0;

for( ; ; )
    {
    // Read async data until the driver returns less than the
    // max async data size, which signifies the end of a packet
    //BYTE theTempBuffer[ASYNC_DATA_SIZE];
    BYTE theTempBuffer[MAX_BUFFER_SIZE];
    BYTE* theNewBuffer = 0;

    theBytesReturned = 0;
	ltoa ((long)gHandle,str,10);
	if (!FirstSinceSend)
	{
		MessageBox2 (0,"Calling Readfile",str,MB_OK);
			ReadFile( gHandle,
					  theTempBuffer,
					  MAX_BUFFER_SIZE,
					  &theBytesReturned,
					  NULL );
		sprintf (str,"Returning Readfile %ld - %ld",theBytesReturned,TotBytes);
		MessageBox2 (0,str,NULL,MB_OK);
	}
	else
	{
		MessageBox2 (0,"Calling DeviceIO",str,MB_OK);
		DeviceIoControl( gHandle,
						 IOCTL_ASYNC_IN,
						 0,
						 0,
						 theTempBuffer,
						 sizeof( theTempBuffer ),
						 &theBytesReturned,
						 NULL );
		MessageBox2 (0,"Return from Calling DeviceIO",str,MB_OK);
	}
	if (!theBytesReturned && !TotBytes)
	{
		free( theBuffer );
		return NULL;
	}
	theBufferSize += theBytesReturned;
    theNewBuffer = (BYTE*) malloc( theBufferSize );
    memcpy( theNewBuffer, theBuffer, theBufferSize - theBytesReturned );
    memcpy( theNewBuffer + theBufferSize - theBytesReturned,
            theTempBuffer,
            theBytesReturned );

    free( theBuffer );

    theBuffer = theNewBuffer;

	if (!theBytesReturned)
	{
		MessageBox2 (0,"Got 0 bytes1",NULL,MB_OK);
		((Packet_t*) theNewBuffer)->mDataSize = 0;
	}
//    if(!FirstSinceSend || theBytesReturned != ASYNC_DATA_SIZE )
     if(theBytesReturned != ASYNC_DATA_SIZE )
       {
        thePacket = (Packet_t*) theBuffer;
        break;
        }
    }
	FirstSinceSend = FALSE;

	sprintf (str,"1-%i-%i-%i-%i",theBytesReturned,thePacket->mPacketType,
											   thePacket->mPacketId,
							                   thePacket->mDataSize);
	MessageBox2 (0,str,"",MB_OK);
// If this was a small "signal" packet, read a real
// packet using ReadFile
if( thePacket->mPacketType == 0 &&
    thePacket->mPacketId == 2 )
    {
		BYTE* theNewBuffer = (BYTE*) malloc( MAX_BUFFER_SIZE );
		DWORD theBytesReturned = 0;

		free( thePacket );
		MessageBox2 (0,"Using Readfile","",MB_OK);
		// A full implementation would keep reading (and queueing)
		// packets until the driver returns a 0 size buffer.
		ReadFile( gHandle,
				  theNewBuffer,
				  MAX_BUFFER_SIZE,
				  &theBytesReturned,
				  NULL );
		sprintf (str,"2-%i-%i-%i-%i",theBytesReturned,(int)((Packet_t*) theNewBuffer)->mPacketType,
												   (int)((Packet_t*) theNewBuffer)->mPacketId,
												   ((Packet_t*) theNewBuffer)->mDataSize);
		MessageBox2 (0,str,"",MB_OK);
		if (!theBytesReturned)
		{
			MessageBox2 (0,"Got 0 bytes2",NULL,MB_OK);
			((Packet_t*) theNewBuffer)->mDataSize = 0;
		}
/*		if (theBytesReturned)
		{
			BYTE* theEmptyBuffer = (BYTE*) malloc( MAX_BUFFER_SIZE );
			MessageBox2 (0,"Reading empty 1",NULL,MB_OK);
			ReadFile( gHandle,
					  theEmptyBuffer,
					  MAX_BUFFER_SIZE,
					  &theBytesReturned,
					  NULL );
			free (theEmptyBuffer);
			MessageBox2 (0,"Got empty 1",NULL,MB_OK);
		}*/
		return (Packet_t*) theNewBuffer;
    }
else
    {
/*		if (theBytesReturned && thePacket->mPacketType)
		{
			BYTE* theEmptyBuffer = (BYTE*) malloc( MAX_BUFFER_SIZE );
			MessageBox2 (0,"Reading empty 2",NULL,MB_OK);
			ReadFile( gHandle,
					  theEmptyBuffer,
					  MAX_BUFFER_SIZE,
					  &theBytesReturned,
					  NULL );
			free (theEmptyBuffer);
			MessageBox2 (0,"Got empty 2",NULL,MB_OK);
		}*/
		return thePacket;
    }
}

//-----------------------------------------------------------------------------
BOOL
InitializeUSB
    (
    )
{
// Make all the necessary Windows calls to get a handle
// to our USB device
DWORD theBytesReturned = 0;

PSP_INTERFACE_DEVICE_DETAIL_DATA theDevDetailData = 0;
SP_DEVINFO_DATA theDevInfoData = { sizeof( SP_DEVINFO_DATA ) };

Packet_t theStartSessionPacket = { 0, 0, 0, 5, 0 , 0 };
Packet_t* thePacket = 0;

HDEVINFO theDevInfo = SetupDiGetClassDevs(
    (GUID*) &GUID_DEVINTERFACE_GRMNUSB,
    NULL,
    NULL,
    DIGCF_ALLCLASSES |
	DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

SP_DEVICE_INTERFACE_DATA theInterfaceData;
theInterfaceData.cbSize = sizeof( theInterfaceData );

if( !SetupDiEnumDeviceInterfaces( theDevInfo,
                                  NULL,
                                  (GUID*) &GUID_DEVINTERFACE_GRMNUSB,
                                  0,
                                  &theInterfaceData ) &&
    GetLastError() == ERROR_NO_MORE_ITEMS )
    {
    gHandle = 0;
    return FALSE;
    }


SetupDiGetDeviceInterfaceDetail(
    theDevInfo,
    &theInterfaceData,
    NULL,
    0,
    &theBytesReturned,
    NULL );

theDevDetailData =
    (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc( theBytesReturned );
theDevDetailData->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

SetupDiGetDeviceInterfaceDetail( theDevInfo,
                                 &theInterfaceData,
                                 theDevDetailData,
                                 theBytesReturned,
                                 NULL,
                                 &theDevInfoData );

gHandle = CreateFile(
    theDevDetailData->DevicePath,
    GENERIC_READ | GENERIC_WRITE,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL );
if (gHandle == INVALID_HANDLE_VALUE)
{
	DWORD	err=GetLastError();
	MessageBox2 (0,"Got write error",NULL,MB_OK);
	DisplayError (err);
	return FALSE;
}

free( theDevDetailData );
sprintf (str,"Open %ld",(long)gHandle);
MessageBox2 (0,str,NULL,MB_OK);
// Get the USB packet size, which we need for sending packets
DeviceIoControl( gHandle,
                 IOCTL_USB_PACKET_SIZE,
                 0,
                 0,
                 &gUSBPacketSize,
                 sizeof( gUSBPacketSize ),
                 &theBytesReturned,
                 NULL );

// Tell the device that we are starting a session.
if (!SendPacket_USB( &theStartSessionPacket ))
	return FALSE;

// Wait until the device is ready to the start the session
for( ; ; )
    {
    thePacket = GetPacket_USB();

    if( thePacket->mPacketType == 0 &&
        thePacket->mPacketId == 6 )
        {
        break;
        }

    free( thePacket );
    }

free( thePacket );
return TRUE;
}

//-----------------------------------------------------------------------------
BOOL OpenGarminUSB (LPSTR ProductID)
{
Packet_t theProductDataPacket = { 20, 0, 0, 254, 0 , 0 };
Packet_t* thePacket = 0;
int	n=0;

InitializeUSB();

if( gHandle == 0 )
    {
//    printf( "%s", "No device" );
    return FALSE;
    }

// Tell the device to send product data
if (!SendPacket_USB(&theProductDataPacket ))
	return FALSE;

// Get the product data packet
for( ; ; )
    {
    thePacket = GetPacket_USB();
	if (!thePacket)
			break;
	MessageBox2 (0,"Hi",NULL,MB_OK);
	sprintf (str,"Got packet %i - %i",thePacket->mPacketType,thePacket->mPacketId);
	MessageBox2 (0,str,NULL,MB_OK);
    if( thePacket->mPacketType == 20 &&
        thePacket->mPacketId == 255 )
        {
			strcpy(ProductID, (char*) &thePacket->mData[4] );
        }

    free( thePacket );
	if (n++ > MaxWait)
		return FALSE;
    }

// Print out the product description
//strcpy(ProductID, (char*) &thePacket->mData[4] );

//free( thePacket );
return TRUE;
}

BOOL CloseGarminUSB (LPSTR NullArg)
{
	if (gHandle)
		CloseHandle (gHandle);
	sprintf (str,"Close handle %ld",(long)gHandle);
	MessageBox2 (0,str,NULL,MB_OK);
	gHandle = 0;
	return TRUE;
}

void ShowN(int n)
{
	char	str[32];

	itoa (n,str,10);
	SetWindowText (hWndMain,str);
	return;
}

DWORD GM32GetPacketUSB (LPSTR pPacket)
{
	Packet_t* thePacket = 0;
	int	n=0;

	if (!gHandle)
		return FALSE;
	for( ; ; )
    {
		thePacket = GetPacket_USB();
		if (!thePacket)
			return FALSE;
		if( thePacket->mPacketType == 20 || thePacket->mDataSize == 0)
			{
			break;
			}

		free( thePacket );
		n++;
//		ShowN (n); 
		if (n > MaxWait)
			return FALSE;
	}
	memmove (pPacket,thePacket,sizeof(Packet_t)+thePacket->mDataSize-1);
	free( thePacket );
	return TRUE;
}
#else
BOOL OpenGarminUSB (LPSTR ProductID)
{
	return FALSE;
}

DWORD GM32GetPacketUSB (LPSTR pPacket)
{
	return FALSE;
}
BOOL CloseGarminUSB (LPSTR NullArg)
{
	return FALSE;
}

BOOL SendPacketP (LPSTR pPacket)
{
	return FALSE;
}

#endif

