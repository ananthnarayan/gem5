#include<bits/stdc++.h>
#include "gem5/m5ops.h"
#include "gem5/asm/generic/m5ops.h"
int main()
{
	uint64_t a = 14;
	uint64_t b = 5;
	uint64_t sum = 2;
	sum = m5_mynewop(a,b);
	printf("\n%ld\n",sum);
}