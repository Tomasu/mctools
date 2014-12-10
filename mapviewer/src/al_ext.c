#include "al_ext.h"
#include <allegro5/allegro.h>

void al_transform_coordinates_3d(const ALLEGRO_TRANSFORM *trans, float *x, float *y, float *z)
{
	float t,s;

	t = *x;
	s = *y;
	
	*x = t * trans->m[0][0] + s * trans->m[1][0] + *z * trans->m[2][0] + trans->m[3][0];
	*y = t * trans->m[0][1] + s * trans->m[1][1] + *z * trans->m[2][1] + trans->m[3][1];
	*z = t * trans->m[0][2] + s * trans->m[1][2] + *z * trans->m[2][2] + trans->m[3][2];
}
