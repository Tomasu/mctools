#ifndef RENDERER_CHUNK_H_GUARD
#define RENDERER_CHUNK_H_GUARD

#include <vector>

class NBT_Tag_List;
class NBT_Tag_Compound;
class NBT_Tag_ByteArray;

class Chunk;

class ChunkData;
class ResourceManager;


struct BlockInfo
{
	int32_t id, data;
};

class BlockAddress
{
	public:
		BlockAddress(int32_t x, int32_t y, int32_t z) : x_(x), y_(y_), z_(z)
		{
			section_ = y % 16;
			idx_ = section_*16*16 + z*16 + dx;
		}
		
		int32_t section() const { return y_ % 16; }
		int32_t x() const { return x_; }
		int32_t y() const { return y_; }
		int32_t z() const { return z_; }
		
		int32_t idx() const { return idx_; }
		
	private:
		int32_t x_, int32_t y_, int32_t z_;
		int32_t idx_;
		int32_t section_;
};

class RendererChunk
{
	public:
		static const int32_t MAX_SECTIONS = 16;
		
		RendererChunk() : 
			xpos_(-1), zpos_(-1), chunk_(nullptr), biomes_(nullptr) { }
		
		~RendererChunk();
		
		bool init(Chunk *c, ResourceManager *rm);
		
		uint32_t getSectionCount() { return sections_.size(); }
		
		const std::vector<RendererChunkSection*> sections() const { return sections_; }
		
		RendererChunkSection *section(int32_t idx);
		
		BiomeID getBiomeId(int32_t x, int32_t z);
		
		bool getBlockAddress(int32_t x, int32_t y, int32_t z, BlockAddress *addr);
		void getBlockInfo(const BlockAddress &addr, BlockInfo *bi);

		bool isBlockSolidForCull(const BlockAddress &baddr);
		
		const char *getBlockStateName(const BlockAddress &addr);
		
		int32_t xPos() const { return xpos_; }
		int32_t zPos() const { return zpos_; }
		
		void draw(ALLEGRO_TRANSFORM *trans);
		
		ChunkData *cdata() const { return cdata_; }
		
	private:
		int32_t xpos_, zpos_;
		Chunk *chunk_;
		
		RendererChunkSection *sections_[MAX_SECTIONS];

		NBT_Tag_ByteArray *biomes_;
		
		ResourceManager *rm_;
		
		// wrap ChunkData for now...
		ChunkData *cdata_;
};

#endif /* RENDERER_CHUNK_H_GUARD */

