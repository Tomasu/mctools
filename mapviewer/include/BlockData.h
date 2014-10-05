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
		
		static bool isSolid(uint32_t blockid);
		static bool isTranslucent(uint32_t blockid);
		static bool isSolidForCull(uint32_t blockid);
		
		static uint32_t ID(const uint8_t *data, const uint8_t *add, uint32_t idx)
		{
			uint32_t blkid_a = data[idx];
			uint32_t blkid_b = Nibble4(add, idx);
			uint32_t blkid = blkid_a + (blkid_b << 8);
					
			return blkid;
		}
		
		BlockData();
		virtual ~BlockData();
		
	private:
		static uint8_t Nibble4(const uint8_t *arr, uint32_t index) { if(!arr) return 0; return index % 2 == 0 ? arr[index / 2] & 0x0F : (arr[index / 2] >> 4) & 0x0F; }
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
