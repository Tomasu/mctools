#include "RendererModel.h"
#include "Model/Model.h"

bool RendererModel::setModel(Model::Model *m)
{
	if(!m)
		return false;
	
	
	
	model_ = m;
	
	return true;
}