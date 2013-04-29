#ifndef INVENTORYMODEL_H
#define INVENTORYMODEL_H

#include <QAbstractListModel>

class NBT_Tag_List;
class InventoryModel : public QAbstractListModel
{
		Q_OBJECT
	public:
		explicit InventoryModel(NBT_Tag_List *tag, QObject *parent = 0);
		
		QVariant data(const QModelIndex &index, int role) const;
		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(int section, Qt::Orientation orientation,
								  int role = Qt::DisplayRole) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;

		bool setData(const QModelIndex &index, const QVariant &value, int role);

	signals:
		
	public slots:

	private:
		NBT_Tag_List *nbt_tag;
		
};

#endif // INVENTORYMODEL_H
