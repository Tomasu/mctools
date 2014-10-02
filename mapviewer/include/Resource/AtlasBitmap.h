#ifndef ATLASBITMAP_H_GUARD
#define ATLASBITMAP_H_GUARD

#include <stdint.h>

class ALLEGRO_BITMAP;
class AtlasBitmap
{
	public:
#if __SIZEOF_POINTER__ == 4
		typedef uint32_t DataType;
#elif __SIZEOF_POINTER__ == 8
		typedef uint64_t DataType;
#else
#error "unsupported pointer size!!!!"
#endif
		
		struct Pos { uint32_t x, y; };
		
		AtlasBitmap(uint32_t);
		~AtlasBitmap();
		
		bool get(uint32_t x, uint32_t y);
		void set(uint32_t x, uint32_t y);
		void unset(uint32_t x, uint32_t y);
		
	private:
		uint32_t size_;
		uint32_t data_size_;
		DataType *data_;
		
		uint32_t bitN(uint32_t x, uint32_t y);
};

#endif /* ATLASBITMAP_H_GUARD */