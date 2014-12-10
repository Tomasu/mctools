#ifndef RESOURCMODEL_H_GUARD
#define RESOURCMODEL_H_GUARD
#include <string>
#include "Resource.h"
#include "NBT_Debug.h"

namespace MCModel {
class Model;
}

class ResourceModel : public Resource
{
	public:
		ResourceModel(const std::string &path, MCModel::Model *model = nullptr) : Resource(Resource::ModelType, path), model_(model) { NBT_Debug("new model[%i](%p): %s", id(), model_, path.c_str()); }
		virtual ~ResourceModel();
		
		MCModel::Model *model() { return model_; }
	private:
		MCModel::Model *model_;
};

#endif /* RESOURCMODEL_H_GUARD */
