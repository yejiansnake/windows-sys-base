
// SystemMoniterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SystemMoniter.h"
#include "SystemMoniterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLG_TIMER_ID 1001

// CSystemMoniterDlg 对话框



CSystemMoniterDlg::CSystemMoniterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSystemMoniterDlg::IDD, pParent)
    , m_SystemCpuUsage(0)
    , m_SystemCpuIdle(0)
    , m_SystemCommitTotal(0)
    , m_SystemCommitLimit(0)
    , m_SystemCommitPeak(0)
    , m_PhcTotal(0)
    , m_PhcAVB(0)
    , m_SystemCache(0)
    , m_HandleCount(0)
    , m_ProcCount(0)
    , m_ThreadCount(0)
    , m_KernelTotal(0)
    , m_KernelPaged(0)
    , m_KernelNonpaged(0)
    , m_PageSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSystemMoniterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_CPU_USAGE, m_SystemCpuUsage);
    DDX_Text(pDX, IDC_EDIT_CPU_IDLE, m_SystemCpuIdle);
    DDX_Text(pDX, IDC_EDIT_COMMIT_TOTAL, m_SystemCommitTotal);
    DDX_Text(pDX, IDC_EDIT_COMMIT_LIMIT, m_SystemCommitLimit);
    DDX_Text(pDX, IDC_EDIT_COMMIT_PEAK, m_SystemCommitPeak);
    DDX_Text(pDX, IDC_EDI_PHC_TOTAL, m_PhcTotal);
    DDX_Text(pDX, IDC_EDIT_PHC_AVB, m_PhcAVB);
    DDX_Text(pDX, IDC_EDIT_SYS_CACHE, m_SystemCache);
    DDX_Text(pDX, IDC_EDIT_HANDLE_COUNT, m_HandleCount);
    DDX_Text(pDX, IDC_EDIT_PROC_COUNT, m_ProcCount);
    DDX_Text(pDX, IDC_EDIT_THREAD_COUNT, m_ThreadCount);
    DDX_Text(pDX, IDC_EDIT_KERNEL_TOTAL, m_KernelTotal);
    DDX_Text(pDX, IDC_EDIT_LERNEL_PAGED, m_KernelPaged);
    DDX_Text(pDX, IDC_EDIT_KERNEL_NONPAGED, m_KernelNonpaged);
    DDX_Text(pDX, IDC_EDITPAGE_SIZE, m_PageSize);
    DDX_Control(pDX, IDC_LIST_PROC, m_ProcListCtl);
}

