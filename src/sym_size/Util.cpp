#include "StdAfx.h"
#include "Util.h"

#include <ShlObj.h>
#include <ShellAPI.h>

namespace Util
{
    namespace
    {
        // Allow/disallow window of low right to send message to window of high right.
        BOOL FilterWindowMessage(UINT message, DWORD dwValue)
        {
            typedef BOOL (WINAPI FAR *ChangeWindowMessageFilter_PROC)(UINT,DWORD);

            static ChangeWindowMessageFilter_PROC pfnChangeWindowMessageFilter
                = (ChangeWindowMessageFilter_PROC)::GetProcAddress (::GetModuleHandle(_T("USER32")), "ChangeWindowMessageFilter");

            BOOL bResult = TRUE;
            if(pfnChangeWindowMessageFilter != NULL)
                bResult = pfnChangeWindowMessageFilter(message, dwValue);
            return bResult;
        }
    }
    void EnableDrop(HWND hWnd)
    {
        ::DragAcceptFiles(hWnd, TRUE);

        FilterWindowMessage(0x0049 /*WM_COPYGLOBALDATA*/, 1);
        FilterWindowMessage(WM_DROPFILES, 1);
    }

    BOOL SaveStringToClipboard(HWND hWnd, LPCTSTR szData)
    {
        if(!::OpenClipboard(hWnd))
            return FALSE;

        ::EmptyClipboard();

        DWORD dwLength = (_tcslen(szData) + 1) * sizeof(TCHAR);
        HANDLE hData = ::GlobalAlloc(GMEM_MOVEABLE, dwLength);
        if(hData == NULL)
        {
            ::CloseClipboard();
            return FALSE;
        }

        LPVOID pCopy = ::GlobalLock(hData); 
        memcpy(pCopy, szData, dwLength);
        ::GlobalUnlock(hData);

        ::SetClipboardData(CF_UNICODETEXT, hData);
        ::CloseClipboard();
        return TRUE;
    }
};
