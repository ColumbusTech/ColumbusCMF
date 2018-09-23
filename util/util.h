#include <cstdio>
#include <iostream>
#include <iterator>
#include <algorithm>

struct Vertex
{
	float X, Y, Z;
	float U, V;
	float NX, NY, NZ;
};

#define PBSTR "||||||||||||||||||||||||||||||"
#define PBWIDTH 30

void PrintProgress(double Percentage)
{
    int Value  = (int)(Percentage * 100);
    int LValue = (int)(Percentage * PBWIDTH);
    int RValue = PBWIDTH - LValue;
	printf("\r[%.*s%*s] %d%%", LValue, PBSTR, RValue, "", Value);
	fflush(stdout);
}











