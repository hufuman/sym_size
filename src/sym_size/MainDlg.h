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
    BOOL bShowFunc;
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

        MESSAGE_HANDLER(g_NotifyPEParserProgress, OnNotifyPEParserProgress)
        MESSAGE_HANDLER(g_NotifyPEParserFinish, OnNotifyPEParserFinish)

        NOTIFY_HANDLER(IDC_LIST_SYMBOLE, LVN_GETDISPINFO, OnLvnGetDispInfo)

        COMMAND_ID_HANDLER(IDC_RADIO_FUNCTION, OnFilter)
        COMMAND_ID_HANDLER(IDC_RADIO_UDT, OnFilter)
        COMMAND_ID_HANDLER(IDC_BTN_FILTER, OnFilter)

        NOTIFY_HANDLER(0, HDN_ITEMCLICK, OnListHeaderClicked)

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
        m_RadioFunc.Attach(GetDlgItem(IDC_RADIO_FUNCTION));
        m_RadioUDT.Attach(GetDlgItem(IDC_RADIO_UDT));

        m_List.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 120);
        m_List.InsertColumn(1, _T("Size"), LVCFMT_LEFT, 360);
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
                || !::PathMatchSpec(szPath, _T("*.pdb")))
            {
                MsgBox(IDS_ERR_UNKNOWN);
                break;
            }

            SetDlgItemText(IDC_LABEL_FILE_PATH, szPath);
            SetDlgItemText(IDC_EDIT_FILTER, _T("*"));

            ParsePDB(szPath);

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
        m_WndLayout.AddControlById(IDC_BTN_FILTER, Layout_Top | Layout_Right);

        m_WndLayout.AddControlById(IDC_LABEL_PROGRESS, Layout_Top | Layout_HCenter);
        m_WndLayout.AddControlById(IDC_PROGRESS_PARSING, Layout_Top | Layout_HCenter);

        m_WndLayout.AddControlById(IDC_RADIO_FUNCTION, Layout_Top | Layout_Left);
        m_WndLayout.AddControlById(IDC_RADIO_UDT, Layout_Top | Layout_Left);
    }

    int MsgBox(UINT nResId)
    {
        CString strMsg;
        strMsg.LoadString(nResId);
        return ::MessageBox(m_hWnd, strMsg, _T("sym_size"), MB_OK | MB_ICONERROR);
    }

    void ParsePDB(LPCTSTR szPDBPath)
    {
        m_List.ShowWindow(SW_HIDE);

        m_Progress.SetPos(0);
        m_LabelProgress.SetWindowText(_T("Progress: 0%"));

        m_Progress.ShowWindow(SW_SHOW);
        m_LabelProgress.ShowWindow(SW_SHOW);

        m_PEParser.Load(m_hWnd, szPDBPath);
    }

    LRESULT OnNotifyPEParserProgress(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = TRUE;

        m_Progress.SetPos(wParam);

        CString strText;
        strText.Format(_T("Progress: %d%%"), wParam);
        m_LabelProgress.SetWindowText(strText);

        return 0;
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
        m_RadioFunc.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);
        m_RadioUDT.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);

        m_Progress.ShowWindow(bShowResult ? SW_HIDE : SW_SHOW);
        m_LabelProgress.ShowWindow(bShowResult ? SW_HIDE : SW_SHOW);

    }

    void ShowResult()
    {
        ShowResultUI(TRUE);

        m_RadioFunc.SetCheck(BST_CHECKED);

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
        if(m_RadioFunc.GetCheck() == BST_CHECKED)
        {
            const FuncInfoList& list = m_PEParser.GetFuncInfo();
            DWORD dwLength = list.GetLength();
            for(DWORD i=0; i<dwLength; ++ i)
            {
                const CFunctionInfo& func = list.GetAt(i);
                if(::PathMatchSpec(func.bstrName, strFilter))
                {
                    m_ItemIndex.Add(i);
                }
            }
        }
        else if(m_RadioUDT.GetCheck() == BST_CHECKED)
        {
            const UDTInfoList& list = m_PEParser.GetUDTInfo();
            DWORD dwLength = list.GetLength();
            for(DWORD i=0; i<dwLength; ++ i)
            {
                const CUDTInfo& udt = list.GetAt(i);
                if(::PathMatchSpec(udt.bstrName, strFilter))
                {
                    m_ItemIndex.Add(i);
                }
            }
        }

        SortListItems();
    }

    void SortListItems()
    {
        BOOL bAsc = TRUE;
        int nSortIndex = -1;
        m_List.GetSortParam(bAsc, nSortIndex);

        g_SortInfo.bShowFunc = (m_RadioFunc.GetCheck() == BST_CHECKED);
        g_SortInfo.bAsc = bAsc;
        g_SortInfo.nSortIndex = nSortIndex;
        g_SortInfo.pDlg = this;

        qsort(m_ItemIndex.GetData(), m_ItemIndex.GetSize(), sizeof(DWORD), &CMainDlg::SortHlpFunc);

        m_List.SetItemCount(m_ItemIndex.GetSize());
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

        if(m_RadioFunc.GetCheck() == BST_CHECKED)
        {
            const FuncInfoList& list = m_PEParser.GetFuncInfo();
            const CFunctionInfo& func = list.GetAt(m_ItemIndex[pInfo->item.iItem]);
            if(pInfo->item.iSubItem == 0)
            {
                pInfo->item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(func.bstrName));
            }
            else
            {
                pInfo->item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(func.strSize));
            }
        }
        else if(m_RadioUDT.GetCheck() == BST_CHECKED)
        {
            const UDTInfoList& list = m_PEParser.GetUDTInfo();
            const CUDTInfo& udt = list.GetAt(m_ItemIndex[pInfo->item.iItem]);
            if(pInfo->item.iSubItem == 0)
            {
                pInfo->item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(udt.bstrName));
            }
            else
            {
                pInfo->item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(udt.strSize));
            }
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

        if(g_SortInfo.bShowFunc)
        {
            const FuncInfoList& list = g_SortInfo.pDlg->m_PEParser.GetFuncInfo();
            const CFunctionInfo& func1 = list.GetAt(dwIndex1);
            const CFunctionInfo& func2 = list.GetAt(dwIndex2);
            if(g_SortInfo.nSortIndex == 1)
            {
                if(func1.uLength < func2.uLength)
                    nResult = -1;
                else if(func1.uLength == func2.uLength)
                    nResult = 0;
                else
                    nResult = 1;
            }
            else
            {
                nResult = _tcsicmp(func1.bstrName, func2.bstrName);
            }
        }
        else
        {
            const UDTInfoList& list = g_SortInfo.pDlg->m_PEParser.GetUDTInfo();
            const CUDTInfo& udt1 = list.GetAt(dwIndex1);
            const CUDTInfo& udt2 = list.GetAt(dwIndex2);
            if(g_SortInfo.nSortIndex == 1)
            {
                if(udt1.uLength < udt2.uLength)
                    nResult = -1;
                else if(udt1.uLength == udt2.uLength)
                    nResult = 0;
                else
                    nResult = 1;
            }
            else
            {
                nResult = _tcsicmp(udt1.bstrName, udt2.bstrName);
            }
        }
        if(!g_SortInfo.bAsc)
            nResult = -nResult;
        return nResult;
    }

private:
    CWndLayout      m_WndLayout;
    CPEParser       m_PEParser;

    CProgressBarCtrl m_Progress;
    CStatic         m_LabelProgress;

    CButton         m_RadioFunc;
    CButton         m_RadioUDT;

    CSortListCtrl   m_List;

    ATL::CSimpleArray<DWORD> m_ItemIndex;
};
