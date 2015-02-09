#include "RendererModel.h"
#include "MCModel/Model.h"

bool RendererModel::setModel(MCModel::Model *m)
{
	if(!m)
		return false;
	
	
	
	model_ = m;
	
	return true;
}