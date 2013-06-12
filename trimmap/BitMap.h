#ifndef BITMAP_H_GUARD
#define BITMAP_H_GUARD

#include <stdint.h>
#include <stdlib.h>
#include "NBT_Debug.h"

class BitMap
{
	public:
		BitMap(int64_t top, int64_t left, int64_t bottom, int64_t right);
		~BitMap();
		
		//bool operator[](const uint64_t bit) { return get(bit); }
		bool get(int64_t x, int64_t y);
		void set(int64_t x, int64_t y);
		void unset(int64_t x, int64_t y);
		
		uint64_t getSize() { return size_; }
		
		int64_t top() { return top_; }
		int64_t left() { return left_; }
		int64_t bottom() { return bottom_; }
		int64_t right() { return right_; }
		uint64_t width() { return width_; }
		uint64_t height() { return height_; }
		
	private:
		int64_t top_, left_, bottom_, right_;
		uint64_t width_, height_;
		
		uint64_t size_;
		uint32_t *map_;
		
		uint64_t map_pos(int64_t x, int64_t z);
};

inline BitMap::BitMap(int64_t top, int64_t left, int64_t bottom, int64_t right) : top_(top), left_(left), bottom_(bottom), right_(right)
{
	width_ = abs(left_ - right_) + 1;
	height_ = abs(top_ - bottom_) + 1;
	size_ = width_ * height_;
	
	NBT_Debug("create BitMap of size %lu", size_);
	
	map_ = (uint32_t*)calloc(size_ / sizeof(*map_)+1, sizeof(*map_));
}

inline BitMap::~BitMap()
{
	free(map_);
	map_ = 0;
	size_ = 0;
}

inline bool BitMap::get(int64_t x, int64_t z)
{
	uint64_t bit = map_pos(x, z);
	if(bit >= size_)
		return false;
	
	uint32_t idx = bit / sizeof(*map_);
	uint32_t rem = bit & (sizeof(*map_)*8-1);
	return( (map_[idx] >> ((sizeof(*map_)*8-1)-rem)) & 0x1 );
}

inline void BitMap::set(int64_t x, int64_t z)
{
	uint64_t bit = map_pos(x, z);
	if(bit >= size_)
		return;
	
	uint32_t idx = bit / sizeof(*map_);
	uint32_t rem = bit & (sizeof(*map_)*8-1);
	
	map_[idx] |= 1 << ((sizeof(*map_)*8-1)-rem);
}

inline void BitMap::unset(int64_t x, int64_t z)
{
	uint64_t bit = map_pos(x, z);
	if(bit >= size_)
		return;
	
	uint32_t idx = bit / sizeof(*map_);
	uint32_t rem = bit & (sizeof(*map_)*8-1);
	
	map_[idx] &= ~(1 << ((sizeof(*map_)*8-1)-rem));
}

inline uint64_t BitMap::map_pos(int64_t x, int64_t z)
{
	if(x < left_ || x > right_)
	{
		NBT_Error("x coord %li out of bounds %li/%li", x, left_, right_);
		//abort();
	}	
	
	if(z < top_ || z > bottom_)
	{
		NBT_Error("z coord %li out of bounds %li/%li", z, top_, bottom_);
		//abort();
	}
	
	x += abs(left_);
	z += abs(top_);
	
	return x + z * width_;
}

void horizLine(BitMap *bmp, int x1, int y1, int len);
void circleFill(BitMap *bmp, int x, int y, int r);

#endif /* BITMAP_H_GUARD */
