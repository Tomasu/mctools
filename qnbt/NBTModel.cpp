#include "NBTModel.h"
#include "NBT_File.h"
#include "NBT_Debug.h"
#include "NBT_Tag_Byte.h"
#include "NBT_Tag_Byte_Array.h"
#include "NBT_Tag_Compound.h"
#include "NBT_Tag_Double.h"
#include "NBT_Tag_Float.h"
#include "NBT_Tag_Int.h"
#include "NBT_Tag_Int_Array.h"
#include "NBT_Tag_List.h"
#include "NBT_Tag_Long.h"
#include "NBT_Tag_Short.h"
#include "NBT_Tag_String.h"

#include <vector>

NBTModel::NBTModel(QObject *parent) :
	QAbstractItemModel(parent), nbt(0)
{
}

NBTModel::~NBTModel()
{
	delete nbt;
}

bool NBTModel::open(const QString &fname)
{
	NBT_File *fh = new NBT_File(fname.toStdString());
	if(!fh->open())
	{
		NBT_Error("failed to open nbt file :(");
		return false;
	}

	if(!fh->seek(0, SEEK_END))
	{
		NBT_Error("failed to seek to end of file");
		return false;
	}

	uint32_t length = fh->tell();

	if(!fh->seek(0, SEEK_SET))
	{
		NBT_Error("failed to seek to beginning of file");
		return false;
	}

	if(!fh->readCompressedMode(length, false))
	{
		NBT_Error("failed to enter compressed mode for %i bytes :(", length);
		return false;
	}

	NBT_Debug("file len: %i", length);
	NBT_Tag *new_nbt = NBT_Tag::LoadTag(fh);
	if(!new_nbt)
	{
		NBT_Error("failed to read nbt data");
		delete fh;
		return false;
	}

	fh->endCompressedMode();

	if(nbt)
		delete nbt;

	beginResetModel();
	nbt = (NBT_Tag_Compound *)new_nbt;
	endResetModel();

	return true;
}

QVariant NBTModel::data(const QModelIndex &index, int role) const
{
	//NBT_Debug("begin");

	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	NBT_Tag *tag = static_cast<NBT_Tag *>(index.internalPointer());
	if(index.column() == 0)
	{
		if(tag->name().length() < 1 && tag->parent() && tag->parent()->type() == TAG_List)
		{
			return QVariant(tag->row());
		}

		return QVariant(tag->name().c_str());
	}
	else if(index.column() == 1)
	{
		return QVariant(NBT_Tag::tagNames[tag->type()]);
	}

	//NBT_Debug("end");

	switch(tag->type())
	{
		case TAG_End:
			return QVariant("END");

		case TAG_Byte:
			return QVariant(static_cast<NBT_Tag_Byte *>(tag)->value());

		case TAG_Short:
			return QVariant(static_cast<NBT_Tag_Short *>(tag)->value());

		case TAG_Int:
			return QVariant(static_cast<NBT_Tag_Int *>(tag)->value());

		case TAG_Long:
			return QVariant(static_cast<NBT_Tag_Int *>(tag)->value());

		case TAG_Float:
			return QVariant(static_cast<NBT_Tag_Float *>(tag)->value());

		case TAG_Double:
			return QVariant(static_cast<NBT_Tag_Double *>(tag)->value());

		case TAG_Byte_Array:
			return QVariant("");

		case TAG_String:
			return QVariant(static_cast<NBT_Tag_String *>(tag)->value().c_str());

		case TAG_List:
			return QVariant(static_cast<NBT_Tag_List *>(tag)->count());

		case TAG_Compound:
			return QVariant(static_cast<NBT_Tag_Compound *>(tag)->count()-1);

		case TAG_Int_Array:
			return QVariant("");

		default:
			return QVariant("unknown");
	}

	return QVariant("wat");
}

