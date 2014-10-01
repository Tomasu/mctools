#ifndef CHUNKDATA_H_GUARD
#define CHUNKDATA_H_GUARD


#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_primitives.h>

class ResourceManager;
class Chunk;
class CUSTOM_VERTEX;
class ChunkData
{
	public:
		static const uint32_t MAX_VERTS = 2359296; // half of the max amount a chunk can possibly fill
		
		static ChunkData *Create(Chunk *c, ResourceManager *rm);
		
		void draw();
		
	protected:
		ChunkData(CUSTOM_VERTEX *data, uint32_t size);
		~ChunkData();
		
	private:
		ALLEGRO_VERTEX_BUFFER *vbo_;
		ALLEGRO_VERTEX_DECL *vtxdecl_;
		uint32_t size_;
};

#endif /* CHUNKDATA_H_GUARD */
