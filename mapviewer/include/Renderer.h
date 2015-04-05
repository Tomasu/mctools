#ifndef RENDERER_H_GUARD
#define RENDERER_H_GUARD

#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>
#include <allegro5/allegro.h>

#include "glm/fwd.hpp"

#include "PairHash.h"
#include "Util.h"
#include "Vector.h"
#include "Chunk.h"
#include "BlockAddress.h"
#include "BlockInfo.h"
#include "Camera.h"

class Ray;
struct ALLEGRO_FONT;
class ResourceManager;
class Minecraft;
class Level;
class Map;
class ChunkData;
class AtlasSheet;
class ChunkData;



class Renderer
{
	public:
		const static int MAX_VERTEX_BUFFERS = 16;
		
		Renderer();
		~Renderer();
		
		void setLevel(Level *);
		Level *getLevel();
		
		bool init(Minecraft *mc, const char *argv0);
		void uninit();
		void run();
		
		void drawHud();
		void draw();
		
		Chunk::Key getChunkKey(int32_t x, int32_t z) const { return std::make_pair(x, z); }
		
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
		bool grab_mouse_;
		
		ALLEGRO_EVENT_QUEUE *queue_;
		ALLEGRO_TIMER *tmr_;
		ALLEGRO_DISPLAY *dpy_;
		ALLEGRO_SHADER *prg_;
		ALLEGRO_SHADER *al_prg_;
		ALLEGRO_BITMAP *bmp_;
		ALLEGRO_FONT *fnt_;
		
		glm::mat4x4 projection_;
		
		Camera camera_;
	
		int mouse_x, mouse_y;
		
		glm::vec3 look_pos_;
		BlockAddress look_block_address_;
		BlockInfo look_block_info_;
		
		ALLEGRO_TRANSFORM al_proj_transform_;
		
		ResourceManager *resManager_;
		
		std::queue<Chunk::Key> loadChunkQueue;
		
		void processChunk(int x, int z);
		void autoLoadChunks(int x, int z);
		bool isChunkVisible(Vector3D origin, Vector3D pos);
		
		bool chunkDataExists(int32_t x, int32_t z);
		bool loadShaders(const char *vertex_file_path, const char *fragment_file_path);
		bool loadAllegroShaders();
		void setupProjection(ALLEGRO_TRANSFORM *m);
		
		void updateLookPos();
		
		bool rayBlockFaceIntersects(const glm::vec3 &orig, const Ray& ray, glm::vec3 &out, float &distance);
		bool lookCollision(const Ray &ray, BlockInfo &outInfo, float &distance);

		void drawSelection();
		
		bool getBlockInfo(const glm::vec3 &block, BlockInfo &blockInfo);
		
		std::unordered_map<Chunk::Key, ChunkData *> chunkData_;
};

#endif /* RENDERER_H_GUARD */
