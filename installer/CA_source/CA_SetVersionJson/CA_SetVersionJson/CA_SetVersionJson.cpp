// CA_SetVersionJson.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "pch.h"
#include "framework.h"
#include "CA_SetVersionJson.h"

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

LPCTSTR g_lpszRegistry = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}");
DWORD g_dwRegData = 0;
DWORD g_dwRegType = 0;
DWORD g_cbRegData = 0;

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int SetAllowEveryoneAll(LPTSTR lpszPath)
{
    int iRet = 0;
    DWORD dwResult = 0;

    // SECURITY_DESCRIPTOR
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    // DACL
    ACL* pDacl = NULL;
    ACL* pNewDacl = NULL;

    // SID
    PSID pSid = NULL;

    // EXPLICIT_ACCESS
    EXPLICIT_ACCESS oExpAcs = { 0 };


    /*
        DACLの取得
    */
    dwResult = ::GetNamedSecurityInfo(
        lpszPath           // パス
        , SE_FILE_OBJECT
        , DACL_SECURITY_INFORMATION
        , NULL                      // 所有者SID
        , NULL                      // プライマリグループSID
        , &pDacl                    // DACL
        , NULL                      // SACL
        , &pSecurityDescriptor      // SECURITY_DESCRIPTOR
    );
    if (ERROR_SUCCESS != dwResult) {

        // エラー
        iRet = -1;
        goto err;
    }


    /*
        EveryoneのSIDを取得する
    */
    if (0 == ::ConvertStringSidToSid(L"S-1-1-0", &pSid)) {

        // エラー
        iRet = -1;
        goto err;
    }


    /*
        EXPLICIT_ACCESSの設定
    */
    oExpAcs.grfAccessMode = SET_ACCESS;
    oExpAcs.grfAccessPermissions = GENERIC_ALL; // 全て
    oExpAcs.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    oExpAcs.Trustee.pMultipleTrustee = NULL;
    oExpAcs.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    oExpAcs.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    oExpAcs.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    oExpAcs.Trustee.ptstrName = (LPTSTR)pSid;


    /*
        ACLの結合
    */
    dwResult = ::SetEntriesInAcl(1, &oExpAcs, pDacl, &pNewDacl);
    if (ERROR_SUCCESS != dwResult) {

        // エラー
        iRet = -1;
        goto err;
    }


    /*
        DACLの設定
    */
    dwResult = ::SetNamedSecurityInfo(
        lpszPath
        , SE_FILE_OBJECT            // パス
        , DACL_SECURITY_INFORMATION
        , NULL                      // 所有者SID
        , NULL                      // プライマリグループSID
        , pNewDacl                  // DACL(new)
        , NULL                      // SACL
    );
    if (ERROR_SUCCESS != dwResult) {

        // エラー
        iRet = -1;
        goto err;
    }


err:
    // SIDの解放
    if (NULL != pSid) {
        LocalFree(pSid);
    }

    // DACL(new)の解放
    if (NULL != pNewDacl) {
        LocalFree(pNewDacl);
    }

    // SECURITY_DESCRIPTORの解放
    if (NULL != pSecurityDescriptor) {
        LocalFree(pSecurityDescriptor);
    }

    // 処理結果を返す
    return iRet;
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
    g_cbRegData = sizeof(g_dwRegData);
    dwRet = RegQueryValueEx(hKey, _T("Version"), NULL, &g_dwRegType, (LPBYTE)(&g_dwRegData), &g_cbRegData);
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

    return TRUE;
}

int CreateVersionJson(void)
{
    TCHAR lszAppDataFolder[MAX_PATH];
    if (SHGetSpecialFolderPath(NULL, lszAppDataFolder, CSIDL_APPDATA, FALSE)) {
        TCHAR lszPath[MAX_PATH];
        _tcscpy_s(lszPath, lszAppDataFolder);
        _tcscat_s(lszPath, _T("\\cocoro-office\\version.json"));

        HANDLE hFile;

        hFile = CreateFile(lszPath,                // name of the write
            GENERIC_WRITE,          // open for writing
            0,                      // do not share
            NULL,                   // default security
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,  // normal file
            NULL);                  // no attr. template
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            SetAllowEveryoneAll(lszPath);
        }

    }

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
    if (lpCmdLine != NULL) {
        LPWSTR* szArglist;
        int nArgs;

        szArglist = CommandLineToArgvW(lpCmdLine, &nArgs);
        if (szArglist != NULL) {
            if (_wcsicmp(szArglist[0], L"-i") == 0) {
                CreateVersionJson();
            }
            if (_wcsicmp(szArglist[0], L"-up") == 0) {
                if (CheckRegistry(g_lpszRegistry)) {
                    if (g_dwRegData == 0x011a00cb) {    // 1.26.203
                        CreateVersionJson();
                    }
                }
            }

            LocalFree(szArglist);
        }
    }

    return 0;
#if false
    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CASETVERSIONJSON, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CASETVERSIONJSON));

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

    return (int) msg.wParam;
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CASETVERSIONJSON));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CASETVERSIONJSON);
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
