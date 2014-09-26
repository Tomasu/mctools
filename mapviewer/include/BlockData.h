#ifndef BLOCKDATA_H_GUARD
#define BLOCKDATA_H_GUARD
#include <stdint.h>

class BlockData
{
	public:
		static BlockData *Create(uint32_t blkid, uint32_t data);
		virtual uint32_t toVerticies(double *buff, double xoff, double zoff, double yoff) = 0;
		
		BlockData();
		virtual ~BlockData();
		
};

class UnknownBlockData : public BlockData
{
	public:
		uint32_t toVerticies(double *buff, double xoff, double zoff, double yoff);
};

class SolidBlockData : public BlockData
{
	public:
		uint32_t toVerticies(double *buff, double xoff, double zoff, double yoff);
};

#endif /* BLOCKDATA_H_GUARD */
