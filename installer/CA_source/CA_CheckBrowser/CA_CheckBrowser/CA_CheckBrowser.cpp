// CA_CheckBrowser.cpp : DLL 用にエクスポートされる関数を定義します。
//

#include "pch.h"
#include "framework.h"
#include "CA_CheckBrowser.h"

#pragma comment(lib, "msi.lib")

int CallTaskKill(LPCTSTR lpszApplication)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	TCHAR lszCmdLine[1024];
	
	if (lpszApplication == NULL) return -1;
	_stprintf_s(lszCmdLine, _T("taskkill.exe /F /IM %s"), lpszApplication);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		lszCmdLine,   // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		return -1;
	}
	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, 60 * 1000L);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return 0;
}


int _CheckBrowser(void)
{
	DWORD dwIDProcess[1024];
	DWORD dwSize;
	DWORD dwProcess;
	DWORD i;
	HANDLE hProcess;
	BOOL bIsWow64 = FALSE;

	IsWow64Process(GetCurrentProcess(), &bIsWow64);

	// プロセス識別子の取得
	EnumProcesses(dwIDProcess, sizeof(dwIDProcess), &dwSize);
	dwProcess = dwSize / sizeof(DWORD);
	// プロセス識別子からプロセス名を列挙
	for (i = 0; i < dwProcess; i++) {
		TCHAR   lszFileName[1024] = { 0 };
		HMODULE hModules[1024] = { 0 };

		// プロセスのフルパス名を取得
		if ((hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwIDProcess[i])) != NULL) {
			if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &dwSize)) {
				if (GetModuleBaseName(hProcess, hModules[0], lszFileName, 1024) != 0) {
					if (lstrcmpi(lszFileName, _T("chrome.exe")) == 0) {
						CloseHandle(hProcess);
						return 1;
					}
					if (lstrcmpi(lszFileName, _T("firefox.exe")) == 0) {
						CloseHandle(hProcess);
						return 2;
					}
				}
#if 0
				int j;
				DWORD dwModuleNum = dwSize / sizeof(HMODULE);
				MODULEINFO mi;
				/* 各モジュールの情報を表示 */
				for (j = 0; j < dwModuleNum; j++) {
					/* モジュールのファイル名と情報を取得 */
					GetModuleBaseName(hProcess, hModules[j], lszFileName, 1024);
					GetModuleInformation(hProcess, hModules[j], &mi, sizeof(mi));

					/* 結果を表示 */
					if (lszFileName[0] != '\0') {
						wprintf(_T("%d\t%d\t%s\n"), i, j, lszFileName);
					}
					//					wprintf(_T("\tロードアドレス: %p\n"), mi.lpBaseOfDll);
					//					wprintf(_T("\tサイズ: %08X\n"), mi.SizeOfImage);
					//					wprintf(_T("\tエントリポイント: %p\n"), mi.EntryPoint);
					//					wprintf(_T("\n"));
				}
#endif
				CloseHandle(hProcess);
			}
		}
	}
	return 0;
}

LPCTSTR lpszMessage[] = {
	_T("ブラウザが起動されているとインストールを続行できません。\nブラウザを終了させますか？"),
	_T("ブラウザが起動されているとインストールを続行できません。\nブラウザを終了させますか？"),
	_T("ブラウザが起動されているとインストールを続行できません。\nブラウザを終了させますか？"),
	NULL
};

CACHECKBROWSER_API UINT CheckBrowser(MSIHANDLE hInstall)
{
	PMSIHANDLE hRecord = MsiCreateRecord(1);
	while (1) {
		int iResult = _CheckBrowser();
		if (iResult == 0) {
			break;
		}
		if (iResult > 0 && iResult <= 3) {
			MsiRecordSetString(hRecord, 0, lpszMessage[iResult - 1]);
			int iMessageResult = MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER | MB_YESNOCANCEL), hRecord);
			if (iMessageResult == IDCANCEL) {
				return ERROR_INSTALL_USEREXIT;
			}
			if (iMessageResult == IDYES) {
				if ((iResult & 1) != 0) {
					CallTaskKill(_T("chrome.exe"));
				}
				if ((iResult & 2) != 0) {
					CallTaskKill(_T("firefox.exe"));
				}
			}
			Sleep(1000);
		}
	}

	return ERROR_SUCCESS;
}

#if false
// これは、エクスポートされた変数の例です
CACHECKBROWSER_API int nCACheckBrowser=0;

// これは、エクスポートされた関数の例です。
CACHECKBROWSER_API int fnCACheckBrowser(void)
{
    return 0;
}

// これは、エクスポートされたクラスのコンストラクターです。
CCACheckBrowser::CCACheckBrowser()
{
    return;
}
#endif
