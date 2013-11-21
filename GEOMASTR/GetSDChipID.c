/*HEADER FILE 
#ifndef _FLASHCARD_H 
#define _FLASHCARD_H 


class CFlashCard 
{ 
public: 
        enum { MAX_CARD_PATH = 128 }; 


        static BOOL isCardAvailable(); 
        static BOOL getSerialNumber(TCHAR* buf, UINT bufLength); 


        static TCHAR* getCardPath() { return m_CardPath; } 


protected: 
        static TCHAR m_CardPath[MAX_CARD_PATH]; 



}; 


#endif 

CPP FILE 
#include "stdafx.h" 
#include "FlashCard.h" 
#include <projects.h> 
#include <winioctl.h> 


// Include library Note_Prj.lib 
#pragma comment (lib, "Note_Prj.lib") 


//-------------------------------------------------------------------------­----- 
// 
// Static data members 
// 
//-------------------------------------------------------------------------­----- 
TCHAR CFlashCard::m_CardPath[CFlashCard::MAX_CARD_PATH]; 


// The list of known on-board storage names 
static const TCHAR* STORAGE[] = 
{ 
        _T("iPAQ File Store"), 
        _T("Built-in Storage") 


}; 


static const ULONG STORAGE_COUNT = sizeof(STORAGE) / sizeof(TCHAR*); 

//-------------------------------------------------------------------------­----- 
// 
// Used to see if a flash card is inserted into a socket.  Return true 
if card 
// is found, false otherwise.  Also, if found, sets card's path 
// 
//-------------------------------------------------------------------------­----- 
BOOL CFlashCard::isCardAvailable() 
{ 
        BOOL res = TRUE; 
        HANDLE hFlashCard = NULL; 
        WIN32_FIND_DATA find; 
        BOOL loop = TRUE; 
        BOOL found = FALSE; 
        UINT flashCardCount = 0; 


        // Look for the SD card. 
        memset(&find, 0, sizeof(WIN32_FIND_DATA)); 


        hFlashCard = ::FindFirstFlashCard(&find); 


        if(INVALID_HANDLE_VALUE != hFlashCard) 
        { 
                // We must enumerate the flash cards, since the dumb 
                // iPAQ file store is defined as a flash card 
                while(loop) 
                { 
                        // Only look at the flash card if it is a directory and temporary 
                        if(((find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 
FILE_ATTRIBUTE_DIRECTORY) && 
                ((find.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) == 
FILE_ATTRIBUTE_TEMPORARY)) 
                        { 
                                found = FALSE; 


                                for(ULONG i=0; i<STORAGE_COUNT; i++) 
                                { 
                                        if(_tcscmp(find.cFileName, STORAGE[i]) == 0) 
                                        { 
                                                found = TRUE; 
                                        } 
                                } 


                                // Only count the card if it is the correct size 
                                if(found == FALSE) 
                                { 
                                        // Save the name of the flash card 
                                        _tcsncpy(m_CardPath, find.cFileName, MAX_CARD_PATH); 
                                        flashCardCount++; 
                                } 
                        } 


                        // Next flash card 
                        loop = ::FindNextFlashCard(hFlashCard, &find); 
                } 


                ::FindClose (hFlashCard); 


                // If no flash cards were found in the enumeration, then leave 
                if(flashCardCount == 0) 
                { 
                        res = FALSE; 
                } 
        } 
        else 
        { 
                // No flash cards found 
                _stprintf(m_CardPath, _T("ERR: %d"), ::GetLastError()); 
                res = FALSE; 
        } 


        return res; 



} 


//-------------------------------------------------------------------------­----- 
// 
// Get the flash card serial number and save in passed in buffer. 
// 
//-------------------------------------------------------------------------­----- 
BOOL CFlashCard::getSerialNumber(TCHAR* buf, UINT bufLength) 
{ 
        // These values are only defined in Platform Builder, so we have to 
        // redefine them here 
        #define IOCTL_DISK_BASE           FILE_DEVICE_DISK 
        #define IOCTL_DISK_GET_STORAGEID  CTL_CODE(IOCTL_DISK_BASE, 0x709, \ 
                                                                                           METHOD_BUFFERED, FILE_ANY_ACCESS) 
        #define MANUFACTUREID_INVALID     0x01 
        #define SERIALNUM_INVALID         0x02 

        // This structure is only defined in Platform Builder, so we have to 
        // redefine it here 
        typedef struct _STORAGE_IDENTIFICATION 
        { 
                DWORD    dwSize; 
                DWORD    dwFlags; 
                DWORD    dwManufactureIDOffest; 
                DWORD    dwSerialNumOffset; 
        } STORAGE_IDENTIFICATION, *PSTORAGE_IDENTIFICATION; 


        const UINT BBUF_LENGTH = 256; 
        const UINT STR_LENGTH = 256; 
        const UINT SERIAL_NUMBER_LENGTH = 8; 
        BOOL res = TRUE; 
        byte bbuf[BBUF_LENGTH]; 
        TCHAR str[STR_LENGTH]; 
        STORAGE_IDENTIFICATION* si; 
        HANDLE hCard = NULL; 
        ULONG dwNumReturned = 0; 
        ULONG err = 0; 


        // Make sure a card is inserted and found before attempting to read 
        // the serial number 
//        if(isCardAvailable() == TRUE) 
        { 
                // Clear the buffer 
                memset(bbuf, 0, BBUF_LENGTH); 


                // Clear the buffer 
                memset(buf, 0, bufLength * sizeof(TCHAR)); 


                // Set a STORAGE_IDENTIFICATION pointer to the buffer (overlaying the structure) 
                si = (STORAGE_IDENTIFICATION*)bbuf; 


                // Create the volume name of the flash card 
                _stprintf(str, _T("\\%s\\Vol:"), m_CardPath); 


                // Get a handle to the flash card (drive) volume 
                hCard = ::CreateFile(str, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL); 


                // Set the size element of the STORAGE_IDENTIFICATION structure 
                si->dwSize = BBUF_LENGTH; 


                // Fill the STORAGE_IDENTIFICATION structure with the flash card info 
                if(DeviceIoControl(hCard, IOCTL_DISK_GET_STORAGEID, (LPVOID)NULL, 0, 
                                                     si, BBUF_LENGTH, &dwNumReturned, NULL) == FALSE) 
                { 
                        err = ::GetLastError(); 
                        res = FALSE; 
                } 


                // Close the handle 
                ::CloseHandle (hCard); 


                // See if the serial number is valid.  If not, return false, otherwise 
get 
                // the serial number 
                if((si->dwFlags & SERIALNUM_INVALID) != SERIALNUM_INVALID) 
                { 
                        // Get a char pointer to the serial number in the byte array 
                        CHAR* sn = (CHAR*)((UINT)si + si->dwSerialNumOffset); 


                        // Covert the CHAR serial number to a TCHAR string 
                        mbstowcs(buf, sn, SERIAL_NUMBER_LENGTH); 
                        buf[SERIAL_NUMBER_LENGTH] = 0; 
                } 
                else 
                { 
                        // Serial number is not available 
                        _tcscpy(buf, _T("! NONE !")); 
                        res = FALSE; 
                } 
        } 
        else 
        { 
                // No card 
                _tcscpy(buf, _T("NO CARD")); 
                res = FALSE; 
        } 


        return res; 



- Hide quoted text -*/
