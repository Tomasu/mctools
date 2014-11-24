#ifndef RENDERER_H_GUARD
#define RENDERER_H_GUARD

#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>
#include <allegro5/allegro.h>

#include "PairHash.h"
#include "Util.h"

struct ALLEGRO_FONT;
class Vector3D;
class ResourceManager;
class Level;
class Map;
class ChunkData;
class AtlasSheet;

class Renderer
{
	public:
		const static int MAX_VERTEX_BUFFERS = 16;
		
		Renderer();
		~Renderer();
		
		void setLevel(Level *);
		Level *getLevel();
		
		bool init(const char *argv0);
		void uninit();
		void run();
		
		void drawHud();
		void draw();
		
		typedef std::pair<int32_t, int32_t> ChunkDataKey;
		ChunkDataKey getChunkKey(int32_t x, int32_t z) const { return std::make_pair(x, z); }
		
		bool setShaderSampler(AtlasSheet *sheet);
		void unsetShaderSampler(AtlasSheet *sheet);
		
		enum ShaderType {
			SHADER_DEFAULT = 1,
			SHADER_ALLEGRO
		};
		
		bool setShader(ShaderType type);
		
	private:
		Level *level_;
		Map *dim0_;
		uint32_t vao_;
		
		bool key_state_[ALLEGRO_KEY_MAX];
		
		ALLEGRO_EVENT_QUEUE *queue_;
		ALLEGRO_TIMER *tmr_;
		ALLEGRO_DISPLAY *dpy_;
		ALLEGRO_SHADER *prg_;
		ALLEGRO_SHADER *al_prg_;
		ALLEGRO_BITMAP *bmp_;
		ALLEGRO_FONT *fnt_;
		
		ALLEGRO_TRANSFORM al_proj_transform_;
		
		ALLEGRO_TRANSFORM camera_transform_;
		float rx_look;
		float ry_look;
		
		ResourceManager *resManager_;
		
		std::queue<std::pair<int32_t, int32_t>> loadChunkQueue;
		
		void processChunk(int x, int z);
		void autoLoadChunks(int x, int z);
		bool isChunkVisible(Vector3D origin, Vector3D pos);
		
		bool chunkDataExists(int32_t x, int32_t z);
		bool loadShaders(const char *vertex_file_path, const char *fragment_file_path);
		bool loadAllegroShaders();
		void setupProjection(ALLEGRO_TRANSFORM *m);
		
		std::unordered_map<ChunkDataKey, ChunkData *> chunkData_;
};

#endif /* RENDERER_H_GUARD */
