#include <stdio.h>
#include <windows.h>

int main ()
{
  int i;
  for (i = 0; i < 3; i++)
    {
      int num;
      printf ("write a num: ");
      fflush (stdout);
      scanf ("%d", &num);
      printf ("written = %d\n", num);
    }

  printf ("exiting\n");
  return 0;
}
