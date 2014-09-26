#ifndef CHUNKDATA_H_GUARD
#define CHUNKDATA_H_GUARD


#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>

class Chunk;
class ChunkData
{
	public:
		static const uint32_t MAX_DATA_SIZE = 6291456;
		
		static ChunkData *Create(Chunk *c);
		
		void draw();
		
	protected:
		ChunkData(double *data, uint32_t size);
		~ChunkData();
		
	private:
		GLuint vbo_;
		uint32_t size_;
		double *data_;
};

#endif /* CHUNKDATA_H_GUARD */
