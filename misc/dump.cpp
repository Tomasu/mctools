#include "NBT.h"
#include "NBT_Debug.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("usage: %s <dat file to read>\n", argv[0]);
		return -1;
	}
	
	NBT_Debug("loading nbt: %s\n", argv[1]);
	
	NBT *nbt = new NBT();
	if(!nbt->load(argv[1]))
	{
		NBT_Error("failed to load nbt :(");
		delete nbt;
		return -1;
	}
	
	printf("nbt:\n%s\n", nbt->serialize().c_str());
	
	delete nbt;
	
	return 0;
}