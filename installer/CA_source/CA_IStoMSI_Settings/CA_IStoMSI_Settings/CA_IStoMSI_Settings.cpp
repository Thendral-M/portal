// CA_IStoMSI_Settings.cpp : DLL 用にエクスポートされる関数を定義します。
//

#include "pch.h"
#include "framework.h"
#include "CA_IStoMSI_Settings.h"

BOOL _ReadRegistry(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, PDWORD lpType, LPTSTR lpData, PDWORD lpcbData)
{
    HKEY _hKey;
    DWORD dwRet;
    DWORD dwLastError;

    if (lpSubKey == NULL || lpValueName == NULL || lpType == NULL || lpData == NULL || lpcbData == NULL) {
        return FALSE;
    }
    ZeroMemory(lpData, *lpcbData);

    dwRet = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &_hKey);
    dwLastError = GetLastError();
    if (dwRet != ERROR_SUCCESS) {
#if false
        TCHAR lszBuffer[1024];
        _stprintf_s(lszBuffer, _T("Err:RegOpenKeyEx():%d"), dwLastError);
        MessageBox(NULL, lszBuffer, NULL, MB_OK | MB_TOPMOST);
#endif
        return FALSE;
    }

    dwRet = RegQueryValueEx(_hKey, lpValueName, NULL, lpType, (LPBYTE)lpData, lpcbData);
    dwLastError = GetLastError();
    RegCloseKey(_hKey);

    if (dwRet != ERROR_SUCCESS) {
#if false
        TCHAR lszBuffer[1024];
        _stprintf_s(lszBuffer, _T("Err:RegQueryValueEx():%d"), dwLastError);
        MessageBox(NULL, lszBuffer, NULL, MB_OK | MB_TOPMOST);
#endif
        return FALSE;
    }

    return TRUE;
}

BOOL _DeleteRegistry(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName)
{
    HKEY _hKey;
    DWORD dwRet;
    DWORD dwLastError;

    if (lpSubKey == NULL || lpValueName == NULL) {
        return FALSE;
    }

    dwRet = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_WRITE, &_hKey);
    dwLastError = GetLastError();
    if (dwRet != ERROR_SUCCESS) {
#if false
        TCHAR lszBuffer[1024];
        _stprintf_s(lszBuffer, _T("Err:RegOpenKeyEx():%d"), dwLastError);
        MessageBox(NULL, lszBuffer, NULL, MB_OK | MB_TOPMOST);
#endif
        return FALSE;
    }

    dwRet = RegDeleteValue(_hKey, lpValueName);
    dwLastError = GetLastError();
    RegCloseKey(_hKey);

    if (dwRet != ERROR_SUCCESS) {
#if false
        TCHAR lszBuffer[1024];
        _stprintf_s(lszBuffer, _T("Err:RegQueryValueEx():%d"), dwLastError);
        MessageBox(NULL, lszBuffer, NULL, MB_OK | MB_TOPMOST);
#endif
        return FALSE;
    }

    return TRUE;
}

int DeleteDirectoryAndFile(LPCTSTR lpszPath)
{
    TCHAR lszPath[MAX_PATH] = { 0 };
    TCHAR lszFindPath[MAX_PATH] = { 0 };
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    ZeroMemory(&fd, sizeof(fd));
    _tcscpy_s(lszFindPath, lpszPath);
    _tcscat_s(lszFindPath, _T("\\*"));

    hFind = FindFirstFile(lszFindPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        while (1) {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                _tcscpy_s(lszPath, lpszPath);
                _tcscat_s(lszPath, _T("\\"));
                _tcscat_s(lszPath, fd.cFileName);
                DeleteFile(lszPath);
            }
            else {
                if (!(_tcscmp(fd.cFileName, _T(".")) == 0 ||
                    _tcscmp(fd.cFileName, _T("..")) == 0))
                {
                    _tcscpy_s(lszPath, lpszPath);
                    _tcscat_s(lszPath, _T("\\"));
                    _tcscat_s(lszPath, fd.cFileName);
                    DeleteDirectoryAndFile(lszPath);
                }
            }
            if (!(FindNextFile(hFind, &fd))) {
                break;
            }
        }
        FindClose(hFind);
    }
    RemoveDirectory(lpszPath);

    return 0;
}

