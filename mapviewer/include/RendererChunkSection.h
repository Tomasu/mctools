#ifndef RENDERER_CHUNK_SECTION_H_GUARD
#define RENDERER_CHUNK_SECTION_H_GUARD

class NBT_Tag_Compound;
class NBT_Tag_ByteArray;

class RendererChunkSection
{
	public:
		RendererChunkSection() : 
		id_(-1), section_(nullptr), block_ids_(nullptr),
		block_add_(nullptr), block_data_(nullptr) { };
		
		~RendererChunkSection() { }
		
		bool init(int32_t id, NBT_Tag_Compound *section);
		
		int32_t getY() const { return y_; }
		int32_t getIdx() const { return idx_; }
		
		bool getBlockInfo(const BlockAddress &addr, BlockInfo *info);
		const char *getBlockStateName(const BlockAddress &addr);
		
	private:
		int32_t idx_;
		int32_t y_;
		
		NBT_Tag_Compound *section_;
		
		NBT_Tag_ByteArray *block_ids_;
		NBT_Tag_ByteArray *block_add_;
		NBT_Tag_ByteArray *block_data_;
		
};

#endif /* RENDERER_CHUNK_SECTION_H_GUARD */
