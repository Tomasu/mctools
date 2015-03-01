#include "ioaccess/IOAccess.h"
#include "AllegroIOAccessInterface.h"

#include <allegro5/allegro5.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

#include "NBT_Debug.h"

namespace IOAccess
{

	File *AllegroInterface::openFile(const std::string &path, const std::string &mode)
	{
		AllegroFile *fh = new AllegroFile();
		if(!fh->open(path, mode))
		{
			delete fh;
			return nullptr;
		}

		return fh;
	}

	Directory *AllegroInterface::openDir(const std::string &path)
	{
		AllegroDirectory *fh = new AllegroDirectory();
		if(!fh->open(path))
		{
			delete fh;
			return nullptr;
		}

		return fh;
	}

	bool AllegroInterface::stat(const std::string &path, StatInfo *si)
	{
		ALLEGRO_FS_ENTRY *ent = al_create_fs_entry(path.c_str());
		if(checkError(ent == nullptr))
			return false;

		// allegro doesn't export device
		si->device = -1;

		uint32_t mode = al_get_fs_entry_mode(ent);
		NBT_Debug("mode: %i", mode);
		si->mode = 0;
		si->mode |= (mode & ALLEGRO_FILEMODE_READ) ? IOAccess::ModeOwnerRead : 0;
		si->mode |= (mode & ALLEGRO_FILEMODE_WRITE) ? IOAccess::ModeOwnerWrite : 0;
		si->mode |= (mode & ALLEGRO_FILEMODE_EXECUTE) ? IOAccess::ModeOwnerExecute : 0;
		// FIXME: implement this
		// si->mode |= (mode & ALLEGRO_FILEMODE_HIDDEN) ? WAT : 0;

		si->mode |= (mode & ALLEGRO_FILEMODE_ISFILE) ? IOAccess::ModeRegular : 0;
		si->mode |= (mode & ALLEGRO_FILEMODE_ISDIR) ? IOAccess::ModeDir : 0;

		si->nlink = 1; // not provided by allegro
	//	si->uid = geteuid(); // not provided by allegro, fake it with current uid
//		si->gid = getegid(); // not provided by allegro, fake it with current gid
		si->gid = si->uid = 0;
		si->rdev = -1; // not provided by allegro
		si->size = al_get_fs_entry_size(ent);

		si->atime.sec = al_get_fs_entry_atime(ent);
		si->atime.nsec = 0;

		si->ctime.sec = al_get_fs_entry_ctime(ent);
		si->ctime.nsec = 0;

		si->mtime.sec = al_get_fs_entry_mtime(ent);
		si->mtime.nsec = 0;

		return true;
	}

	bool AllegroInterface::exists(const std::string &path)
	{
		bool ret = al_filename_exists(path.c_str());
		if(checkError(!ret))
			return false;

		return true;
	}

	int32_t AllegroInterface::getErrno()
	{
		return errno_;
	}

	bool AllegroInterface::checkError(bool err)
	{
		if(err)
		{
			errno_ = al_get_errno();
			return true;
		}

		errno_ = 0;
		return false;
	}

	AllegroFile::~AllegroFile()
	{
		if(fh_)
			close();

		errno_ = 0;
	}

	bool AllegroFile::checkError(bool err)
	{
		if(err)
		{
			errno_ = al_get_errno();
			return true;
		}

		errno_ = 0;
		return false;
	}

	bool AllegroFile::open(const std::string &path, const std::string &mode)
	{
		fh_ = al_fopen(path.c_str(), mode.c_str());
		if(checkError(fh_ == nullptr))
			return false;

		return true;
	}

	size_t AllegroFile::read(void *ptr, size_t len)
	{
		size_t ret = al_fread(fh_, ptr, len);
		checkError(ret != len);

		return ret;
	}

	size_t AllegroFile::write(void *ptr, size_t len)
	{
		size_t ret = al_fwrite(fh_, ptr, len);
		checkError(ret != len);

		return ret;
	}

	void AllegroFile::close()
	{
		bool ret = al_fclose(fh_);
		if(checkError(!ret))
			return;

		fh_ = nullptr;
		errno_ = 0;
	}

	bool AllegroFile::eof()
	{
		return al_feof(fh_);
	}

	bool AllegroFile::error()
	{
		return al_ferror(fh_);
	}

	int32_t AllegroFile::getErrno()
	{
		return errno_;
	}

	bool AllegroDirectory::checkError(bool err)
	{
		if(err)
		{
			errno_ = errno;
			return true;
		}

		errno_ = 0;
		return false;
	}

	AllegroDirectory::~AllegroDirectory()
	{
		if(dir_)
			close();

		errno_ = 0;
	}

	bool AllegroDirectory::open(const std::string &path)
	{
		dir_ = al_create_fs_entry(path.c_str());
		if(checkError(dir_ == nullptr))
			return false;

		if(checkError(!al_open_directory(dir_)))
		{
			al_destroy_fs_entry(dir_);
			dir_ = nullptr;
			return false;
		}

		bool isDir = al_get_fs_entry_mode(dir_) & ALLEGRO_FILEMODE_ISDIR;
		if(checkError(!isDir))
		{
			NBT_Debug("%s does not exist :(", path.c_str());
			al_destroy_fs_entry(dir_);
			dir_ = nullptr;
			return false;
		}

		path_ = path;

		return true;
	}

	bool AllegroDirectory::read(std::string *ent, bool fullpath)
	{
		std::string entname;
		std::string path;

		do {
			ALLEGRO_FS_ENTRY *dent = al_read_directory(dir_);
			if(checkError(dent == nullptr))
				break;

			entname = al_get_fs_entry_name(dent);

			if(!fullpath)
				entname = entname.substr(path_.length()+1);


			ent->assign(entname);

			al_destroy_fs_entry(dent);
			return true;

		} while(1);

		return false;
	}

	void AllegroDirectory::close()
	{
		al_destroy_fs_entry(dir_);
		dir_ = nullptr;
	}

	int32_t AllegroDirectory::getErrno()
	{
		return errno_;
	}

}
