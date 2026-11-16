// COMigrationTool.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "pch.h"
#include "framework.h"
#include "COMigrationTool.h"

#include "resource.h"

#define MAX_LOADSTRING 100

#define IDT_TIMER1	1

#pragma comment(lib, "Shlwapi.lib")

// グローバル変数:
HINSTANCE g_hInstance;                          // 現在のインターフェイス
WCHAR g_wszTitle[MAX_LOADSTRING];               // タイトル バーのテキスト
WCHAR g_wszWindowClass[MAX_LOADSTRING];         // メイン ウィンドウ クラス名

LPCTSTR g_lpszRegistry = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}");
TCHAR g_lszRegData[1024] = { 0 };
DWORD g_dwRegType = 0;
DWORD g_cbRegData = 0;
TCHAR g_lszISSFileName[MAX_PATH] = { 0 };
const char* g_pszISSData[] = {
    "[InstallShield Silent]",
    "Version=v7.00",
    "File=Response File",
    "[File Transfer]",
    "OverwrittenReadOnly=NoToAll",
    "[{DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}-DlgOrder]",
    "Dlg0={DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}-MessageBox-0",
    "Count=3",
    "Dlg1={DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}-MessageBox-1",
    "Dlg2={DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}-SdFinishReboot-0",
    "[{DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}-MessageBox-0]",
    "Result=6",
    "[{DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}-MessageBox-1]",
    "Result=6",
    "[Application]",
    "[{DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}-SdFinishReboot-0]",
    "Result=1",
    "BootOption=0",
    NULL
};
PROCESS_INFORMATION	g_processInformation;
HANDLE g_hJob = NULL;
PJOBOBJECT_BASIC_PROCESS_ID_LIST g_pProcessIdList = NULL;
DWORD g_dwExitCode = 0;
int g_iCount = 0;

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK	DialogProc(HWND, UINT, WPARAM, LPARAM);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT	OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
LRESULT OnDestroy(HWND hDlg, WPARAM wParam, LPARAM lParam);

BOOL MakeISSFile(void)
{
    DWORD dwSize = MAX_PATH - 20;
    TCHAR lszTempPath[MAX_PATH] = { 0 };
    if (GetTempPath(dwSize, lszTempPath) == 0) {
        return FALSE;
    }
    TCHAR lszTempFileName[MAX_PATH] = { 0 };
    if (GetTempFileName(lszTempPath, _T("iss"), 0, lszTempFileName) == 0) {
        return FALSE;
    }
    _tcscpy_s(g_lszISSFileName, lszTempFileName);

    FILE* fp = NULL;
    if (_tfopen_s(&fp, g_lszISSFileName, _T("w")) != 0) {
        return FALSE;
    }
    if (fp != NULL) {
        for (int i = 0; g_pszISSData[i] != NULL; i++) {
            if (fputs(g_pszISSData[i], fp) < 0) break;
            if (fputs("\n", fp) < 0) break;
        }
        fclose(fp);
    }

    return TRUE;
}

BOOL CheckRegistry(LPCTSTR lpszSubKey)
{
    HKEY hKey;
    DWORD dwRet;
    DWORD dwLastError;

    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszSubKey, 0, KEY_READ, &hKey);
    dwLastError = GetLastError();
    if (dwRet != ERROR_SUCCESS) {
#if false
        TCHAR lszBuffer[1024];
        _stprintf_s(lszBuffer, _T("Err:RegOpenKeyEx():%d"), dwLastError);
        MessageBox(NULL, lszBuffer, NULL, MB_OK | MB_TOPMOST);
#endif
        return FALSE;
    }

    g_dwRegType = 0;
    g_cbRegData = sizeof(g_lszRegData);
    dwRet = RegQueryValueEx(hKey, _T("UninstallString"), NULL, &g_dwRegType, (LPBYTE)g_lszRegData, &g_cbRegData);
    dwLastError = GetLastError();
    RegCloseKey(hKey);

    if (dwRet != ERROR_SUCCESS) {
#if falseE
        TCHAR lszBuffer[1024];
        _stprintf_s(lszBuffer, _T("Err:RegQueryValueEx():%d"), dwLastError);
        MessageBox(NULL, lszBuffer, NULL, MB_OK | MB_TOPMOST);
#endif
        return FALSE;
    }

    if (g_lszRegData[0] == '\0') {
        return FALSE;
    }

    return TRUE;
}

BOOL CheckExecute(void)
{
    DWORD dwExitCode = 0;
    BOOL bJobAllEnd;
    HANDLE hProcess;

    if (g_pProcessIdList == NULL || g_hJob == NULL) {
        return TRUE;
    }

    QueryInformationJobObject(g_hJob, JobObjectBasicProcessIdList, g_pProcessIdList, 10000, NULL);

    bJobAllEnd = TRUE;
    for (DWORD dw = 0; dw < g_pProcessIdList->NumberOfProcessIdsInList; dw++)
    {
        hProcess = OpenProcess(SYNCHRONIZE, FALSE, g_pProcessIdList->ProcessIdList[dw]);
        if (hProcess != NULL)
        {
            CloseHandle(hProcess);
            bJobAllEnd = FALSE;
        }
    }

    return bJobAllEnd;
}

