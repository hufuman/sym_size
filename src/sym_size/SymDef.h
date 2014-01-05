#pragma once



#include "XItemList.h"

class CUDTInfo
{
public:
    BSTR        bstrName;
    ULONGLONG   uLength;

    CString     strSize;
};
typedef XItemList<CUDTInfo> UDTInfoList;

class CFunctionInfo
{
public:
    BSTR        bstrName;
    ULONGLONG   uLength;

    CString     strSize;
};
typedef XItemList<CFunctionInfo> FuncInfoList;