int CopyDirectoryAndFile(LPCTSTR lpszExistingPath, LPCTSTR lpszNewPath)
{
    TCHAR lszExistingFileName[MAX_PATH] = { 0 };
    TCHAR lszNewFileName[MAX_PATH] = { 0 };
    TCHAR lszFindPath[MAX_PATH] = { 0 };
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    ZeroMemory(&fd, sizeof(fd));
    _tcscpy_s(lszFindPath, lpszExistingPath);
    _tcscat_s(lszFindPath, _T("\\*"));

    hFind = FindFirstFile(lszFindPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        while (1) {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                _tcscpy_s(lszExistingFileName, lpszExistingPath);
                _tcscat_s(lszExistingFileName, _T("\\"));
                _tcscat_s(lszExistingFileName, fd.cFileName);
                _tcscpy_s(lszNewFileName, lpszNewPath);
                _tcscat_s(lszNewFileName, _T("\\"));
                _tcscat_s(lszNewFileName, fd.cFileName);
                CopyFile(lszExistingFileName, lszNewFileName, FALSE);
            }
            else {
                if (!(_tcscmp(fd.cFileName, _T(".")) == 0 ||
                    _tcscmp(fd.cFileName, _T("..")) == 0))
                {
                    _tcscpy_s(lszExistingFileName, lpszExistingPath);
                    _tcscat_s(lszExistingFileName, _T("\\"));
                    _tcscat_s(lszExistingFileName, fd.cFileName);
                    _tcscpy_s(lszNewFileName, lpszNewPath);
                    _tcscat_s(lszNewFileName, _T("\\"));
                    _tcscat_s(lszNewFileName, fd.cFileName);
                    if (!PathFileExists(lszNewFileName)) {
                        CreateDirectory(lszNewFileName, NULL);
                    }
                    CopyDirectoryAndFile(lszExistingFileName, lszNewFileName);
                }
            }
            if (!(FindNextFile(hFind, &fd))) {
                break;
            }
        }
        FindClose(hFind);
    }

    return 0;
}

int CopyData(LPCTSTR lpszExistingPath, LPCTSTR lpszNewPath)
{
    LPCTSTR lpszCopyFile[] = {
        _T("deviceid.json"),
        _T("deviceinfo.json"),
        _T("endpoint.json"),
        NULL
    };
    TCHAR lszExistingFileName[MAX_PATH] = { 0 };
    TCHAR lszNewFileName[MAX_PATH] = { 0 };
    TCHAR lszFindPath[MAX_PATH] = { 0 };
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    ZeroMemory(&fd, sizeof(fd));
    _tcscpy_s(lszFindPath, lpszExistingPath);
    _tcscat_s(lszFindPath, _T("\\*"));

    if (!PathFileExists(lpszNewPath)) {
        CreateDirectory(lpszNewPath, NULL);
    }

    for (int i = 0; lpszCopyFile[i] != NULL; i++) {
        _tcscpy_s(lszExistingFileName, lpszExistingPath);
        _tcscat_s(lszExistingFileName, _T("\\"));
        _tcscat_s(lszExistingFileName, lpszCopyFile[i]);
        if (PathFileExists(lszExistingFileName)) {
            _tcscpy_s(lszNewFileName, lpszNewPath);
            _tcscat_s(lszNewFileName, _T("\\"));
            _tcscat_s(lszNewFileName, lpszCopyFile[i]);
            CopyFile(lszExistingFileName, lszNewFileName, FALSE);
        }
    }
    hFind = FindFirstFile(lszFindPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        while (1) {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!(_tcscmp(fd.cFileName, _T(".")) == 0 ||
                    _tcscmp(fd.cFileName, _T("..")) == 0))
                {
                    TCHAR lszPath[MAX_PATH] = { 0 };
                    _tcscpy_s(lszPath, fd.cFileName);
                    _tcslwr_s(lszPath);
                    if (_tcsstr(lszPath, _T("cache")) == NULL) {
                        _tcscpy_s(lszExistingFileName, lpszExistingPath);
                        _tcscat_s(lszExistingFileName, _T("\\"));
                        _tcscat_s(lszExistingFileName, fd.cFileName);
                        _tcscpy_s(lszNewFileName, lpszNewPath);
                        _tcscat_s(lszNewFileName, _T("\\"));
                        _tcscat_s(lszNewFileName, fd.cFileName);
                        if (!PathFileExists(lszNewFileName)) {
                            CreateDirectory(lszNewFileName, NULL);
                        }
                        CopyDirectoryAndFile(lszExistingFileName, lszNewFileName);
                    }
                }
            }
            if (!(FindNextFile(hFind, &fd))) {
                break;
            }
        }
        FindClose(hFind);
    }

    return 0;
}

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


