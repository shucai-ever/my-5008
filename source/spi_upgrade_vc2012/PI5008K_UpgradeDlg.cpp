
// PI5008K_UpgradeDlg.cpp : 구현 파일
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


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CPI5008K_UpgradeDlg 대화 상자



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


// CPI5008K_UpgradeDlg 메시지 처리기

BOOL CPI5008K_UpgradeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	InitCtrl();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CPI5008K_UpgradeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	SPICOMM_Initialize(CLOCK_RATE_46K);

	SPICOMM_UpgradeReady();

	SPICOMM_deInitialize();
}
