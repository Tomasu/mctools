#ifndef NBTITEMDELEGATE_H
#define NBTITEMDELEGATE_H

#include <cstdint>
#include <QItemDelegate>
class QSpinBox;

class NBTItemDelegate : public QItemDelegate
{
		Q_OBJECT
	public:
		explicit NBTItemDelegate(QObject *parent = 0);
		
		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
									 const QModelIndex &index) const;

		void setEditorData(QWidget *editor, const QModelIndex &index) const;
		void setModelData(QWidget *editor, QAbstractItemModel *model,
								const QModelIndex &index) const;

		void updateEditorGeometry(QWidget *editor,
										  const QStyleOptionViewItem &option, const QModelIndex &index) const;

	signals:
		
	public slots:

	private:
		QWidget *createNumberEditor(uint32_t bits, QWidget *parent) const;
		void updateNumberEditor(uint32_t bits, QWidget *widget, uint64_t value) const;
		
};

#endif // NBTITEMDELEGATE_H
