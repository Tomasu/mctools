#ifndef IOACCESS_ALLEGROINTERFACE_H_GUARD
#define IOACCESS_ALLEGROINTERFACE_H_GUARD

#include <string>

#include "ioaccess/IOAccess.h"

struct ALLEGRO_FILE;
struct ALLEGRO_FS_ENTRY;

namespace IOAccess
{
	class AllegroInterface : public FileSystemInterface
	{
		public:
			File *openFile(const std::string &path, const std::string &mode);
			Directory *openDir(const std::string &path);
			bool stat(const std::string &path, StatInfo *si);
			bool exists(const std::string &path);
			int32_t getErrno();
		
		private:
			int32_t errno_;
			
			bool checkError(bool e);
	};
	
	class AllegroFile : public File
	{
		public:
			AllegroFile() : fh_(nullptr), errno_(0) { };
			virtual ~AllegroFile();
			
			bool open(const std::string &path, const std::string &mode);
			size_t read(void *ptr, size_t len);
			size_t write(void *ptr, size_t len);
			
			void close();
			
			bool eof();
			bool error();
			int32_t getErrno();
		
		private:
			ALLEGRO_FILE *fh_;
			int32_t errno_;
			
			bool checkError(bool e);
	};
	
	class AllegroDirectory : public Directory
	{
		public:
			AllegroDirectory() : path_(), dir_(nullptr), errno_(0) {}
			virtual ~AllegroDirectory();
			
			bool open(const std::string &path);
			 bool read(std::string* ent, bool fullpath);
			void close();
			int32_t getErrno();
			
		private:
			std::string path_;
			ALLEGRO_FS_ENTRY *dir_;
			int32_t errno_;
			
			bool checkError(bool e);
	};
}

#endif /* IOACCESS_STDIOINTERFACE_H_GUARD */
