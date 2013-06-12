#include "BitMap.h"
#include <cmath>

void horizLine(BitMap *bmp, int x1, int y1, int len)
{
	for(int i = x1; i < x1+len; i++)
	{
		bmp->set(i, y1);
	}
}

void circleFill(BitMap *bmp, int x, int y, int r)
{
	int x1;
	int x2;

	int counter=(y+r);

	for(int count=(y-r);count<=counter;count++)
	{
		x1=int(x+sqrt((r*r)-((count-y)*(count-y)))+0.5);
		x2=int(x-sqrt((r*r)-((count-y)*(count-y)))+0.5);

		horizLine(bmp, x2, count, abs(x1-x2));
	}
}