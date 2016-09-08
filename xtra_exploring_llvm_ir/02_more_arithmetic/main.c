// Get LLVM IR with:
// clang main.c -emit-ir -s -c
//
// Get optimized LLVM IR with:
// clang main.c -emit-r -O3 -s -c

int add(int x, int y)
{
	int z = x + y;
	return z;
}

int sub(int x, int y)
{
	int z = x * y;
	return z;
}

int div(int x, int y)
{
	int z = x / y;
	return z;
}

int mod(int x, int y)
{
	int z = x % y;
	return z;
}
