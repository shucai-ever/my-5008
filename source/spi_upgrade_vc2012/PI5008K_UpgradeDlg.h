
// PI5008K_UpgradeDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"


// CPI5008K_UpgradeDlg ��ȭ ����
class CPI5008K_UpgradeDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CPI5008K_UpgradeDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_PI5008K_UPGRADE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
public:
	void InitCtrl(void);
	
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edFile;
	CComboBox m_SectionList;
	CButton m_VerifyCheck;
	afx_msg void OnBnClickedButtonUpgrade();
	afx_msg void OnBnClickedButtonFileOpen();
	afx_msg void OnBnClickedButtonUpgradeReady();
};
