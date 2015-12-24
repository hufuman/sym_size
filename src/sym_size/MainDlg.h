// maindlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////


#pragma once


#include "AboutDlg.h"
#include "Util.h"
#include "WndLayout.h"
#include "PEParser.h"
#include "SortListCtrl.h"


class CMainDlg;
struct stSortInfo
{
    BOOL bAsc;
    int  nSortIndex;
    CMainDlg* pDlg;
};
extern stSortInfo g_SortInfo;

class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter
{
public:
	enum { IDD = IDD_DIALOG_MAIN };

	CMainDlg()
	{
        m_nTotalSize = 0;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return ::IsDialogMessage(m_hWnd, pMsg);
	}

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
        MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)

        MESSAGE_HANDLER(g_NotifyPEParserFinish, OnNotifyPEParserFinish)

        NOTIFY_HANDLER(IDC_LIST_SYMBOLE, LVN_GETDISPINFO, OnLvnGetDispInfo)

        COMMAND_ID_HANDLER(IDOK, OnFilter)

        NOTIFY_HANDLER(IDC_LIST_SYMBOLE, NM_RCLICK, OnItemRightClicked)
        NOTIFY_HANDLER(IDC_LIST_SYMBOLE, LVN_ITEMCHANGED, OnItemChanged)
        NOTIFY_HANDLER(IDC_LIST_SYMBOLE, LVN_ODSTATECHANGED, OnOdStateChanged)

        NOTIFY_HANDLER(0, HDN_ITEMCLICK, OnListHeaderClicked)

        COMMAND_ID_HANDLER(ID_ITEMCONTEXTMENU_COPYNAME, OnItemCopyName)
        COMMAND_ID_HANDLER(ID_ITEMCONTEXTMENU_COPYLINE, OnItemCopyLine)

    END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

		// Add "About..." menu item to system menu.

		// IDM_ABOUTBOX must be in the system command range.
		_ASSERTE((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
		_ASSERTE(IDM_ABOUTBOX < 0xF000);

		CMenu SysMenu = GetSystemMenu(FALSE);
		if(::IsMenu(SysMenu))
		{
			TCHAR szAboutMenu[256];
			if(::LoadString(_Module.GetResourceInstance(), IDS_ABOUTBOX, szAboutMenu, 255) > 0)
			{
				SysMenu.AppendMenu(MF_SEPARATOR);
				SysMenu.AppendMenu(MF_STRING, IDM_ABOUTBOX, szAboutMenu);
			}
		}
		SysMenu.Detach();

		// register object for message filtering
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);

        // 
        Util::EnableDrop(m_hWnd);

        m_List.SubclassWindow(GetDlgItem(IDC_LIST_SYMBOLE));

        m_Progress.Attach(GetDlgItem(IDC_PROGRESS_PARSING));
        m_LabelProgress.Attach(GetDlgItem(IDC_LABEL_PROGRESS));

        m_List.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 120);
        m_List.InsertColumn(1, _T("Size"), LVCFMT_LEFT, 120);
        m_List.InsertColumn(2, _T("Module"), LVCFMT_LEFT, 120);
        m_List.InsertColumn(3, _T("Obj"), LVCFMT_LEFT, 120);
        m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

        m_Progress.SetRange(0, 100);

        SetDlgItemText(IDC_EDIT_FILTER, _T("*"));

        ShowResultUI(TRUE);

        InitLayout();

		return TRUE;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

    LRESULT OnFilter(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        FilterResult();
        return 0;
    }

    LRESULT OnItemCopyLine(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
    {
        bHandled = TRUE;

        const FuncInfoList& funcList = m_PEParser.GetFuncInfo();

        CString strMsg, strTemp;
        LVITEMINDEX index = {-1, 0};
        while(m_List.GetNextItemIndex(&index, LVNI_ALL | LVNI_SELECTED))
        {
            const stFuncData& func = funcList[m_ItemIndex[index.iItem]];
            strTemp.Format(_T("%I64u"), func.nFuncSize);
            strMsg += func.strFuncName;
            strMsg += _T(", ");
            strMsg += strTemp;
            strMsg += _T(", ");
            strMsg += func.strModuleName;
            strMsg += _T(", ");
            strMsg += func.strObjName;
            strMsg += _T("\r\n");
        }

        if(!strMsg.IsEmpty())
            strMsg = strMsg.Mid(0, strMsg.GetLength() - 2);

        Util::SaveStringToClipboard(m_hWnd, strMsg);

        return 0;
    }

    LRESULT OnItemCopyName(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
    {
        bHandled = TRUE;

        const FuncInfoList& funcList = m_PEParser.GetFuncInfo();

        CString strMsg;
        LVITEMINDEX index = {-1, 0};
        while(m_List.GetNextItemIndex(&index, LVNI_ALL | LVNI_SELECTED))
        {
            const stFuncData& func = funcList[m_ItemIndex[index.iItem]];
            strMsg += func.strFuncName;
            strMsg += _T(", ");
        }

        if(!strMsg.IsEmpty())
            strMsg = strMsg.Mid(0, strMsg.GetLength() - 2);

        Util::SaveStringToClipboard(m_hWnd, strMsg);

        return 0;
    }

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

	LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		UINT uCmdType = (UINT)wParam;

		if((uCmdType & 0xFFF0) == IDM_ABOUTBOX)
		{
			CAboutDlg dlg;
			dlg.DoModal();
		}
		else
			bHandled = FALSE;

		return 0;
	}

    LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = TRUE;
        HDROP hDrop = reinterpret_cast<HDROP>(wParam);

        TCHAR szPath[MAX_PATH * 2] = {0};

        ShowResultUI(FALSE);

        for(;;)
        {
            UINT uCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, szPath, _countof(szPath));
            if(uCount == 0)
            {
                MsgBox(IDS_ERR_NO_FILE_DROPPED);
                break;
            }
            else if(uCount > 1)
            {
                MsgBox(IDS_ERR_MORE_THAN_ONE_FILE);
                break;
            }

            if(::DragQueryFile(hDrop, 0, szPath, _countof(szPath)) == 0)
            {
                MsgBox(IDS_ERR_UNKNOWN);
                break;
            }

            DWORD dwAttr = ::GetFileAttributes(szPath);
            if(dwAttr == INVALID_FILE_ATTRIBUTES
                || ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                || !::PathMatchSpec(szPath, _T("*.map")))
            {
                MsgBox(IDS_ERR_UNKNOWN);
                break;
            }

            SetDlgItemText(IDC_LABEL_FILE_PATH, szPath);
            SetDlgItemText(IDC_EDIT_FILTER, _T("*"));

            ParseMap(szPath);

            break;
        }

        ::DragFinish(hDrop);
        return 0;
    }

    // Layout
    void InitLayout()
    {
        m_WndLayout.Init(m_hWnd);
        m_WndLayout.AddControlById(IDC_LABEL_STEP1, Layout_Top | Layout_Left);
        m_WndLayout.AddControlById(IDC_LABEL_STEP2, Layout_Top | Layout_Left);
        m_WndLayout.AddControlById(IDC_LIST_SYMBOLE, Layout_VFill | Layout_HFill);
        m_WndLayout.AddControlById(IDC_LABEL_FILE_PATH, Layout_Top | Layout_HFill);
        m_WndLayout.AddControlById(IDC_EDIT_FILTER, Layout_Top | Layout_HFill);
        m_WndLayout.AddControlById(IDOK, Layout_Top | Layout_Right);

        m_WndLayout.AddControlById(IDC_LABEL_PROGRESS, Layout_Top | Layout_HCenter);
        m_WndLayout.AddControlById(IDC_PROGRESS_PARSING, Layout_Top | Layout_HCenter);
    }

    int MsgBox(UINT nResId)
    {
        CString strMsg;
        strMsg.LoadString(nResId);
        return ::MessageBox(m_hWnd, strMsg, _T("sym_size"), MB_OK | MB_ICONERROR);
    }

    void ParseMap(LPCTSTR szMapPath)
    {
        m_List.ShowWindow(SW_HIDE);

        m_Progress.SetPos(0);
        m_LabelProgress.SetWindowText(_T("Progress: 0%"));

        m_Progress.ShowWindow(SW_SHOW);
        m_LabelProgress.ShowWindow(SW_SHOW);

        m_PEParser.Load(m_hWnd, szMapPath);
    }

    LRESULT OnNotifyPEParserFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = TRUE;

        if(SUCCEEDED(wParam))
        {
            m_Progress.SetPos(100);
            m_LabelProgress.SetWindowText(_T("Progress: 100%"));

            ShowResult();
        }
        else
        {
            MsgBox(IDS_ERR_PARSE);
        }

        return 0;
    }

    void ShowResultUI(BOOL bShowResult)
    {
        m_List.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);

        m_Progress.ShowWindow(bShowResult ? SW_HIDE : SW_SHOW);
        m_LabelProgress.ShowWindow(bShowResult ? SW_HIDE : SW_SHOW);

    }

    void ShowResult()
    {
        ShowResultUI(TRUE);

        FilterResult();
    }

    void FilterResult()
    {
        m_ItemIndex.RemoveAll();

        TCHAR szFilter[MAX_PATH * 2] = {0};
        GetDlgItemText(IDC_EDIT_FILTER, szFilter, MAX_PATH * 2);
        szFilter[MAX_PATH * 2 - 1] = 0;

        CString strFilter;
        strFilter.Format(_T("*%s*"), szFilter);

        int nTotalSize = 0;
        const FuncInfoList& list = m_PEParser.GetFuncInfo();
        DWORD dwLength = list.GetSize();
        for(DWORD i=0; i<dwLength; ++ i)
        {
            const stFuncData& func = list[i];
            if(::PathMatchSpec(func.strFuncName, strFilter))
            {
                nTotalSize += func.nFuncSize;
                m_ItemIndex.Add(i);
            }
        }

        SortListItems();

        m_nTotalSize = nTotalSize;
        UpdateInfoLabel();
    }

    void SortListItems()
    {
        BOOL bAsc = TRUE;
        int nSortIndex = -1;
        m_List.GetSortParam(bAsc, nSortIndex);

        g_SortInfo.bAsc = bAsc;
        g_SortInfo.nSortIndex = nSortIndex;
        g_SortInfo.pDlg = this;

        qsort(m_ItemIndex.GetData(), m_ItemIndex.GetSize(), sizeof(DWORD), &CMainDlg::SortHlpFunc);

        m_List.SetItemCount(m_ItemIndex.GetSize());
    }

    LRESULT OnOdStateChanged(int nId, LPNMHDR pNMHDr, BOOL& bHandled)
    {
        NMLVODSTATECHANGE* pItems = (NMLVODSTATECHANGE*)pNMHDr;
        if((pItems->uNewState & LVIS_SELECTED) ^ (pItems->uOldState & LVIS_SELECTED))
        {
            UpdateInfoLabel();
        }
        return 0;
    }

    LRESULT OnItemChanged(int nId, LPNMHDR pNMHDr, BOOL& bHandled)
    {
        NMLISTVIEW* pItem = (NMLISTVIEW*)pNMHDr;
        if((pItem->uNewState & LVIS_SELECTED) ^ (pItem->uOldState & LVIS_SELECTED))
        {
            UpdateInfoLabel();
        }
        return 0;
    }

    LRESULT OnItemRightClicked(int nId, LPNMHDR pNMHDr, BOOL& bHandled)
    {
        HMENU hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MENU_ITEM));
        HMENU hSubMenu = ::GetSubMenu(hMenu, 0);

        POINT Pt;
        ::GetCursorPos(&Pt);
        ::TrackPopupMenu(hSubMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Pt.x, Pt.y, 0, m_hWnd, NULL);

        bHandled = TRUE;
        return 0;
    }

    LRESULT OnListHeaderClicked(int nId, LPNMHDR pNMHDr, BOOL& bHandled)
    {
        SortListItems();
        bHandled = TRUE;
        return 0;
    }

    LRESULT OnLvnGetDispInfo(int nId, LPNMHDR pNMHDr, BOOL& bHandled)
    {
        NMLVDISPINFO* pInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDr);

        const FuncInfoList& list = m_PEParser.GetFuncInfo();
        const stFuncData& func = list[m_ItemIndex[pInfo->item.iItem]];
        if(pInfo->item.iSubItem == 0)
        {
            pInfo->item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(func.strFuncName));
        }
        else if(pInfo->item.iSubItem == 1)
        {
            pInfo->item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(func.strFuncSize));
        }
        else if(pInfo->item.iSubItem == 2)
        {
            pInfo->item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(func.strModuleName));
        }
        else if(pInfo->item.iSubItem == 3)
        {
            pInfo->item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(func.strObjName));
        }

        pInfo->item.mask |= LVIF_PARAM;
        pInfo->item.lParam = m_ItemIndex[pInfo->item.iItem];

        return 0;
    }

    static int SortHlpFunc(const void *arg1, const void *arg2)
    {
        int nResult = 0;

        DWORD dwIndex1 = *reinterpret_cast<DWORD*>(reinterpret_cast<DWORD>(arg1));
        DWORD dwIndex2 = *reinterpret_cast<DWORD*>(reinterpret_cast<DWORD>(arg2));

        const FuncInfoList& list = g_SortInfo.pDlg->m_PEParser.GetFuncInfo();
        const stFuncData& func1 = list[dwIndex1];
        const stFuncData& func2 = list[dwIndex2];
        if(g_SortInfo.nSortIndex == 0)
        {
            nResult = _tcsicmp(func1.strFuncName, func2.strFuncName);
        }
        else if(g_SortInfo.nSortIndex == 1)
        {
            if(func1.nFuncSize < func2.nFuncSize)
                nResult = -1;
            else if(func1.nFuncSize == func2.nFuncSize)
                nResult = 0;
            else
                nResult = 1;
        }
        else if(g_SortInfo.nSortIndex == 2)
        {
            nResult = _tcsicmp(func1.strModuleName, func2.strModuleName);
        }
        else if(g_SortInfo.nSortIndex == 3)
        {
            nResult = _tcsicmp(func1.strObjName, func2.strObjName);
        }
        if(!g_SortInfo.bAsc)
            nResult = -nResult;
        return nResult;
    }

    void UpdateInfoLabel()
    {
        int nSelectionSize = 0;
        const FuncInfoList& funcList = m_PEParser.GetFuncInfo();
        LVITEMINDEX index = {-1, 0};
        while(m_List.GetNextItemIndex(&index, LVNI_SELECTED))
        {
            const stFuncData& func = funcList[m_ItemIndex[index.iItem]];
            nSelectionSize += func.nFuncSize;
        }

        int nSelectionCount = m_List.GetItemCount();
        CString strInfo;
        if(nSelectionSize < 10 * 1024)
            strInfo.Format(_T("TotalSize: %d B, Size of selection: %d B, TotalCount: %d"), m_nTotalSize, nSelectionSize, nSelectionCount);
        else if(nSelectionSize < 100 * 1024 * 1024)
            strInfo.Format(_T("TotalSize: %d KB, Size of selection: %d KB, TotalCount: %d"), m_nTotalSize / 1024, nSelectionSize / 1024, nSelectionCount);
        else if(nSelectionSize < 100 * 1024 * 1024 * 1024)
            strInfo.Format(_T("TotalSize: %d MB, Size of selection: %d MB, TotalCount: %d"), m_nTotalSize / 1024 / 1024, nSelectionSize / 1024 / 1024, nSelectionCount);
        SetDlgItemText(IDC_LABEL_INFO, strInfo);
    }

private:
    CWndLayout      m_WndLayout;
    CPEParser       m_PEParser;

    int             m_nTotalSize;

    CProgressBarCtrl m_Progress;
    CStatic         m_LabelProgress;

    CSortListCtrl   m_List;

    ATL::CSimpleArray<DWORD> m_ItemIndex;
};
