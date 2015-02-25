#include <allegro5/allegro.h>
#include <dirent.h>
#include "Map.h"
#include <Level.h>
#include <Minecraft.h>
#include <tgetopt.h>
#include <Renderer.h>
#include <NBT.h>
#include <NBT_Debug.h>
#include <sstream>
#include <ioaccess/IOAccess.h>
#include "AllegroIOAccessInterface.h"

// TODO: add options: 
//		--list-versions	show all detected versions
//		--select-version	select a specific version
//		--path				base dir to look for versions (and levels/saves if --level isn't given)
std::vector<TGOOptionBase *> options = {
	new TGOStringOption("level")
};

int main(int argc, const char **argv)
{
	TGOParser *optParser = new TGOParser(options);
	
	if(!optParser->validate(argc, argv))
	{
		NBT_Debug("invalid options...");
		return -1;
	}
	
	al_init();
	IOAccess::Initialize(new IOAccess::AllegroInterface());
	
	Renderer *renderer = new Renderer();
	
	std::string levelPath = optParser->getValue<TGOStringOption>("level");
	
	Minecraft *minecraft = Minecraft::Create("", levelPath);
	if(!minecraft)
	{
		NBT_Debug("Failed to init minecraft :(");
		return -1;
	}
	
	minecraft->autoSelectVersion();
	
	Level *level = new Level();
	if(!level->load(minecraft->saves().at(0)))
	{
		delete level;
		delete renderer;
		NBT_Debug("exiting...");
		return -1;
	}
	
	if(!level->maps().size())
	{
		delete level;
		delete renderer;
		NBT_Debug("failed to load level at %s", minecraft->saves().at(0).c_str());
		return -1;
	}
	
	if(!renderer->init(minecraft, argv[0]))
	{
		NBT_Debug("failed to init renderer");
		delete renderer;
		delete level;
		return -1;
	}
	
	renderer->setLevel(level);
	
	renderer->run();
	NBT_Debug("eor");

	delete renderer;
	delete level;
	
	return 0;
}
