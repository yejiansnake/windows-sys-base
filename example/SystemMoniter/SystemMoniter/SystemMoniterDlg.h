
// SystemMoniterDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"


// CSystemMoniterDlg 对话框
class CSystemMoniterDlg : public CDialogEx
{
// 构造
public:
	CSystemMoniterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SYSTEMMONITER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

    void OnOK(){};

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:

    BYTE m_SystemCpuUsage;
    BYTE m_SystemCpuIdle;

    ULONGLONG m_SystemCommitTotal;
    ULONGLONG m_SystemCommitLimit;
    ULONGLONG m_SystemCommitPeak;

    ULONGLONG m_PhcTotal;
    ULONGLONG m_PhcAVB;
    ULONGLONG m_SystemCache;

    ULONGLONG m_KernelTotal;
    ULONGLONG m_KernelPaged;
    ULONGLONG m_KernelNonpaged;
    ULONGLONG m_PageSize;

    DWORD m_HandleCount;
    DWORD m_ProcCount;
    DWORD m_ThreadCount;

public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose();
private:
    CListCtrl m_ProcListCtl;
};
