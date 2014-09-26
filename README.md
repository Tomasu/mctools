README
======

This project contains a few miscelaneous tools for working with minecraft files.
It's all currently a work in progress and will not be all that useful to most people (except maybe trimmap).

Working tools
-------------

### trimmap ###

scans through a map and delete all chunks it doesn't think are important,

### qnbt ###

is a very basic nbt/dat viewer. it will eventually be able to edit those files as well.

### loadsave ###

is a dumb program that just loads a region file dumps the structure, saves it, then loads the saved file back in and dumps it's structure. Not very useful.

Non working tools
----------------

### procmap ###

trimmap's big brother. will support doing any kind of processing of a minecraft save. uses lua to customize the actions.

### mapviewer ###

An OpenGL 3 powered minecraft map viewer.

Dependencies
------------

### Required ###

- [libnbt](https://github.com/Tomasu/libnbt) (all)
- [libmcmap](https://github.com/Tomasu/libmcmap) (trimmap, loadsave, procmap, mapviewer)

### Optional ###

- [Qt4/5](http://qt-project.org/) (qnbt)
- [LuaGlue](https://github.com/Tomasu/LuaGlue) (procmap)
- [Lua 5.2](http://www.lua.org/) (procmap)
- [Allegro 5.1+](http://alleg.sf.net) (mapviewer)
- OpenGL 3+ (mapviewer)

LICENSE
-------

Curently licensed under GPL3.
