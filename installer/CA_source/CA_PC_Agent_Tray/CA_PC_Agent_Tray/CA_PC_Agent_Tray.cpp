// CA_PC_Agent_Tray.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "pch.h"
#include "framework.h"
#include "CA_PC_Agent_Tray.h"

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

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

int CallTaskKill(LPCTSTR lpszApplication)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR lszCmdLine[MAX_PATH] = { 0 };

    _tcscpy_s(lszCmdLine, _T("taskkill.exe /F /IM "));
    _tcscat_s(lszCmdLine, lpszApplication);

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

int ExecutePC_Agent_Tray(LPCWSTR lpszDirectory)
{
    STARTUPINFO         startupInfo;
    PROCESS_INFORMATION processInformation;
    WCHAR               wszCommandLine[MAX_PATH] = { 0 };

    wcscpy_s(wszCommandLine, L"\"");
    wcscat_s(wszCommandLine, lpszDirectory);
    wcscat_s(wszCommandLine, L"PC_Agent_Tray.exe\"");

    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));

    if (!CreateProcessW(NULL, wszCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation)) {
        return -1;
    }

    CloseHandle(processInformation.hThread);
    CloseHandle(processInformation.hProcess);

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
    if (lpCmdLine == NULL) return -1;

    LPWSTR* szArglist;
    int nArgs;

    szArglist = CommandLineToArgvW(lpCmdLine, &nArgs);
    if (szArglist != NULL) {
        if (nArgs >= 2) {
            WCHAR wszDirectory[MAX_PATH] = { 0 };
            int l;
            if (szArglist[1][0] == '\"') {
                wcscpy_s(wszDirectory, &(szArglist[1][1]));
            }
            else {
                wcscpy_s(wszDirectory, szArglist[1]);
            }
            l = 0;
            while (wszDirectory[l] != '\0') l++;
            if (l > 0) {
                if (wszDirectory[l - 1] == '\"') {
                    wszDirectory[l - 1] = '\\';
                }
                else {
                    if (l < MAX_PATH - 1) {
                        wszDirectory[l] = '\\';
                        wszDirectory[l + 1] = '\0';
                    }
                }
            }
            if (_wcsicmp(szArglist[0], L"-u") == 0) {
                // uninstall
                for (int i = 0; i < 300; i++) {
                    if (CheckProcess(L"PC_Agent_Tray.exe") != 1) {
                        break;
                    }
                    if ((i % 10) == 0) {
                        CallTaskKill(L"PC_Agent_Tray.exe");
                    }
                    Sleep(1000);
                }
                //ExecuteServiceControllerApp(wszDirectory, L"-uninstallpcagent");
            }
            if (_wcsicmp(szArglist[0], L"-i") == 0) {
                // install
                ExecutePC_Agent_Tray(wszDirectory);
            }
            if (_wcsicmp(szArglist[0], L"-upb") == 0) {
                // update before
                for (int i = 0; i < 300; i++) {
                    if (CheckProcess(L"PC_Agent_Tray.exe") != 1) {
                        break;
                    }
                    if ((i % 10) == 0) {
                        CallTaskKill(L"PC_Agent_Tray.exe");
                    }
                    Sleep(1000);
                }
            }
            if (_wcsicmp(szArglist[0], L"-upa") == 0) {
                // update after
                ExecutePC_Agent_Tray(wszDirectory);
            }
        }

        LocalFree(szArglist);
    }
    return 0;
#if false
    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CAPCAGENTTRAY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CAPCAGENTTRAY));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
#endif
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
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CAPCAGENTTRAY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CAPCAGENTTRAY);
    wcex.lpszClassName  = szWindowClass;
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
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

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
