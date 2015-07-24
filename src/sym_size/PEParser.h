#pragma once


#include "SymDef.h"

// wParam: bResult
const UINT g_NotifyPEParserFinish = ::RegisterWindowMessage(_T("NotifyPEParserFinish"));

class CPEParser
{
public:
    CPEParser(void);
    ~CPEParser(void);

    const FuncInfoList& GetFuncInfo();

    BOOL Load(HWND hWnd, LPCTSTR szMapFile);
    void Stop();

private:
    void Clear();

    void ParseImpl();

    static unsigned CALLBACK ThreadProc(void* pParam);

    void NotifyProgress(ULONG nProgress);
    void NotifyFinish(BOOL bResult);

private:
    BOOL            m_bStop;
    HANDLE          m_hThread;

    CString         m_strMapFile;
    FuncInfoList    m_FuncInfo;

    HWND            m_hWnd;
};