BOOL ExecuteUninstall(void)
{
    STARTUPINFO	startupInfo;
    DWORD dwLastError;
    TCHAR lszCmdLine[1024] = { 0 };

    if (g_lszISSFileName[0] != '\0') {
        _stprintf_s(lszCmdLine, _T("%s -s -f1\"%s\""), g_lszRegData, g_lszISSFileName);
    }
    else {
        _tcscpy_s(lszCmdLine, g_lszRegData);
    }

    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    ZeroMemory(&g_processInformation, sizeof(PROCESS_INFORMATION));

    BOOL bResult = CreateProcess(NULL, lszCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &g_processInformation);
    dwLastError = GetLastError();
    if (!bResult) {
#if false
        TCHAR lszBuffer[1024];
        _stprintf_s(lszBuffer, _T("Err:CreateProcess():%d"), dwLastError);
        MessageBox(NULL, lszBuffer, NULL, MB_OK | MB_TOPMOST);
#endif
        return FALSE;
    }
    g_pProcessIdList = (PJOBOBJECT_BASIC_PROCESS_ID_LIST)GlobalAlloc(GMEM_FIXED, 10000);
    if (g_pProcessIdList == NULL) {
        return FALSE;
    }
    g_hJob = CreateJobObject(NULL, _T("COMigrationTool Job Object"));
    if (g_hJob == NULL) {
        GlobalFree(g_pProcessIdList);
        g_pProcessIdList = NULL;
        return FALSE;
    }

    AssignProcessToJobObject(g_hJob, g_processInformation.hProcess);

    return TRUE;
}

void _FreeAll(void)
{
    if (g_hJob != NULL) {
        CloseHandle(g_hJob);
    }
    if (g_pProcessIdList != NULL) {
        GlobalFree(g_pProcessIdList);
    }
    if (g_processInformation.hProcess != NULL) {
        GetExitCodeProcess(g_processInformation.hProcess, &g_dwExitCode);
    }
    if (g_processInformation.hThread != NULL) {
        CloseHandle(g_processInformation.hThread);
    }
    if (g_processInformation.hProcess != NULL) {
        CloseHandle(g_processInformation.hProcess);
    }
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, g_wszTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_COMIGRATIONTOOL, g_wszWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!(CheckRegistry(g_lpszRegistry))) {
        return ERROR_SUCCESS;
    }

    MakeISSFile();
    if (!ExecuteUninstall())
    {
        return ERROR_INSTALL_FAILURE;
    }

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

//  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COMIGRATIONTOOL));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
//      if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		if (!IsDialogMessage(msg.hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    DeleteFile(g_lszISSFileName);
    _FreeAll();
    if (!(CheckRegistry(g_lpszRegistry))) {
        return ERROR_SUCCESS;
    }

//  return (int) msg.wParam;
    return (int)g_dwExitCode;
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
//  wcex.lpfnWndProc    = WndProc;
    wcex.lpfnWndProc    = DialogProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COMIGRATIONTOOL));
//	wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hCursor        = LoadCursor(nullptr, IDC_WAIT);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_COMIGRATIONTOOL);
    wcex.lpszClassName  = g_wszWindowClass;
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

    hDlg = CreateDialog(g_hInstance, (LPTSTR)IDD_CO_MIGRATION_TOOL, NULL, (DLGPROC)DialogProc);
    if (hDlg == NULL) return FALSE;

    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);

#if false
   HWND hWnd = CreateWindowW(g_wszWindowClass, g_wszTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
#endif

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
        {
            // process the 1-second timer 
            if (CheckExecute()) {
                KillTimer(hDlg, IDT_TIMER1);
                DestroyWindow(hDlg);
                return 0;
            }
            if (g_iCount == 0) {
                BOOL bVisible = IsWindowVisible(hDlg);
                SetWindowPos(hDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | (bVisible ? SWP_NOACTIVATE : 0));
                // Should we bring the window to the foreground
                SetForegroundWindow(hDlg);
            }
            g_iCount++;
            if (g_iCount >= 180) {
                KillTimer(hDlg, IDT_TIMER1);
                DestroyWindow(hDlg);
                return 0;
            }

            SetCursor(LoadCursor(nullptr, IDC_WAIT));
            return 0;
        }
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

    SetDlgItemText(hDlg, IDC_STATIC_MESSAGE, _T("以前のバージョンをアンインストールしています"));

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

    MessageBox(NULL, _T("check"), NULL, MB_OK | MB_TOPMOST);
    PostQuitMessage(0);
    return 0;
}

#if false
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
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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
    case WM_DESTROY:
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
