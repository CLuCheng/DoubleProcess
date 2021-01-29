
// demo.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#define APP_ID ""
// CdemoApp:
// See demo.cpp for the implementation of this class
//

class CdemoApp : public CWinApp
{
public:
	CdemoApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CdemoApp theApp;