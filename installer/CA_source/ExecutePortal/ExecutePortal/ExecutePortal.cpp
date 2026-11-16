// ExecutePortal.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "ExecutePortal.h"

#include <psapi.h>
#include <shellapi.h>

#pragma comment(lib, "Version.lib")

#define MAX_LOADSTRING 100

#define IDT_TIMER1	1

// グローバル変数:
HINSTANCE g_hInstance;                                // 現在のインターフェイス
WCHAR g_lszTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR g_lszWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名
int g_iCount = 0;
WCHAR g_lszMessage[1024] = { 0 };
int g_iStatus = 0;
WCHAR g_lszCmdLine[1024] = { 0 };

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK	DialogProc(HWND, UINT, WPARAM, LPARAM);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT	OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
LRESULT OnDestroy(HWND hDlg, WPARAM wParam, LPARAM lParam);

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

int CallTaskKill(void)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	TCHAR lszCmdLine[] = _T("taskkill.exe /F /IM cocoro-office.exe");

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

int ExecutePortal(LPWSTR lpszCmdLine)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if (lpszCmdLine == NULL) {
		return -1;
	}
	if (lpszCmdLine[0] == '\0') {
		return -1;
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcessW(NULL,   // No module name (use command line)
		lpszCmdLine,     // Command line
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。
#if 1
	if (lpCmdLine == NULL) return -1;

	LPWSTR *szArglist;
	int nArgs;
	BOOL bCheckVersion = FALSE;

	szArglist = CommandLineToArgvW(lpCmdLine, &nArgs);
	if (szArglist != NULL) {
		DWORD dwSize;
		BYTE* pVersion;
		VS_FIXEDFILEINFO* pFileInfo;
		UINT uLen;

		dwSize = GetFileVersionInfoSize(szArglist[0], NULL);
		pVersion = new BYTE[dwSize];
		if (GetFileVersionInfo(szArglist[0], NULL, dwSize, pVersion)) {
			VerQueryValue(pVersion, _T("\\"), (void**)&pFileInfo, &uLen);

#if false
			TCHAR szBuf[1024];
			wsprintf(szBuf, _T("%d.%d.%d.%d"),
				HIWORD(pFileInfo->dwFileVersionMS),
				LOWORD(pFileInfo->dwFileVersionMS),
				HIWORD(pFileInfo->dwFileVersionLS),
				LOWORD(pFileInfo->dwFileVersionLS));	// ex 1.11.125.0
			MessageBox(NULL, szBuf, NULL, MB_OK | MB_TOPMOST);
#endif
			bCheckVersion = TRUE;
			if (HIWORD(pFileInfo->dwFileVersionMS) < 2 && LOWORD(pFileInfo->dwFileVersionMS) < 10) {
				bCheckVersion = FALSE;
			}
		}
		delete[] pVersion;

		if (nArgs >= 3) {
			wcscpy_s(g_lszMessage, szArglist[2]);
			wcscat_s(g_lszMessage, _T("\r\nしばらくお待ちください。"));
			wcscpy_s(g_lszCmdLine, szArglist[0]);
			wcscat_s(g_lszCmdLine, _T(" "));
			wcscat_s(g_lszCmdLine, szArglist[1]);
		}

		LocalFree(szArglist);
	}
	if (!bCheckVersion) {
		return 0;
	}

#else
	LPWSTR *szArglist;
	int nArgs;

	szArglist = CommandLineToArgvW(lpCmdLine, &nArgs);
	if (szArglist != NULL) {
		if (nArgs >= 2) {
			ShellExecute(NULL, _T("open"), szArglist[0], szArglist[1], NULL, SW_HIDE);
		}
		LocalFree(szArglist);
	}
#endif

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, g_lszTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EXECUTEPORTAL, g_lszWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
 //   if (!InitInstance (hInstance, nCmdShow))
	if (!InitInstance(hInstance, SW_HIDE))
	{
        return FALSE;
    }

//    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EXECUTEPORTAL));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
//		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		if (!IsDialogMessage(msg.hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DialogProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EXECUTEPORTAL));
//	wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hCursor		= LoadCursor(nullptr, IDC_WAIT);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EXECUTEPORTAL);
    wcex.lpszClassName  = g_lszWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hDlg;
	g_hInstance = hInstance;

	hDlg = CreateDialog(g_hInstance, (LPTSTR)IDD_EXECUTEPORTAL, NULL, (DLGPROC)DialogProc);
	if (hDlg == NULL) return FALSE;
	ShowWindow(hDlg, SW_SHOW);
	UpdateWindow(hDlg);

   return TRUE;
}


//
//  関数: DialogProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	WORD wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch (wmId)
		{
		case IDCANCEL:
			break;
		case IDOK:
			break;
		default:
			break;
		}
		break;
	case WM_INITDIALOG:
		lResult = OnInitDialog(hDlg, wParam, lParam);
		break;
	case WM_TIMER:
		{
			switch (wParam)
			{
			case IDT_TIMER1:
				// process the 10-second timer 
				if (CheckProcess(_T("cocoro-office.exe")) != 1) {
					if (g_iStatus == 1) {
						DestroyWindow(hDlg);
					}
					ExecutePortal(g_lszCmdLine);
					g_iStatus = 1;
				}
				g_iCount++;
				if ((g_iCount % 30) == 0) {
					CallTaskKill();
				}
				if (g_iCount >= 180) {
					DestroyWindow(hDlg);
					return 0;
				}
				SetCursor(LoadCursor(nullptr, IDC_WAIT));
				return 0;
			default:
				break;
			}
		}
		break;
	case WM_DESTROY:
		lResult = OnDestroy(hDlg, wParam, lParam);
		break;
	default:
		break;
	}
	return lResult;
}

