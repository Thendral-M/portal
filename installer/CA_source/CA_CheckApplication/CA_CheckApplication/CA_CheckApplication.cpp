// CA_CheckApplication.cpp : DLL 用にエクスポートされる関数を定義します。
//

#include "pch.h"
#include "framework.h"
#include "CA_CheckApplication.h"

#pragma comment(lib, "msi.lib")

int CheckProcess(LPCTSTR lpszProcess)
{
	DWORD dwIDProcess[1024];
	DWORD dwSize;
	DWORD dwProcess;
	DWORD i;
	HANDLE hProcess;

	if (lpszProcess == NULL) {
		return -1;
	}

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
					if (lstrcmpi(lszFileName, lpszProcess) == 0) {
						CloseHandle(hProcess);
						return 1;
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

int CallTaskKill(LPCTSTR lpszProcess)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR lszCmdLine[1024];

    _stprintf_s(lszCmdLine, _T("taskkill.exe /F /IM %s"), lpszProcess);

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

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    for (int i = 0; i < 300; i++) {
        if (CheckProcess(lpszProcess) != 1) {
            break;
        }
        Sleep(1000);
    }

    return 0;
}

CACHECKAPPLICATION_API UINT TaskKillPortal(MSIHANDLE hInstall)
{
	CallTaskKill(_T("cocoro-office.exe"));

	return ERROR_SUCCESS;
}


#if false
// これは、エクスポートされた変数の例です
CACHECKAPPLICATION_API int nCACheckApplication=0;

// これは、エクスポートされた関数の例です。
CACHECKAPPLICATION_API int fnCACheckApplication(void)
{
    return 0;
}

// これは、エクスポートされたクラスのコンストラクターです。
CCACheckApplication::CCACheckApplication()
{
    return;
}
#endif