Qt::ItemFlags NBTModel::flags(const QModelIndex &index) const
{
	//NBT_Debug("");

	if (!index.isValid())
		return 0;

	NBT_Tag *tag = static_cast<NBT_Tag *>(index.internalPointer());
	if(tag->type() == TAG_List || tag->type() == TAG_Compound)
		return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	if(index.column() == 2)
		return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant NBTModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
			case 0:
				return QVariant("Name");

			case 1:
				return QVariant("Type");

			case 2:
				return QVariant("Value");
		}
	}

	return QVariant();
}

QModelIndex NBTModel::index(int row, int column, const QModelIndex &parent) const
{
	//NBT_Debug("");

	//if(!hasIndex(row, column, parent))
	//	return QModelIndex();

	NBT_Tag *parentTag = 0;

	if (!parent.isValid())
		parentTag = nbt;
	else
		parentTag = static_cast<NBT_Tag *>(parent.internalPointer());

	NBT_Tag *childTag = 0;
	if(parentTag->type() == TAG_Compound)
	{
		childTag = static_cast<NBT_Tag_Compound *>(parentTag)->childAt(row);
	}
	else if(parentTag->type() == TAG_List)
	{
		childTag = static_cast<NBT_Tag_List *>(parentTag)->items().at(row);
	}

	if (childTag)
		return createIndex(row, column, childTag);
	else
		return QModelIndex();
}

QModelIndex NBTModel::parent(const QModelIndex &index) const
{
	//NBT_Debug("");

	if (!index.isValid())
		return QModelIndex();

	NBT_Tag *childTag = static_cast<NBT_Tag *>(index.internalPointer());
	NBT_Tag *parentTag = childTag->parent();

	if (!parentTag)
		return QModelIndex();

	return createIndex(parentTag->row(), 0, parentTag);
}

int NBTModel::rowCount(const QModelIndex &parent) const
{
	//NBT_Debug("");

	NBT_Tag *tag = 0;
	if(!parent.isValid())
		tag = nbt;
	else
		tag = static_cast<NBT_Tag *>(parent.internalPointer());

	if(tag->type() == TAG_Compound)
	{
		NBT_Tag_Compound *cpd = static_cast<NBT_Tag_Compound *>(tag);
		return cpd->count()-1;
	}
	else if(tag->type() == TAG_List)
		return static_cast<NBT_Tag_List *>(tag)->count();

	return 0;
}

int NBTModel::columnCount(const QModelIndex &parent) const
{
	return 3;

	//NBT_Debug("");

	NBT_Tag *tag = 0;
	if(!parent.isValid())
		return 3;
	else
		tag = static_cast<NBT_Tag *>(parent.internalPointer());

	if(tag->type() == TAG_Compound || tag->type() == TAG_List)
		return 1;

	return 2;
}

template<class C, typename T>
static void setValue(C *tag, T value)
{
	tag->setValue(value);
}

bool NBTModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
		return false;

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return false;

	if(index.column() != 2)
		return false;

	NBT_Tag *tag = static_cast<NBT_Tag *>(index.internalPointer());

	switch(tag->type())
	{
		case TAG_Byte:
			setValue(static_cast<NBT_Tag_Byte *>(tag), value.toInt() & 0xff);
			break;

		case TAG_Short:
			setValue(static_cast<NBT_Tag_Short *>(tag), value.toInt() & 0xffff);
			break;

		case TAG_Int:
			setValue(static_cast<NBT_Tag_Int *>(tag), value.toInt());
			break;

		case TAG_Long:
			setValue(static_cast<NBT_Tag_Long *>(tag), (int64_t)value.toLongLong());
			break;

		case TAG_Float:
			setValue(static_cast<NBT_Tag_Float *>(tag), value.toFloat());
			break;

		case TAG_Double:
			setValue(static_cast<NBT_Tag_Double *>(tag), value.toDouble());
			break;

		case TAG_String:
			setValue(static_cast<NBT_Tag_String *>(tag), value.toString().toStdString());
			break;

		default:
			return false;
	}

	return true;
}
