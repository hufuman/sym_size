#pragma once

enum FuncAttr
{
    FuncAttrFlat        = 0x0001,
    FuncAttrPublic      = 0x0002,
    FuncAttrPrivate     = 0x0004,
    FuncAttrProtected   = 0x0008,
    FuncAttrThisCall    = 0x0010,
    FuncAttrVirtual     = 0x0020,
};

struct stFuncData
{
    int     nSectionNo;
    int     pFuncAddr;
    CString strFuncName;
    int     nFuncSize;
    CString strModuleName;
    CString strObjName;
    CString strFuncSize;
    DWORD   dwAttr; // FuncAttr
};

typedef CSimpleArray<stFuncData> FuncInfoList;