BEGIN_MESSAGE_MAP(CSystemMoniterDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CSystemMoniterDlg 消息处理程序

BOOL CSystemMoniterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    SetTimer(DLG_TIMER_ID, 1000, NULL);

    DWORD dwStyle = m_ProcListCtl.GetExtendedStyle();
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_DOUBLEBUFFER;
    m_ProcListCtl.SetExtendedStyle(dwStyle); //设置扩展风格

    m_ProcListCtl.InsertColumn(0, L"序号", 0, 40);
    m_ProcListCtl.InsertColumn(1, L"进程名称", 0, 130);
    m_ProcListCtl.InsertColumn(2, L"PID", 0, 40);
    m_ProcListCtl.InsertColumn(3, L"CPU", 0, 40);
    m_ProcListCtl.InsertColumn(4, L"用户名", 0, 90);
    m_ProcListCtl.InsertColumn(5, L"当前内存", 0, 90);
    m_ProcListCtl.InsertColumn(6, L"峰值内存", 0, 90);
    m_ProcListCtl.InsertColumn(7, L"句柄数", 0, 50);
    m_ProcListCtl.InsertColumn(8, L"GDI数", 0, 50);
    m_ProcListCtl.InsertColumn(9, L"用户对象数", 0, 60);
    m_ProcListCtl.InsertColumn(10, L"64位", 0, 50);
    m_ProcListCtl.InsertColumn(11, L"启动时间", 0, 150);
    m_ProcListCtl.InsertColumn(12, L"命令行", 0, 200);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSystemMoniterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSystemMoniterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSystemMoniterDlg::OnTimer(UINT_PTR nIDEvent)
{
    //////////////////////////////////////////////////////////////////////////

    if (DLG_TIMER_ID == nIDEvent)
    {
        ProcessPerformanceMap aProcessPerformanceMap;
        TSystemPerformance aTSystemPerformance = {0};

        CSystemPerformanceCollecter::Instance().Get(&aProcessPerformanceMap, &aTSystemPerformance);

        //////////////////////////////////////////////////////////////////////////

        m_SystemCpuUsage = aTSystemPerformance.aCupUsage.nSystemCpuUsage;
        m_SystemCpuIdle = aTSystemPerformance.aCupUsage.nSystemIdleCpuUsage;

        m_SystemCommitTotal = aTSystemPerformance.aResource.nCommitTotal;
        m_SystemCommitLimit = aTSystemPerformance.aResource.nCommitLimit;
        m_SystemCommitPeak = aTSystemPerformance.aResource.nCommitPeak;

        m_PhcTotal = aTSystemPerformance.aResource.nPhysicalTotal;
        m_PhcAVB = aTSystemPerformance.aResource.nPhysicalAvailable;
        m_SystemCache = aTSystemPerformance.aResource.nSystemCache;

        m_KernelTotal = aTSystemPerformance.aResource.nKernelTotal;
        m_KernelPaged = aTSystemPerformance.aResource.nKernelPaged;
        m_KernelNonpaged = aTSystemPerformance.aResource.nKernelNonpaged;
        m_PageSize = aTSystemPerformance.aResource.nPageSize;

        m_HandleCount = aTSystemPerformance.aResource.nHandleCount;
        m_ProcCount = aTSystemPerformance.aResource.nProcessCount;
        m_ThreadCount = aTSystemPerformance.aResource.nThreadCount;

        UpdateData(FALSE);

        //////////////////////////////////////////////////////////////////////////

        m_ProcListCtl.SetRedraw(FALSE);

        m_ProcListCtl.DeleteAllItems();

        ProcessPerformanceMap::iterator entity = aProcessPerformanceMap.begin();
        ProcessPerformanceMap::iterator endEntity = aProcessPerformanceMap.end();

        for (; entity != endEntity; ++entity)
        {
            WCHAR szTemp[1024] = {0};

            int nIndex = m_ProcListCtl.GetItemCount();

            LVITEMW aLvItem = {0};
            aLvItem.mask = LVIF_TEXT;
            wsprintfW(szTemp, L"%d", nIndex + 1);
            aLvItem.pszText = szTemp;
            aLvItem.iItem = m_ProcListCtl.GetItemCount();
            m_ProcListCtl.InsertItem(&aLvItem);

            aLvItem.pszText = entity->second.aNormal.szExeFile;
            aLvItem.iSubItem = 1;
            m_ProcListCtl.SetItem(&aLvItem);

            wsprintfW(szTemp, L"%lu", entity->second.aNormal.nProcessID);
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 2;
            m_ProcListCtl.SetItem(&aLvItem);

            wsprintfW(szTemp, L"%lu", (UINT32)entity->second.aCpuUsage.nCpuUsage);
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 3;
            m_ProcListCtl.SetItem(&aLvItem);
			
            aLvItem.pszText = entity->second.aAdvanced.szUserName;
            aLvItem.iSubItem = 4;
            m_ProcListCtl.SetItem(&aLvItem);

            wsprintfW(szTemp, L"%I64u", entity->second.aMemory.nWorkingSetSize);
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 5;
            m_ProcListCtl.SetItem(&aLvItem);

            wsprintfW(szTemp, L"%I64u", entity->second.aMemory.nPeakWorkingSetSize);
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 6;
            m_ProcListCtl.SetItem(&aLvItem);

            wsprintfW(szTemp, L"%ld", entity->second.aResource.nHandleCount);
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 7;
            m_ProcListCtl.SetItem(&aLvItem);

            wsprintfW(szTemp, L"%ld", entity->second.aResource.nGDICount);
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 8;
            m_ProcListCtl.SetItem(&aLvItem);

            wsprintfW(szTemp, L"%ld", entity->second.aResource.nUserObjectCount);
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 9;
            m_ProcListCtl.SetItem(&aLvItem);

            wsprintfW(szTemp, L"%s", entity->second.aAdvanced.bIs64 == 0 ? L"否" : L"是");
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 10;
            m_ProcListCtl.SetItem(&aLvItem);

            INT64 nCreateTime = entity->second.aTime.nCreateTime;
            struct tm createTimeTM = {0};
            _localtime64_s(&createTimeTM, &nCreateTime);

            wsprintfW(szTemp, L"%d-%d-%d %d:%d:%d", createTimeTM.tm_year + 1900, createTimeTM.tm_mon + 1, createTimeTM.tm_mday, createTimeTM.tm_hour, createTimeTM.tm_min, createTimeTM.tm_sec);
            aLvItem.pszText = szTemp;
            aLvItem.iSubItem = 11;
            m_ProcListCtl.SetItem(&aLvItem);

            aLvItem.pszText = entity->second.aAdvanced.szImageFileName;
            aLvItem.iSubItem = 12;
            m_ProcListCtl.SetItem(&aLvItem);
        }

        m_ProcListCtl.SetRedraw(TRUE);
        m_ProcListCtl.Invalidate();
        m_ProcListCtl.UpdateWindow();
    }

    //////////////////////////////////////////////////////////////////////////

    CDialogEx::OnTimer(nIDEvent);

    SetTimer(DLG_TIMER_ID, 1000, NULL);
}


void CSystemMoniterDlg::OnClose()
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    KillTimer(DLG_TIMER_ID);

    CDialogEx::OnClose();
}
