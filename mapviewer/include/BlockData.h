#ifndef BLOCKDATA_H_GUARD
#define BLOCKDATA_H_GUARD
#include <stdint.h>

struct CUSTOM_VERTEX;

class BlockData
{
	public:
		const static int32_t NUM_VERTS = 36;
		static BlockData *Create(uint32_t blkid, uint32_t data);
		virtual uint32_t toVerticies(CUSTOM_VERTEX *buff, float xoff, float zoff, float yoff, float tx_xfact, float tx_yfact, float tx_x, float tx_y, float tx_page) = 0;
		
		BlockData();
		virtual ~BlockData();
		
};

class UnknownBlockData : public BlockData
{
	public:
		uint32_t toVerticies(CUSTOM_VERTEX*, float, float, float, float, float, float, float, float);
};

class SolidBlockData : public BlockData
{
	public:
		uint32_t toVerticies(CUSTOM_VERTEX *buff, float xoff, float zoff, float yoff, float tx_xfact, float tx_yfact, float tx_x, float tx_y, float tx_page);
};

#endif /* BLOCKDATA_H_GUARD */
