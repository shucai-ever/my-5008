
// PI5008K_Upgrade.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CPI5008K_UpgradeApp:
// �� Ŭ������ ������ ���ؼ��� PI5008K_Upgrade.cpp�� �����Ͻʽÿ�.
//

class CPI5008K_UpgradeApp : public CWinApp
{
public:
	CPI5008K_UpgradeApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CPI5008K_UpgradeApp theApp;