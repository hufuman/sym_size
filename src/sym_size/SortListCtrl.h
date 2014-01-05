#pragma once

class CSortListCtrl : public CWindowImpl<CSortListCtrl, CListViewCtrl>, public CCustomDraw<CSortListCtrl>
{
public:
    CSortListCtrl(void)
    {
        m_bSortAsc = FALSE;
        m_nSortIndex = -1;
        m_hBmp = (HBITMAP)::LoadImage(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BMP_ARROW), IMAGE_BITMAP, 0, 0, 0);
    }
    ~CSortListCtrl(void)
    {
        ;
    }

    BEGIN_MSG_MAP(CSortListCtrl)
        NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
        NOTIFY_CODE_HANDLER(HDN_ITEMCLICK, OnColumnClicked)

    END_MSG_MAP()

    LRESULT OnColumnClicked(int nId, LPNMHDR pNMHDr, BOOL& bHandled)
    {
        bHandled = FALSE;
        NMHEADER* pHeader = reinterpret_cast<NMHEADER*>(pNMHDr);

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
        ::SendMessage(wndList.GetParent(), WM_NOTIFY, (WPARAM)nId, (LPARAM)pNMHDr);
        return 0;
    }

    void GetSortParam(BOOL& bAsc, int& nSortIndex)
    {
        bAsc = m_bSortAsc;
        nSortIndex = m_nSortIndex;
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
        if(lpNMCustomDraw->dwItemSpec == m_nSortIndex && GetItemCount() > 0)
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
