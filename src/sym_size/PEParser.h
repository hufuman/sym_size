#pragma once


#include <dia2.h>
#include "SymDef.h"

// wParam: progress
const UINT g_NotifyPEParserProgress = ::RegisterWindowMessage(_T("NotifyPEParserProgress"));

// wParam: hResult
const UINT g_NotifyPEParserFinish = ::RegisterWindowMessage(_T("NotifyPEParserFinish"));

class CPEParser
{
public:
    CPEParser(void);
    ~CPEParser(void);

    const UDTInfoList& GetUDTInfo();
    const FuncInfoList& GetFuncInfo();

    BOOL Load(HWND hWnd, LPCTSTR szPdbFile);
    void Stop();

private:
    void Clear();

    HRESULT ParseUDT(IDiaEnumSymbols* pEnumSymbols);
    HRESULT ParseFunc(IDiaEnumSymbols* pEnumSymbols);

    void ParseImpl();

    static unsigned CALLBACK ThreadProc(void* pParam);

    void NotifyProgress(ULONG nProgress);
    void NotifyFinish(HRESULT hResult);

private:
    BOOL            m_bStop;
    HANDLE          m_hThread;

    CString         m_strPDBFile;
    UDTInfoList     m_UDTInfo;
    FuncInfoList    m_FuncInfo;

    HWND            m_hWnd;
};
