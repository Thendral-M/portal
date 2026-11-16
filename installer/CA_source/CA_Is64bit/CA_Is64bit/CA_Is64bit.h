// 以下の ifdef ブロックは、DLL からのエクスポートを容易にするマクロを作成するための
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された CAIS64BIT_EXPORTS
// シンボルを使用してコンパイルされます。このシンボルは、この DLL を使用するプロジェクトでは定義できません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、
// CAIS64BIT_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef CAIS64BIT_EXPORTS
#define CAIS64BIT_API __declspec(dllexport)
#else
#define CAIS64BIT_API __declspec(dllimport)
#endif

extern "C" {
	CAIS64BIT_API UINT Is64bit(MSIHANDLE hInstall);
}

#if false
// このクラスは dll からエクスポートされました
class CAIS64BIT_API CCAIs64bit {
public:
	CCAIs64bit(void);
	// TODO: メソッドをここに追加します。
};

extern CAIS64BIT_API int nCAIs64bit;

CAIS64BIT_API int fnCAIs64bit(void);
#endif

