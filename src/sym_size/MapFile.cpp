#include "StdAfx.h"
#include "MapFile.h"

CMapFile::CMapFile(void)
{
}

CMapFile::~CMapFile(void)
{
}

bool CMapFile::LoadFile(LPCTSTR szFilePath, FuncInfoList& listFuncDatas)
{
    HANDLE hFile = ::CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(hFile == INVALID_HANDLE_VALUE)
        return false;

    const int pointerLength = 8;
    bool result = false;
    for(;;)
    {
        // Read File
        DWORD dwSize = ::GetFileSize(hFile, NULL);
        LPSTR pFileData = (LPSTR)::malloc(dwSize + 1);
        LPSTR pFileEnd = pFileData + dwSize;
        DWORD dwRead;
        if(!::ReadFile(hFile, (LPVOID)pFileData, dwSize, &dwRead, NULL) || dwRead != dwSize)
            break;

        pFileData[dwSize - 1] = 0;

        // Find begin of function datas
        LPSTR pLine = strstr(pFileData, "Lib:Object");
        if(pLine == NULL)
            break;

        // Replace \r\n with \0
        for(DWORD i=0; i<dwSize; ++ i)
        {
            if(pFileData[i] == '\r' || pFileData[i] == '\n')
                pFileData[i] = 0;
        }

        char buffer[1024];
        while((pLine = NextLine(pLine, pFileEnd)) != NULL)
        {
            stFuncData data;
            data.nFuncSize = 0;
            data.strFuncSize = "0";

            if(strstr(pLine, "entry point at") != NULL)
            {
                result = true;
                break;
            }

            // 0005:000002b0       __imp___CRT_RTC_INITW      004182b0     MSVCRTD:MSVCR90D.dll

            // Section No
            data.nSectionNo = atoi(pLine);

            pLine = strchr(pLine, ':');
            if(pLine == NULL)
                break;

            // FuncName
            pLine = strtok(pLine, " ");
            if(pLine == NULL)
                break;
            pLine = (LPSTR)strtok(NULL, " ");
            if(pLine == NULL)
                break;

            data.strFuncName = pLine;
            if(data.strFuncName == _T("__TI2PAD"))
                printf("");

            // DecodedFuncName
#ifdef UnDecorateSymbolName
#undef UnDecorateSymbolName
#endif // UnDecorateSymbolName
            if(UnDecorateSymbolName(pLine, buffer, _countof(buffer), 
                UNDNAME_NO_LEADING_UNDERSCORES | UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_FUNCTION_RETURNS | UNDNAME_NO_ALLOCATION_MODEL | UNDNAME_NO_ALLOCATION_LANGUAGE | UNDNAME_NO_THISTYPE | UNDNAME_NO_ACCESS_SPECIFIERS | UNDNAME_NO_THROW_SIGNATURES | UNDNAME_NO_MEMBER_TYPE | UNDNAME_NO_RETURN_UDT_MODEL | UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY))
            {
                data.strFuncName = buffer;
            }

            // RVA
            pLine = strtok(NULL, " ");
            if(pLine == NULL)
                break;

            // FuncAddr
            sscanf(pLine, "%x", &data.pFuncAddr);

            if(listFuncDatas.GetSize() > 0)
            {
                stFuncData& lastFuncData = listFuncDatas[listFuncDatas.GetSize() - 1];
                if(lastFuncData.nSectionNo == data.nSectionNo && lastFuncData.nFuncSize <= 0)
                {
                    lastFuncData.nFuncSize = data.pFuncAddr - lastFuncData.pFuncAddr;
                    lastFuncData.strFuncSize.Format(_T("%d"), lastFuncData.nFuncSize);
                }
            }

            // find last token
            LPSTR lastToken = pLine;
            while(pLine != NULL)
            {
                lastToken = pLine;
                pLine = strtok(NULL, " ");
            }
            pLine = lastToken;

            // Module:Obj
            LPCSTR pos = strchr(pLine, ':');
            if(pos == NULL)
            {
                data.strObjName = pLine;
            }
            else
            {
                CString tmp(pLine, pos - pLine);
                data.strModuleName = tmp;
                data.strObjName = pos + 1;
            }

            listFuncDatas.Add(data);
        }

        break;
    }
    ::CloseHandle(hFile);
    return result;
}

LPSTR CMapFile::NextLine(LPSTR pCurPos, LPSTR pFileEnd)
{
    LPSTR result = strchr(pCurPos, '\0');
    if(result == NULL)
        return NULL;
    for(;(result[0] == _T('\n') || result[0] == _T('\0')) && result < pFileEnd; ++ result)
    {
    }
    return result;
}
