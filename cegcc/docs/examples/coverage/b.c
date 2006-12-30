/*
 * Profiling test
 */
int fibo(int x)
{
	if (x < 3)
		return 1;
	return fibo(x-1) + fibo(x-2);
}

int func_b(int i)
{
	return fibo(i);
}
