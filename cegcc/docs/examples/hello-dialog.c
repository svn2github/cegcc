//
// In this example, all include files have been removed just to get
// a minimal working sample.
// Bad programming practice, obviously, but it'll test the compiler
// without testing the include files.
//

typedef unsigned short wchar_t;
typedef wchar_t WCHAR;
typedef const WCHAR* LPCWSTR;
typedef void* HWND;
typedef unsigned int UINT;

int MessageBoxW ( HWND hWnd , LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

int WinMain()
{
	  MessageBoxW(0, L"HELLO!", L"H3LLO!", 0);
}
