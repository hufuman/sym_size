#include "StdAfx.h"
#include "PEParser.h"


#include "MapFile.h"

CPEParser::CPEParser(void)
{
    m_bStop = FALSE;
    m_hThread = NULL;
    m_hWnd = NULL;
}

CPEParser::~CPEParser(void)
{
    Clear();
}

const FuncInfoList& CPEParser::GetFuncInfo()
{
    return m_FuncInfo;
}

BOOL CPEParser::Load(HWND hWnd, LPCTSTR szMapFile)
{
    Clear();

    m_strMapFile = szMapFile;
    m_hWnd = hWnd;

    m_bStop = FALSE;
    m_hThread = (HANDLE)_beginthreadex(0, 0, &CPEParser::ThreadProc, (void*)this, 0, 0);

    return (m_hThread != NULL);
}

void CPEParser::Stop()
{
    m_bStop = TRUE;
    if(m_hThread)
    {
        ::WaitForSingleObject(m_hThread, INFINITE);
        ::CloseHandle(m_hThread);
        m_hThread = NULL;
    }
}

void CPEParser::Clear()
{
    m_strMapFile = _T("");
    m_hWnd = NULL;
    m_FuncInfo.RemoveAll();
}

unsigned CPEParser::ThreadProc(void* pParam)
{
    CPEParser* pParser = static_cast<CPEParser*>(pParam);
    pParser->ParseImpl();
    return 0;
}

void CPEParser::ParseImpl()
{
    CMapFile mapFile;
    BOOL bResult = mapFile.LoadFile(m_strMapFile, m_FuncInfo);

    NotifyFinish(bResult);
}

void CPEParser::NotifyFinish(BOOL bResult)
{
    ::PostMessage(m_hWnd, g_NotifyPEParserFinish, bResult, 0);
}
