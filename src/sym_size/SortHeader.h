#pragma once


#include "resource.h"

class CSortHeader : public CWindowImpl<CSortHeader, CHeaderCtrl>, public CCustomDraw<CSortHeader>
{
public:
    CSortHeader(void)
    {
        m_bSortAsc = FALSE;
        m_nSortIndex = -1;
        m_hBmp = (HBITMAP)::LoadImage(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BMP_ARROW), IMAGE_BITMAP, 0, 0, 0);
    }
    ~CSortHeader(void)
    {
        ;
    }

    BEGIN_MSG_MAP(CSortHeader)
        CHAIN_MSG_MAP_ALT(CCustomDraw<CSortHeader>, 1)
        REFLECTED_NOTIFY_CODE_HANDLER(HDN_ITEMCLICK, OnClicked)
    END_MSG_MAP()

public:

    void GetSortParam(BOOL& bAsc, int& nSortIndex)
    {
        bAsc = m_bSortAsc;
        nSortIndex = m_nSortIndex;
    }

    DWORD OnClicked(int Id, LPNMHDR pNMHDR, BOOL& bHandled)
    {
        NMHEADER* pHeader = reinterpret_cast<NMHEADER*>(pNMHDR);

        if(pHeader->iItem == m_nSortIndex)
        {
            m_bSortAsc = !m_bSortAsc;
        }
        else
        {
            m_bSortAsc = TRUE;
        }
        m_nSortIndex = pHeader->iItem;
        InvalidateRect(NULL);

        CWindow wndList = GetParent();
        ::SendMessage(wndList.GetParent(), WM_NOTIFY, (WPARAM)Id, (LPARAM)pNMHDR);
        return 0;
    }

    DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {
        return CDRF_NOTIFYITEMDRAW;
    }

    DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {
        return CDRF_NOTIFYPOSTPAINT;
    }

    DWORD OnItemPostPaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
    {
        const int nWidth = 16;
        const int nHeight = 15;
        if(lpNMCustomDraw->dwItemSpec == m_nSortIndex)
        {
            RECT& rc = lpNMCustomDraw->rc;
            rc.right -= 2;
            rc.left = rc.right - nWidth;

            rc.top = (rc.bottom - rc.top - nHeight) / 2;

            HDC hMemDC = ::CreateCompatibleDC(lpNMCustomDraw->hdc);

            HGDIOBJ hOldObj = ::SelectObject(hMemDC, m_hBmp);

            ::TransparentBlt(lpNMCustomDraw->hdc,
                rc.left, rc.top,
                nWidth, nHeight,
                hMemDC,
                m_bSortAsc ? nWidth : 0, 0,
                nWidth, nHeight,
                RGB(255, 0, 255));

            ::SelectObject(hMemDC, hOldObj);

            ::DeleteDC(hMemDC);
        }
        return CDRF_NOTIFYPOSTERASE;
    }

private:
    HBITMAP     m_hBmp;
    BOOL        m_bSortAsc;
    int         m_nSortIndex;
};
