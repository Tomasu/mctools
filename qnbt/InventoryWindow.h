#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <QMainWindow>

class NBT_Tag_List;
class QTreeView;
class InventoryModel;

class InventoryWindow : public QMainWindow
{
		Q_OBJECT
	public:
		explicit InventoryWindow(NBT_Tag_List *tag, QWidget *parent = 0);
		~InventoryWindow();

	signals:
		
	public slots:

	protected:
		void closeEvent(QCloseEvent *);

	private:
		QTreeView *treeView;
		InventoryModel *model;
};

#endif // INVENTORYWINDOW_H
