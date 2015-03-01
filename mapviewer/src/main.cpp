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
#include <cstdio>
#include <ioaccess/IOAccess.h>
#include "AllegroIOAccessInterface.h"

// TODO: add options: 
//		--list-versions	show all detected versions
//		--select-version	select a specific version
//		--path				base dir to look for versions (and levels/saves if --level isn't given)
std::vector<TGOOptionBase *> options = {
	new TGOStringOption("level"),
	new TGOBoolOption("list-versions"),
	new TGOBoolOption("list-levels"),
	new TGOStringOption("select-version"),
	new TGOStringOption("path")
};

int main(int argc, const char **argv)
{
	TGOParser *optParser = new TGOParser(options);
	Level *level = nullptr;
	Renderer *renderer = nullptr;
	Minecraft *minecraft = nullptr;
	
	if(!optParser->validate(argc, argv))
	{
		NBT_Debug("invalid options...");
		return -1;
	}
	
	IOAccess::Initialize();
	
	al_init();
	
	
	bool listVersions = optParser->getValue<TGOBoolOption>("list-versions");
	bool listLevels = optParser->getValue<TGOBoolOption>("list-levels");
	std::string selectedVersion = optParser->getValue<TGOStringOption>("select-version");
	std::string levelPath = optParser->getValue<TGOStringOption>("level");
	std::string basePath = optParser->getValue<TGOStringOption>("path");
	
	minecraft = Minecraft::Create(basePath, levelPath);
	if(!minecraft)
	{
		NBT_Debug("Failed to init minecraft :(");
		goto err;
	}
	
	if(listVersions)
	{
		printf("Versions:\n");
		for(auto &version: minecraft->versionMap())
		{
			printf("\t%s\n", version.first.str().c_str());
		}
		
		goto escape;
	}
	
	if(listLevels)
	{
		printf("Saves:\n");
		for(auto &save: minecraft->saves())
		{
			printf("\t%s\n", save.c_str());
		}
		
		goto escape;
	}
	
	if(selectedVersion.length())
	{
		printf("selected version: %s\n", selectedVersion.c_str());
		if(!minecraft->selectVersion(selectedVersion))
		{
			NBT_Error("selected version %s not found", selectedVersion.c_str());
			goto err;
		}
	}
	else
		minecraft->autoSelectVersion();
	
	IOAccess::SetDefaultInterface(new IOAccess::AllegroInterface());
	NBT_Debug("create Level");
	level = new Level();
	if(!level->load(minecraft->saves().at(0)))
	{
		NBT_Debug("exiting...");
		goto err;
	}
	
	if(!level->maps().size())
	{
		NBT_Debug("failed to load level at %s", minecraft->saves().at(0).c_str());
		goto err;
	}
	
	renderer = new Renderer();
	if(!renderer->init(minecraft, argv[0]))
	{
		NBT_Debug("failed to init renderer");
		goto err;
	}
	
	renderer->setLevel(level);
	
	renderer->run();

	delete renderer;
	delete level;
	delete minecraft;
	delete optParser;
	
	//sleep(60);
	
	NBT_Debug("eor");
	
	return 0;
	
escape:
	NBT_Debug("ESCAPE!");
	delete renderer;
	delete optParser;
	return 0;
	
err:
	NBT_Error("ERROR!");
	delete level;
	delete renderer;
	delete optParser;
	return -1;
}
