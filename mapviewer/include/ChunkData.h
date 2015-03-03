#ifndef CHUNKDATA_H_GUARD
#define CHUNKDATA_H_GUARD


#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_primitives.h>

#include "BlockAddress.h"
#include "BlockInfo.h"

class ResourceManager;
class CustomVertex;
class Chunk;

class Chunk;

class ChunkData
{
	public:
		~ChunkData();
		
		static const uint32_t MAX_VERTS = 256 * 16 * 16 * 50; // half of the max amount a chunk can possibly fill
#ifdef VIEWER_USE_MORE_VBOS
		static const int32_t MAX_SLICES = 16;
#else
		static const int32_t MAX_SLICES = 1;
#endif		
		static ChunkData *Create(Chunk *rc, ResourceManager *rm);
		
		void draw(ALLEGRO_TRANSFORM *transform, const BlockInfo &info);
		
		int32_t x() const { return x_; }
		int32_t z() const { return z_; }
		
		bool fillSlice(int slice, CustomVertex *data, uint32_t size);
		
		Chunk *chunk() { return chunk_; }
		
	protected:
		ChunkData(Chunk *c);
		
	private:
		
		int32_t x_, z_;
		
		Chunk *chunk_;
		
		ResourceManager *rm_;
		
		struct {
			int32_t y;
			ALLEGRO_VERTEX_BUFFER *vbo;
			uint32_t vtx_count;
		} slice_[MAX_SLICES];
};

#endif /* CHUNKDATA_H_GUARD */
