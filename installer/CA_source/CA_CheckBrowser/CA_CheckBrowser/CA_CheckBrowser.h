// 以下の ifdef ブロックは、DLL からのエクスポートを容易にするマクロを作成するための
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された CACHECKBROWSER_EXPORTS
// シンボルを使用してコンパイルされます。このシンボルは、この DLL を使用するプロジェクトでは定義できません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、
// CACHECKBROWSER_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef CACHECKBROWSER_EXPORTS
#define CACHECKBROWSER_API __declspec(dllexport)
#else
#define CACHECKBROWSER_API __declspec(dllimport)
#endif

extern "C" {
	CACHECKBROWSER_API UINT CheckBrowser(MSIHANDLE hInstall);
}

#if false
// このクラスは dll からエクスポートされました
class CACHECKBROWSER_API CCACheckBrowser {
public:
	CCACheckBrowser(void);
	// TODO: メソッドをここに追加します。
};

extern CACHECKBROWSER_API int nCACheckBrowser;

CACHECKBROWSER_API int fnCACheckBrowser(void);
#endif

