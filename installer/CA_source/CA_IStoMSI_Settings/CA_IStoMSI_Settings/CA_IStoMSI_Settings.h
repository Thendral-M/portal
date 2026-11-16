// 以下の ifdef ブロックは、DLL からのエクスポートを容易にするマクロを作成するための
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された CAISTOMSISETTINGS_EXPORTS
// シンボルを使用してコンパイルされます。このシンボルは、この DLL を使用するプロジェクトでは定義できません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、
// CAISTOMSISETTINGS_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef CAISTOMSISETTINGS_EXPORTS
#define CAISTOMSISETTINGS_API __declspec(dllexport)
#else
#define CAISTOMSISETTINGS_API __declspec(dllimport)
#endif

extern "C" {
	CAISTOMSISETTINGS_API UINT BackupSettings(MSIHANDLE hInstall);
	CAISTOMSISETTINGS_API UINT BackupAppData(MSIHANDLE hInstall);
	CAISTOMSISETTINGS_API UINT DisableStartupPortal(MSIHANDLE hInstall);
	CAISTOMSISETTINGS_API UINT RestoreAppData(MSIHANDLE hInstall);
	CAISTOMSISETTINGS_API UINT CreateVersionJson(MSIHANDLE hInstall);
}

#if false
// このクラスは dll からエクスポートされました
class CAISTOMSISETTINGS_API CCAIStoMSISettings {
public:
	CCAIStoMSISettings(void);
	// TODO: メソッドをここに追加します。
};

extern CAISTOMSISETTINGS_API int nCAIStoMSISettings;

CAISTOMSISETTINGS_API int fnCAIStoMSISettings(void);
#endif