CAISTOMSISETTINGS_API UINT BackupSettings(MSIHANDLE hInstall)
{

    static TCHAR _lszData[1024] = { 0 };
    DWORD dwType;
    DWORD cbData;
    BOOL bResult;

    cbData = sizeof(_lszData);
    bResult = _ReadRegistry(
        HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}"),
        _T("DisplayVersion"),
        &dwType, _lszData, &cbData);
    if (!bResult) {
        return ERROR_SUCCESS;
    }
    MsiSetProperty(hInstall, _T("INSTALLED_PORTAL"), _lszData);
    cbData = sizeof(_lszData);
    bResult = _ReadRegistry(HKEY_CURRENT_USER,
        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
        _T("electron.app.cocoro-office"),
        &dwType, _lszData, &cbData);
    if (!bResult) {
        MsiSetProperty(hInstall, _T("DISABLE_STARTUP_PORTAL"), _T("1"));
    }
    cbData = sizeof(_lszData);
    bResult = _ReadRegistry(HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{DB17D435-FC4A-4F4A-9D54-CD05EE7D5AB8}"),
        _T("InstallLocation"),
        &dwType, _lszData, &cbData);
    if (bResult) {
        MsiSetProperty(hInstall, _T("INSTALLDIR"), _lszData);
    }

    LPTSTR lpszPath;
    HRESULT lError;
    lError = ::SHGetKnownFolderPath(
        FOLDERID_PublicDesktop,
        KF_FLAG_DEFAULT_PATH | KF_FLAG_NOT_PARENT_RELATIVE | KF_FLAG_DONT_VERIFY,
        NULL,
        &lpszPath
    );
    if (SUCCEEDED(lError))
    {
        TCHAR lszDesktopShortcutPath[MAX_PATH];

        _stprintf_s(lszDesktopShortcutPath, _T("%s\\COCORO OFFICE.lnk"), lpszPath);
        bResult = PathFileExists(lszDesktopShortcutPath);
        if (!bResult) {
            DWORD dwLastError = GetLastError();
            if (dwLastError == ERROR_FILE_NOT_FOUND) {
                MsiSetProperty(hInstall, _T("CHECKDESKTOPSHORTCUT"), _T("0"));
            }
        }
        // リソース開放
        ::CoTaskMemFree(lpszPath);
    }

    return ERROR_SUCCESS;
}

CAISTOMSISETTINGS_API UINT BackupAppData(MSIHANDLE hInstall)
{
    CallTaskKill(_T("cocoro-office.exe"));

    LPTSTR lpszPath;
    LRESULT lError;
    BOOL bResult;

    lError = ::SHGetKnownFolderPath(
        FOLDERID_RoamingAppData,
        KF_FLAG_DEFAULT_PATH | KF_FLAG_NOT_PARENT_RELATIVE | KF_FLAG_DONT_VERIFY,
        NULL,
        &lpszPath
    );
    if (SUCCEEDED(lError))
    {
        TCHAR lszDataFolder[MAX_PATH];
        TCHAR lszBackupFolder[MAX_PATH];

        _stprintf_s(lszDataFolder, _T("%s\\cocoro-office"), lpszPath);
        _stprintf_s(lszBackupFolder, _T("%s\\cocoro-office.backup"), lpszPath);

#if false
        TCHAR lszMessage[1024];
        _stprintf_s(lszMessage, _T("%s\r\n%s"), lszDataFolder, lszBackupFolder);
        PMSIHANDLE hRecord = MsiCreateRecord(1);
        MsiRecordSetString(hRecord, 0, lszMessage);
        MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER | MB_OK), hRecord);
