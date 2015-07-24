#pragma once

namespace Util
{

    // Enable drop for hWnd
    void EnableDrop(HWND hWnd);

    BOOL SaveStringToClipboard(HWND hWnd, LPCTSTR szData);
};
