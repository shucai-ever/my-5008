
// PI5008K_UpgradeDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "PI5008K_Upgrade.h"
#include "PI5008K_UpgradeDlg.h"
#include "afxdialogex.h"
#include <sys/stat.h>
#include "spicomm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPI5008K_UpgradeDlg ��ȭ ����



CPI5008K_UpgradeDlg::CPI5008K_UpgradeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPI5008K_UpgradeDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPI5008K_UpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SECTION_COMBO, m_SectionList);
	DDX_Control(pDX, IDC_VERIFY_CHECK, m_VerifyCheck);
	DDX_Control(pDX, IDC_EDIT_FILE, m_edFile);
}

BEGIN_MESSAGE_MAP(CPI5008K_UpgradeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_UPGRADE, &CPI5008K_UpgradeDlg::OnBnClickedButtonUpgrade)
	ON_BN_CLICKED(IDC_BUTTON_FILE_OPEN, &CPI5008K_UpgradeDlg::OnBnClickedButtonFileOpen)
	ON_BN_CLICKED(IDC_BUTTON_UPGRADE_READY, &CPI5008K_UpgradeDlg::OnBnClickedButtonUpgradeReady)
END_MESSAGE_MAP()


// CPI5008K_UpgradeDlg �޽��� ó����

BOOL CPI5008K_UpgradeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	InitCtrl();

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CPI5008K_UpgradeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CPI5008K_UpgradeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CPI5008K_UpgradeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//////////////////////////////////////////////////////////////////////////////////
//			SPI COMM. F/W Upgrade EXAMPLE
//////////////////////////////////////////////////////////////////////////////////

#define FLASH_RW_SIZE			(32*1024)
#define FLASH_ERASE_SECTOR		(4*1024)

DWORD GetVersionFromFile(char *pszfilename)
{
	char *ptr1 = strstr(pszfilename, "Ver" );
	char *ptr2;
	int num[4];
	int i;
	DWORD version;

	if( ptr1 == NULL )
	{
		version = 1;
		return version;
	}

	ptr2 = strtok(ptr1, "-");

	if(!(sscanf(ptr2, "Ver%d.%d.%d.%d", &num[0], &num[1], &num[2], &num[3])))
	{
		version = 1;
		return version;
	}

	for( i = 0; i < 4; i++ )
	{
		if( num[i] > 255 )
		{
			version = 1;
			return version;
		}
		else
		{
			if( i == 3 )
			{
				if( num[i] > 254 )
				{
					version = 1;
					return version;
				}
			}
		}
	}

	version =  num[0]<<24;
	version += num[1]<<16;
    version += num[2]<<8;
    version += num[3]<<0;

	if( version == 0 )
		version = 1;

	return version;
}

int ReadDataFromFile(const char *pszfilename, BYTE *pBuf, DWORD size)
{
	int ret = 0;
	FILE *fp;

	if((fp = fopen(pszfilename, "rb")) == NULL){
		TRACE("Open file fail(%s)\n", pszfilename);
		return -1;
	}

	fread(pBuf, size, 1, fp);

	fclose(fp);

	return ret;
}

