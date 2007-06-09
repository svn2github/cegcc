#include <windows.h>

int WINAPI WinMain(HINSTANCE /*hInstance*/,
                   HINSTANCE /*hPrevInstance*/,
                   LPTSTR    /*lpCmdLine*/,
                   int       /*nCmdShow*/)
{
  int count = 0;

  for (int i = 0; i < 10; i++)
    {
      printf ("count = %d\n", count++);
      Sleep (1000);
    }

  //	DWORD* zero = 0;
  //	return *zero;

  return 0;
}
