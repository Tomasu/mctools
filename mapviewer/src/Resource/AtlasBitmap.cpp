#include <cstring>
#include "Resource/AtlasBitmap.h"

#include "NBT_Debug.h"

AtlasBitmap::AtlasBitmap(uint32_t size) : size_(size), data_size_((size*size) / (sizeof(*data_)))
{
	NBT_Debug("begin");
	NBT_Debug("size:%i nelms:%i data_size:%i sizeof(*data):%i", size, size*size, data_size_, sizeof(*data_));
	data_ = new DataType[data_size_+1];
	memset(data_, 0, data_size_ * sizeof(DataType));
	NBT_Debug("end");
}

AtlasBitmap::~AtlasBitmap()
{
	delete[] data_;
}

inline uint32_t AtlasBitmap::bitN(uint32_t x, uint32_t y)
{
	return (y * size_ + x);
}

bool AtlasBitmap::get(uint32_t x, uint32_t y)
{
	DataType bit = bitN(x, y);
	if(bit >= size_*size_)
	{
		NBT_Debug("get failed, bit:%i out of range.", bit);
		return false;
	}
	uint32_t idx = bit / 8 / sizeof(*data_);
	const uint32_t mask = (sizeof(*data_) * 8 - 1);
	uint32_t rem = bit & mask;
	
	bool ret = (data_[idx] >> (mask - rem)) & 1;
	//NBT_Debug("get: %ix%i bit:%i idx:%i mask:%i rem:%i ret:%i", x, y, bit, idx, mask, rem, ret);
	
	return( ret );
}

void AtlasBitmap::set(uint32_t x, uint32_t y)
{
	DataType bit = bitN(x, y);
	if(bit >= size_*size_)
	{
		NBT_Debug("set failed, bit:%i out of range.", bit);
		return;
	}
	
	uint32_t idx = bit / 8 / sizeof(*data_);
	const uint32_t mask = (sizeof(*data_) * 8 - 1);
	uint32_t rem = bit & mask;
	
	//NBT_Debug("set: %ix%i bit:%i idx:%i mask:%i rem:%i val:%i %x", x, y, bit, idx, mask, rem, data_[idx]);
	data_[idx] |= 1LL << (mask - rem);
	//NBT_Debug("set: %x", data_[idx]);
}

void AtlasBitmap::unset(uint32_t x, uint32_t y)
{
	DataType bit = bitN(x, y);
	if(bit >= size_*size_)
		return;
	
	uint32_t idx = bit / sizeof(*data_);
	const uint32_t mask = (sizeof(*data_) * 8 - 1);
	uint32_t rem = bit & mask;
	
	data_[idx] &= ~(1LL << (mask - rem));
}

