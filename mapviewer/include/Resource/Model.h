#ifndef RESOURCMODEL_H_GUARD
#define RESOURCMODEL_H_GUARD
#include <string>
#include "Resource.h"
#include "NBT_Debug.h"

namespace Model {
class Model;
}

class ResourceModel : public Resource
{
	public:
		ResourceModel(const std::string &path, Model::Model *model = nullptr) : Resource(Resource::ModelType, path), model_(model) { NBT_Debug("new model[%i](%p): %s", id(), model_, path.c_str()); }
		virtual ~ResourceModel();
		
		Model::Model *model() { return model_; }
	private:
		Model::Model *model_;
};

#endif /* RESOURCMODEL_H_GUARD */
