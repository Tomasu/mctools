#ifndef NBTMODEL_H
#define NBTMODEL_H

#include <QAbstractListModel>

class NBT_Tag_Compound;
class NBTModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		explicit NBTModel(QObject *parent = 0);
		~NBTModel();

		bool open(const QString &fname);

		QVariant data(const QModelIndex &index, int role) const;
		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(int section, Qt::Orientation orientation,
								  int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column,
								const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;

		bool setData(const QModelIndex &index, const QVariant &value, int role);

		NBT_Tag_Compound *getRootTag() { return nbt; }

	signals:
    
	public slots:
    
	private:
		NBT_Tag_Compound *nbt;
};

#endif // NBTMODEL_H
