#pragma once

struct stFuncData
{
    int     nSectionNo;
    int     pFuncAddr;
    CString strFuncName;
    int     nFuncSize;
    CString strModuleName;
    CString strObjName;
    CString strFuncSize;
};

typedef CSimpleArray<stFuncData> FuncInfoList;
