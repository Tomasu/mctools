#include "Resource/Model.h"
#include "MCModel/Model.h"

ResourceModel::~ResourceModel()
{
	delete model_;
}
