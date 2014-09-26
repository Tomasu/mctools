#ifndef CHUNKDATA_H_GUARD
#define CHUNKDATA_H_GUARD


#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_primitives.h>

class Chunk;
class ChunkData
{
	public:
		static const uint32_t MAX_DATA_SIZE = 6291456;
		
		static ChunkData *Create(Chunk *c);
		
		void draw();
		
	protected:
		ChunkData(float *data, uint32_t size);
		~ChunkData();
		
	private:
		ALLEGRO_VERTEX_BUFFER *vbo_;
		ALLEGRO_VERTEX_DECL *vtxdecl_;
		uint32_t size_;
};

#endif /* CHUNKDATA_H_GUARD */