#endif

        bResult = PathIsDirectory(lszDataFolder);
        if (bResult) {
            int iResult;
            if (PathIsDirectory(lszBackupFolder)) {
                DeleteDirectoryAndFile(lszBackupFolder);
            }
            //iResult = _trename(lszDataFolder, lszBackupFolder);
            if (PathIsDirectory(lszDataFolder)) {
                iResult = CopyData(lszDataFolder, lszBackupFolder);
                DeleteDirectoryAndFile(lszDataFolder);
            }
        }
        // リソース開放
        ::CoTaskMemFree(lpszPath);
    }

    return ERROR_SUCCESS;
}

CAISTOMSISETTINGS_API UINT DisableStartupPortal(MSIHANDLE hInstall)
{
#if false
    PMSIHANDLE hRecord = MsiCreateRecord(1);
    MsiRecordSetString(hRecord, 0, _T("DisableStartupPortal()"));
    int iMessageResult = MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER | MB_OK), hRecord);
#endif

    _DeleteRegistry(HKEY_CURRENT_USER,
        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
        _T("electron.app.cocoro-office"));

    return ERROR_SUCCESS;
}

CAISTOMSISETTINGS_API UINT RestoreAppData(MSIHANDLE hInstall)
{
    LPTSTR lpszPath;
    LRESULT lError;
    BOOL bResult;

    lError = ::SHGetKnownFolderPath(
        FOLDERID_RoamingAppData,
        KF_FLAG_DEFAULT_PATH | KF_FLAG_NOT_PARENT_RELATIVE | KF_FLAG_DONT_VERIFY,
        NULL,
        &lpszPath
    );
    if (SUCCEEDED(lError))
    {
        TCHAR lszDataFolder[MAX_PATH];
        TCHAR lszBackupFolder[MAX_PATH];

        _stprintf_s(lszDataFolder, _T("%s\\cocoro-office"), lpszPath);
        _stprintf_s(lszBackupFolder, _T("%s\\cocoro-office.backup"), lpszPath);

#if false
        TCHAR lszMessage[1024];
        _stprintf_s(lszMessage, _T("%s\r\n%s"), lszDataFolder, lszBackupFolder);
        PMSIHANDLE hRecord = MsiCreateRecord(1);
        MsiRecordSetString(hRecord, 0, lszMessage);
        MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER | MB_OK), hRecord);
#endif

        bResult = PathIsDirectory(lszBackupFolder);
        if (bResult) {
            int iResult;
            if (PathIsDirectory(lszDataFolder)) {
                DeleteDirectoryAndFile(lszDataFolder);
            }
            //iResult = _trename(lszBackupFolder, lszDataFolder);
            if (PathIsDirectory(lszBackupFolder)) {
                iResult = CopyData(lszBackupFolder, lszDataFolder);
                DeleteDirectoryAndFile(lszBackupFolder);
            }
        }
        // リソース開放
        ::CoTaskMemFree(lpszPath);
    }

    return ERROR_SUCCESS;
}

CAISTOMSISETTINGS_API UINT CreateVersionJson(MSIHANDLE hInstall)
{
    LPTSTR lpszPath;
    LRESULT lError;
    BOOL bResult;

    lError = ::SHGetKnownFolderPath(
        FOLDERID_RoamingAppData,
        KF_FLAG_DEFAULT_PATH | KF_FLAG_NOT_PARENT_RELATIVE | KF_FLAG_DONT_VERIFY,
        NULL,
        &lpszPath
    );
    if (SUCCEEDED(lError))
    {
        TCHAR lszPath[MAX_PATH];
        _stprintf_s(lszPath, _T("%s\\cocoro-office\\version.json"), lpszPath);

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
        }
        // リソース開放
        ::CoTaskMemFree(lpszPath);
    }

    return ERROR_SUCCESS;
}

#if false
// これは、エクスポートされた変数の例です
CAISTOMSISETTINGS_API int nCAIStoMSISettings=0;

// これは、エクスポートされた関数の例です。
CAISTOMSISETTINGS_API int fnCAIStoMSISettings(void)
{
    return 0;
}

// これは、エクスポートされたクラスのコンストラクターです。
CCAIStoMSISettings::CCAIStoMSISettings()
{
    return;
}
#endif
