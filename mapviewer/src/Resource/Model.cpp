#include "Resource/Model.h"
#include "Model/Model.h"

ResourceModel::~ResourceModel()
{
	delete model_;
}
