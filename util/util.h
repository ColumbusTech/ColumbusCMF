#include <cstdio>

struct Vertex
{
	float X, Y, Z;
	float U, V;
	float NX, NY, NZ;
};

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void PrintProgress(double Percentage)
{
    int val = (int) (Percentage * 100);
    int lpad = (int) (Percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush (stdout);
}











