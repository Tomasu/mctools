#ifndef RENDERER_H_GUARD
#define RENDERER_H_GUARD

#include <unordered_map>
#include <utility>
#include "PairHash.h"

class ALLEGRO_EVENT_QUEUE;
class ALLEGRO_DISPLAY;
class ALLEGRO_TIMER;
class ALLEGRO_SHADER;
class ALLEGRO_TRANSFORM;
class Level;
class Map;
class ChunkData;

class Renderer
{
	public:
		const static int MAX_VERTEX_BUFFERS = 16;
		
		Renderer();
		~Renderer();
		
		void setLevel(Level *);
		Level *getLevel();
		
		bool init();
		void uninit();
		void run();
		
		void draw();
		
		typedef std::pair<int32_t, int32_t> ChunkDataKey;
		ChunkDataKey getChunkKey(int32_t x, int32_t z) const { return std::make_pair(x, z); }
		
	private:
		Level *level_;
		Map *dim0_;
		
		ALLEGRO_EVENT_QUEUE *queue_;
		ALLEGRO_TIMER *tmr_;
		ALLEGRO_DISPLAY *dpy_;
		ALLEGRO_SHADER *prg_;
		
		void processChunk(int x, int z);
		bool chunkDataExists(int32_t x, int32_t z);
		bool loadShaders(const char *vertex_file_path, const char *fragment_file_path);
		
		void setupProjection(ALLEGRO_TRANSFORM *m);
		
		std::unordered_map<ChunkDataKey, ChunkData *> chunkData_;
};

#endif /* RENDERER_H_GUARD */
