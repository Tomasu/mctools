#ifndef CUSTOMVERTEX_H_GUARD
#define CUSTOMVERTEX_H_GUARD

#include <allegro5/allegro.h>

struct VF4
{
	float f1, f2, f3, f4;
};

struct VF3
{
	float f1, f2, f3;
};

struct VF2
{
	float f1, f2;
};

struct VI3 {
	union {
		int32_t i[3];
		int32_t i1, i2, i3;
	};
};

struct CUSTOM_VERTEX
{
   VF3 pos;
	VF2 txcoord;
	float tx_page;
	ALLEGRO_COLOR color;
};

#endif /* CUSTOMVERTEX_H_GUARD */
