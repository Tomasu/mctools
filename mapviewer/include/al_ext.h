#ifndef AL_EXT_H_GUARD
#define AL_EXT_H_GUARD

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct ALLEGRO_TRANSFORM ALLEGRO_TRANSFORM;
	void al_transform_coordinates_3d(const ALLEGRO_TRANSFORM *trans, float *x, float *y, float *z);
	void al_unproject_transform_3d(const ALLEGRO_TRANSFORM *trans, float *x, float *y, float *z);
	
#ifdef __cplusplus
}
#endif

#endif /* AL_EXT_H_GUARD */
