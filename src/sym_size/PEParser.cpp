#include "StdAfx.h"
#include "PEParser.h"


namespace
{
    template < typename T >
    void SafeRelease(T*& p)
    {
        if(p != NULL)
        {
            p->Release();
            p = NULL;
        }
    }
}

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

const UDTInfoList& CPEParser::GetUDTInfo()
{
    return m_UDTInfo;
}

const FuncInfoList& CPEParser::GetFuncInfo()
{
    return m_FuncInfo;
}

BOOL CPEParser::Load(HWND hWnd, LPCTSTR szPdbFile)
{
    Clear();

    m_strPDBFile = szPdbFile;
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
    m_strPDBFile = _T("");
    m_hWnd = NULL;

    DWORD dwLength = m_UDTInfo.GetLength();
    for(DWORD i=0; i<dwLength; ++ i)
    {
        CUDTInfo& udt = m_UDTInfo.GetAt(i);
        ::SysFreeString(udt.bstrName);
    }

    dwLength = m_FuncInfo.GetLength();
    for(DWORD i=0; i<dwLength; ++ i)
    {
        CFunctionInfo& func = m_FuncInfo.GetAt(i);
        ::SysFreeString(func.bstrName);
    }

    m_UDTInfo.RemoveAll();
    m_FuncInfo.RemoveAll();
}

HRESULT CPEParser::ParseUDT(IDiaEnumSymbols* pEnumSymbols)
{
    LONG lCount = 0;
    HRESULT hResult = pEnumSymbols->get_Count(&lCount);

    if(FAILED(hResult))
        return hResult;

    if(lCount == 0)
        return S_OK;

    m_UDTInfo.SetLength(lCount);

    IDiaSymbol* pSymbol = NULL;
    for(LONG i=0; i<lCount; ++ i)
    {
        hResult = pEnumSymbols->Item(i, &pSymbol);
        if(FAILED(hResult))
            break;

        CUDTInfo& udt = m_UDTInfo.GetAt(i);

        // Name
        hResult = pSymbol->get_undecoratedName(&udt.bstrName);
        if(S_OK != hResult)
            hResult = pSymbol->get_name(&udt.bstrName);

        if(S_OK != hResult)
            break;

        // Length
        hResult = pSymbol->get_length(&udt.uLength);
        if(FAILED(hResult))
            break;

        udt.strSize.Format(_T("%I64u"), udt.uLength);

        SafeRelease(pSymbol);

        NotifyProgress(50 + 50 * i / lCount);
    }

    SafeRelease(pSymbol);

    return hResult;
}

HRESULT CPEParser::ParseFunc(IDiaEnumSymbols* pEnumSymbols)
{
    LONG lCount = 0;
    HRESULT hResult = pEnumSymbols->get_Count(&lCount);

    if(FAILED(hResult))
        return hResult;

    if(lCount == 0)
        return S_OK;

    m_FuncInfo.SetLength(lCount);

    BSTR bstrName = NULL;
    IDiaSymbol* pSymbol = NULL;
    for(LONG i=0; i<lCount; ++ i)
    {
        hResult = pEnumSymbols->Item(i, &pSymbol);
        if(FAILED(hResult))
            break;

        CFunctionInfo& func = m_FuncInfo.GetAt(i);

        // Name
        hResult = pSymbol->get_undecoratedName(&func.bstrName);
        if(S_OK != hResult)
            hResult = pSymbol->get_name(&func.bstrName);

        if(S_OK != hResult)
            break;

        // Length
        hResult = pSymbol->get_length(&func.uLength);
        if(FAILED(hResult))
            break;

        func.strSize.Format(_T("%I64u"), func.uLength);

        SafeRelease(pSymbol);

        NotifyProgress(50 * i / lCount);
    }

    SafeRelease(pSymbol);

    return hResult;
}

unsigned CPEParser::ThreadProc(void* pParam)
{
    CPEParser* pParser = static_cast<CPEParser*>(pParam);
    pParser->ParseImpl();
    return 0;
}

void CPEParser::ParseImpl()
{
    HRESULT hr = E_FAIL;

    ::CoInitialize(NULL);

    IDiaDataSource* pSource = NULL;
    IDiaSession* pSession = NULL;
    IDiaSymbol* pGlobalScope = NULL;
    IDiaEnumSymbols* pEnumSymbols = NULL;

    for(;;)
    {
        // IDiaDataSource
        hr = ::CoCreateInstance(__uuidof(DiaSource),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IDiaDataSource),
            (void**)&pSource);
        if(FAILED(hr))
            break;

        // Load
        hr = pSource->loadDataFromPdb(m_strPDBFile);
        if(FAILED(hr))
            break;

        hr = pSource->openSession(&pSession);
        if(FAILED(hr))
            break;

        hr = pSession->get_globalScope(&pGlobalScope);
        if(FAILED(hr))
            break;

        hr = pSession->findChildren(pGlobalScope, SymTagFunction, NULL, nsNone, &pEnumSymbols);
        if(FAILED(hr))
            break;

        NotifyProgress(0);

        hr = ParseFunc(pEnumSymbols);
        pEnumSymbols->Release();
        if(FAILED(hr))
            break;

        hr = pSession->findChildren(pGlobalScope, SymTagUDT, NULL, nsNone, &pEnumSymbols);
        if(FAILED(hr))
            break;

        hr = ParseUDT(pEnumSymbols);
        pEnumSymbols->Release();
        if(FAILED(hr))
            break;

        NotifyProgress(100);

        break;
    }

    SafeRelease(pSource);
    SafeRelease(pSession);
    SafeRelease(pGlobalScope);

    ::CoUninitialize();

    NotifyFinish(hr);
}

void CPEParser::NotifyProgress(ULONG nProgress)
{
    ::PostMessage(m_hWnd, g_NotifyPEParserProgress, nProgress, 0);
}

void CPEParser::NotifyFinish(HRESULT hResult)
{
    ::PostMessage(m_hWnd, g_NotifyPEParserFinish, hResult, 0);
}