int FW_Upgrade(const char *szFileName, BYTE Verify, BYTE Upgrade_Section, BYTE CurrCnt, BYTE TotalCnt, DWORD Version)
{
	int ret = 0;
	DWORD write_size;
	DWORD written_size;
	DWORD write_unit_size;
	BYTE *pBuf = NULL;
	struct _stat stInfo;

	if(_stat((const char *)szFileName, &stInfo) < 0){
		ret = -1;
		goto END_FT;
	}
	write_size = (stInfo.st_size + (FLASH_ERASE_SECTOR-1)) & (~(FLASH_ERASE_SECTOR-1));

	if((pBuf = (BYTE *)malloc(write_size)) == NULL){
		ret = -1;
		goto END_FT;
	}


	// load f/w file	
	ReadDataFromFile(szFileName, pBuf, stInfo.st_size);

	TRACE("wait until slave device is ready\n");
	
	ret = SPICOMM_CheckStatus(TIMEOUT_MSEC);

	if( ret != RESP_OK )
	{
		ret = -1;
		goto END_FT;
	}

	TRACE("Upgrade Start!!\n");
	
	SPICOMM_UpgradeStart(write_size, Upgrade_Section, Verify, CurrCnt, TotalCnt, Version);
		
	ret = SPICOMM_CheckStatus(TIMEOUT_MSEC);

	if( ret != RESP_OK )
	{
		ret = -1;
		goto END_FT;
	}

	TRACE("Upgrade Start Done!!\n");

#if 0
	SPICOMM_FlashSendReady();

	ret = SPICOMM_CheckStatus(TIMEOUT_MSEC);

	if( ret != RESP_OK )
	{
		ret = -1;
		goto END_FT;
	}
#endif
	written_size = 0;

	TRACE("Upgrade Data Send Start!!\n");
	do{
		if(write_size - written_size > FLASH_RW_SIZE)write_unit_size = FLASH_RW_SIZE;
		else write_unit_size = write_size - written_size;
		
		if(SPICOMM_UpgradeDataSend((BYTE *)(pBuf + written_size), write_unit_size) < 0){
			TRACE("Flash write fail\n");
			ret = -1;
			goto END_FT;
		}

		//TRACE("[[%d]] time_data = %d\n", count, time_data);

		ret = SPICOMM_CheckStatus(TIMEOUT_MSEC);
		
		if(ret != RESP_OK){
			if( ret == ERROR_TIMEOUT_RESP )
			{
				goto END_FT;
			}
			else
			{
				Sleep(1);
				TRACE("resp fail retry. (ret: %d)\n", ret);
			}
		}else{
			written_size += write_unit_size;
			
			//TRACE("Flash written %d / %d\n", written_size, write_size);
		}
	}while(written_size < write_size);

	TRACE("Upgrade Data Send Done!!\n");

	
	if(SPICOMM_UpgradeDone() < 0){
		TRACE("F/W Upgrade fail\n");
		ret = -1;
		goto END_FT;
	}

	ret = SPICOMM_CheckStatus(TIMEOUT_MSEC);

	if( ret != RESP_OK )
	{
		ret = -1;
		goto END_FT;
	}
		

	TRACE("F/W Upgrade Done!!!\n");

END_FT:
	if(pBuf)free(pBuf);

	return ret;
}

void CPI5008K_UpgradeDlg::InitCtrl(void)
{
	m_SectionList.AddString(_T("ISP Firmware"));
	m_SectionList.AddString(_T("Main Firmware"));
	m_SectionList.AddString(_T("SVM LUT"));
	m_SectionList.AddString(_T("Menu"));
	m_SectionList.AddString(_T("GUI Image"));
	m_SectionList.AddString(_T("ISP Data"));
	m_SectionList.AddString(_T("Sensor Data"));
	m_SectionList.AddString(_T("Calibration Data"));
	m_SectionList.AddString(_T("All Flash Image"));

	m_SectionList.SetCurSel(1);
	m_VerifyCheck.SetCheck(1);
}

void CPI5008K_UpgradeDlg::OnBnClickedButtonUpgrade()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	int Section_Index;
	int Verify_Check;
	char szBuf[256];
	char temp[256];
	DWORD Version;

	Section_Index = m_SectionList.GetCurSel();
	Verify_Check = m_VerifyCheck.GetCheck();

	if( Section_Index == 8 ) // All Flash Image
		Section_Index = 0xff;

	SPICOMM_Initialize( CLOCK_RATE_10M );

	// load ootloader
	m_edFile.GetWindowTextA(szBuf, 256);

	strncpy( temp, szBuf, 256 );

	Version = GetVersionFromFile(temp);

	TRACE("Version = %02x\n", Version);

	if( FW_Upgrade(szBuf, Verify_Check, Section_Index,1, 1, Version) != 0 )
		TRACE("Upgrade Done\n");
	else
		TRACE("Upgrade Fail\n");

	SPICOMM_deInitialize();
}


void CPI5008K_UpgradeDlg::OnBnClickedButtonFileOpen()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	BOOL IsOpenDlg = TRUE;
	CFileDialog fileDlg( IsOpenDlg, _T("extension"), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this ); 

	fileDlg.m_ofn.lpstrTitle = (IsOpenDlg == TRUE ? _T("Open ...") : _T("Save as ...")); // File Dialog Title
	fileDlg.m_ofn.lpstrInitialDir = NULL;
	fileDlg.m_ofn.lpstrFilter = _T("bin files( *.* )\0*.*");

	if(fileDlg.DoModal() == IDCANCEL)
		return;

	m_edFile.SetWindowTextA(fileDlg.GetPathName());
}


void CPI5008K_UpgradeDlg::OnBnClickedButtonUpgradeReady()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	SPICOMM_Initialize(CLOCK_RATE_46K);

	SPICOMM_UpgradeReady();

	SPICOMM_deInitialize();
}
