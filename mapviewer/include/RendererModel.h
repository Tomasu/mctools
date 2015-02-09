#ifndef RENDERER_MODEL_H_GUARD
#define RENDERER_MODEL_H_GUARD

namespace MCModel
{
	class Model;
}

class Chunk;

class RendererModel
{
	public:
		RendererModel() { }
		~RendererModel() { }
		
		bool setModel(MCModel::Model *m);
		
		MCModel::Variant *getVariant(RendererChunk *rc, uint32_t x, uint32_t y, uint32_t z);
		
	private:
		MCModel::Model *model_;
};

#endif /* RENDERER_MODEL_H_GUARD */