// ===========================================================================
//    LRESULT OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
// ---------------------------------------------------------------------------
// [機能]     | WM_INITDIALOGのメッセージ処理
// [引数]     | HWND    hDlg    ... [in] ダイアログのウィンドウハンドル
//            | WPARAM  wParam  ... [in] パラメータ
//            | LPARAM  lParam  ... [in] パラメータ
// -----------+---------------------------------------------------------------
// [返値]     |  1      成功
//            |  0      失敗
// ===========================================================================
LRESULT OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	int iCx;
	int iCy;
	int iX;
	int iY;

	// ---- ダイアログを中央に表示
	// -- ダイアログの幅と高さの取得
	GetClientRect(hDlg, &rect);
	// -- 画面の幅と高さの取得
	iCx = GetSystemMetrics(SM_CXSCREEN);
	iCy = GetSystemMetrics(SM_CYSCREEN);
	// -- 表示位置の計算
	iX = iCx - rect.right;
	iY = iCy - rect.bottom;
	if (iX < 0) iX = 0;
	if (iY < 0) iY = 0;
	// -- ダイアログの移動
	SetWindowPos(hDlg, HWND_TOP, iX / 2, iY / 2, 0, 0, SWP_NOSIZE);

	SetDlgItemText(hDlg, IDC_STATIC_MESSAGE, g_lszMessage);

	if (CheckProcess(_T("cocoro-office.exe")) == 1) {
		CallTaskKill();
	}
	else {
		ExecutePortal(g_lszCmdLine);
		g_iStatus = 1;
	}

	SetTimer(hDlg, IDT_TIMER1, 1000, (TIMERPROC)NULL);

	return 1;
}

// ===========================================================================
//    LRESULT OnDestroy(HWND hDlg, WPARAM wParam, LPARAM lParam)
// ---------------------------------------------------------------------------
// [機能]     | WM_DESTROYのメッセージ処理
// [引数]     | HWND    hDlg    ... [in] ダイアログのウィンドウハンドル
//            | WPARAM  wParam  ... [in] パラメータ
//            | LPARAM  lParam  ... [in] パラメータ
// -----------+---------------------------------------------------------------
// [返値]     |  0      成功
// ===========================================================================
LRESULT OnDestroy(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	SetCursor(LoadCursor(nullptr, IDC_ARROW));

	PostQuitMessage(0);
	return 0;
}

#if 0
//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_TIMER:
		{
			switch (wParam)
			{
			case IDT_TIMER1:
				// process the 10-second timer 
				if (CheckProcess(_T("cocoro-office.exe")) != 1) {
					DestroyWindow(hWnd);
				}
				g_iCount++;
				if (g_iCount >= 180) {
					DestroyWindow(hWnd);
					return 0;
				}
				return 0;
			default:
				break;
			}
		}
		break;
    case WM_DESTROY:
		KillTimer(hWnd, IDT_TIMER1);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
#endif
