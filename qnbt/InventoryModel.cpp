#include "InventoryModel.h"

#include "NBT_Tag_List.h"
#include "NBT_Tag_Compound.h"

InventoryModel::InventoryModel(NBT_Tag_List *tag, QObject *parent) :
	QAbstractListModel(parent), nbt_tag(tag)
{

}

QVariant InventoryModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	NBT_Tag_Compound *slot = nbt_tag->itemAt(index.row());
	if(index.column() == 0)
	{
		return QVariant(slot)
	}
}

Qt::ItemFlags InventoryModel::flags(const QModelIndex &index) const
{

}

QVariant InventoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{

}

int InventoryModel::rowCount(const QModelIndex &parent) const
{

}

int InventoryModel::columnCount(const QModelIndex &parent) const
{

}

bool InventoryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

}
