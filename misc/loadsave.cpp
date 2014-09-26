#include <cstring>
#include "MCRegion.h"
#include "Chunk.h"
#include "NBT_Tag.h"
#include "NBT_Tag_Compound.h"
#include "NBT_Debug.h"

void dump_region(MCRegion *region)
{
	for(auto &chunk: region->chunks())
	{
		if(!chunk)
			continue;
		
		//NBT_Debug("dump chunk %ix%i: ", chunk->x(), chunk->z());
		printf("%s", chunk->nbt()->serialize().c_str());
	}
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("usage: %s <region-to-read> <region-to-write>\n", argv[0]);
		return -1;
	}
	
	if(strlen(argv[1]) == strlen(argv[2]) && strncmp(argv[1], argv[2], strlen(argv[1])) == 0)
	{
		printf("input and output files can not be the same.\n");
		return -1;
	}
	
	NBT_Debug("loading region %s", argv[1]);
	MCRegion *in_region = new MCRegion(argv[1]);
	if(!in_region->load())
	{
		printf("failed to load region!\n");
		return -1;
	}
	
	NBT_Debug("dump:");
	dump_region(in_region);
	
	NBT_Debug("saving region %s", argv[2]);
	if(!in_region->save(argv[2]))
	{
		printf("failed to save region :(\n");
		return -1;
	}
	
	NBT_Debug("loading region %s", argv[2]);
	MCRegion *out_region = new MCRegion(argv[2]);
	if(!out_region->load())
	{
		printf("failed to load saved region :(\n");
		return -1;
	}
	
	NBT_Debug("dump:");
	dump_region(out_region);
	
	delete in_region;
	//delete out_region;
	
	return 0;
}