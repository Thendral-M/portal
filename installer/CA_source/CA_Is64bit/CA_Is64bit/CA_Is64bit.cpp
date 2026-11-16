// CA_Is64bit.cpp : DLL 用にエクスポートされる関数を定義します。
//

#include "pch.h"
#include "framework.h"
#include "CA_Is64bit.h"

#pragma comment(lib, "msi.lib")

CAIS64BIT_API UINT Is64bit(MSIHANDLE hInstall)
{
	UINT uiResult;
	BOOL bIsWow64;
	TCHAR lszValue[64];

	IsWow64Process(GetCurrentProcess(), &bIsWow64);

	if (bIsWow64) {
		_tcscpy_s(lszValue, _T("64"));
	}
	else {
		_tcscpy_s(lszValue, _T("32"));
	}

	uiResult = MsiSetProperty(hInstall, _T("OSBIT"), lszValue);

	return ERROR_SUCCESS;
}

#if false
// これは、エクスポートされた変数の例です
CAIS64BIT_API int nCAIs64bit=0;

// これは、エクスポートされた関数の例です。
CAIS64BIT_API int fnCAIs64bit(void)
{
    return 0;
}

// これは、エクスポートされたクラスのコンストラクターです。
CCAIs64bit::CCAIs64bit()
{
    return;
}
#endif
