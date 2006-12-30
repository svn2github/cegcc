/*
 * Profiling test
 */
void func_a(int i)
{
	/* Spend some time */
	int	c, j;

	for (c=0; c < i; c++)
		for (j=0; j<100; j++)
			;

	return;
}
