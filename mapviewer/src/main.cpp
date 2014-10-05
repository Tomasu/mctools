#include <allegro5/allegro.h>
#include <dirent.h>
#include "Map.h"
#include <Level.h>
#include <tgetopt.h>
#include <Renderer.h>
#include <NBT.h>
#include <NBT_Debug.h>
#include <sstream>

std::vector<TGOOptionBase *> options = {
	new TGOStringOption("level")
};

Level *loadLevel(const std::string &path_)
{
	Level *level = new Level();
	std::string path = path_;
	
	if(!path.size())
	{
		const char *home = getenv("HOME");
		if(!home)
		{
			NBT_Debug("failed to get HOME directory");
			return nullptr;
		}
		
		std::string mcLevelPath = std::string(home) + "/.minecraft/saves/";
		DIR *mcDir = opendir(mcLevelPath.c_str());
		if(!mcDir)
		{
			NBT_Debug("failed to list %s directory", mcLevelPath.c_str());
			return nullptr;
		}
		
		struct dirent *dent;
		while((dent = readdir(mcDir)))
		{
			if(dent->d_name[0] == '.')
				continue;
			
			if(dent->d_type == DT_DIR)
			{
			
				std::stringstream sstream;
				sstream << mcLevelPath;
				sstream << "/";
				sstream << dent->d_name;

				path = sstream.str();
				if(level->load(path))
					break;
			}
		}
	}
	else
		level->load(path);
	
	if(!level->maps().size())
	{
		NBT_Debug("failed to load level at %s", path.c_str());
		return nullptr;
	}
	
	return level;
}

int main(int argc, const char **argv)
{
	TGOParser *optParser = new TGOParser(options);
	
	if(!optParser->validate(argc, argv))
	{
		NBT_Debug("invalid options...");
		return -1;
	}
	
	Renderer *renderer = new Renderer();
	
	std::string levelPath = optParser->getValue<TGOStringOption>("level");
	Level *level = loadLevel(levelPath);
	if(!level)
	{
		delete renderer;
		NBT_Debug("exiting...");
		return -1;
	}
	
	if(!renderer->init())
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
