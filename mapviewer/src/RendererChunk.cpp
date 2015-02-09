#include "RendererChunk.h"
#include "Resource/Manager.h"
#include "Chunk.h"
#include "ChunkData.h"

#include "NBT_Debug.h"
#include "NBT_Tag_Compound.h"
#include "NBT_Tag_List.h"
#include "NBT_Tag_Byte.h"

bool RendererChunk::init(Chunk *c, ResourceManager *rm)
{
	memset(sections_, 0, sizeof(sections_));
	
	chunk_ = c;
	rm_ = rm;
	
	NBT_Tag_Compound *nbt = chunk_->nbt()->getCompound("Level");
	if(!nbt)
	{
		NBT_Debug("Level data missing");
		return false;
	}
	
	biomes_ = (NBT_Tag_Compound*)nbt->get("Biomes");
	if(!biomes_)
	{
		NBT_Debug("Biomes data missing");
		return false;
	}
	
	xpos_ = nbt->getInt("xPos");
	zpos_ = nbt->getInt("zPos");
	
	NBT_Tag_List *sections_tag = (NBT_Tag_List*)nbt->get("Sections");
	
	for(uint32_t section_id = 0; i < sections_tag.items().size(); i++)
	{
		RendererChunkSection *rcs = new RendererChunkSection();
		if(!rcs->init(i, sections_tag[i]))
		{
			NBT_Debug("failed to initialize RendererChunkSection(%i)", i);
			delete rcs;
			goto render_chunk_bail;
		}
		
		sections[rcs->yPos()] = rcs;
	}
	
	return true;
	
render_chunk_bail:
	for(auto section: sections_)
	{
		delete section;
	}
	
	return false;
}

RendererChunk::~RendererChunk()
{
	chunk_ = nullptr;
	biomes_ = nullptr;
	
	for(auto section: sections_)
	{
		delete section;
	}
	
	delete cdata_;
	cdata_ = nullptr;
}

RendererChunkSection *RendererChunk::section(int32_t idx)
{
	if(idx < 0 || idx >= sections_.size())
		return nullptr;
	
	return sections_[idx];
}

bool RendererChunk::getBlockAddress(int32_t x, int32_t y, int32_t z, BlockAddress *baddr)
{
	BlockAddress ba(x, y, z);
	
	if(ba.idx() < 0 || ba.idx() >= 4096)
		return false;
	
	*baddr = ba;
	
	return true;
}

void RendererChunk::getBlockInfo(const BlockAddress &baddr, BlockInfo *bi)
{
	auto section = section
}


bool RendererChunkSection::init(int32_t idx, NBT_Tag_Compound *section) 
{
	y_ = section->getByte("Y");
	
	idx_ = idx;
	section_ = section;
	
	block_ids_ = section_->getByteArray("Blocks");
	if(!block_ids_)
	{
		NBT_Debug("missing Blocks data in section %i", id);
		return false;
	}
	
	block_add_ = section_->getByteArray("Add");
	if(!block_add_)
	{
		NBT_Debug("missing Add data in section %i", id);
	}
	
	block_data_ = section_->getByteArray("Data");
	if(!block_data_)
	{
		NBT_Debug("missing Data data in section %i", id);
	}
	
	cdata_ = ChunkData::Create(chunk_, rm_);
	if(!cdata_)
	{
		NBT_Debug("failed to create chunkdata for chunk @ %ix%i", xpos_, zpos_);
		return;
	}
	
	return true;
}

// just wrap ChunkData for now
// maybe later integrate it into RendererChunk?
void RendererChunk::draw(ALLEGRO_TRANSFORM *trans)
{
	return cdata_->draw(trans);
}
