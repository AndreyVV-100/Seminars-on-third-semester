#include <stdio.h>

struct ts
{
	int a;
	char* b;
};

int f (int i, int j);

int main (int argc, char** argv)
{
	int i = 0, j = 3;
	struct ts* a = NULL;
	i = 5 + j;
	for (; i >j; i--)
		j += i;
	printf ("%d\n", i);
}

int f (int i, int j)
{
	int t;
	t = i + j;
	return t;
}
