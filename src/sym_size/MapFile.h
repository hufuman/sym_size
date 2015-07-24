#pragma once

#include "SymDef.h"

class CMapFile
{
public:
    CMapFile(void);
    ~CMapFile(void);

    bool LoadFile(LPCTSTR szFilePath, FuncInfoList& listFuncDatas);

private:
    LPSTR NextLine(LPSTR pCurPos, LPSTR pFileEnd);
};
