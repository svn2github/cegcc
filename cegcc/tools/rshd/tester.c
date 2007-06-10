#include <stdio.h>
#include <windows.h>

int main ()
{
  int counter = 0;
  int i;
  for (i = 0; i < 3; i++)
    {
      printf ("counter = %d\n", counter++);
      Sleep (300);
    }

  for (i = 0; i < 3; i++)
    {
      int num;
      printf ("write a num:");
      scanf ("%d", &num);
      printf ("written = %d\n", num);
    }

  printf ("exiting\n");
  Sleep (3000);
  return 0;
}
