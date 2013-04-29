#ifndef MAINWINDOW_H_GUARD
#define MAINWINDOW_H_GUARD

#include <QMainWindow>

class QSplitter;
class QTreeView;
class QListView;
class InventoryWindow;
class NBTModel;

class MainWindow : public QMainWindow
{
	Q_OBJECT
	public:
		MainWindow(QWidget *parent = 0, Qt::WFlags f = 0);
		~MainWindow();
		

    public slots:
        void fileOpen();
        void fileExit();
		  void editInventory();

	protected:
		  void closeEvent(QCloseEvent *);

	private:
        QTreeView *treeView;
		  NBTModel *nbt_model;
		  InventoryWindow *invWindow;

        void setupMenuBar();

	private slots:
		  void treeViewContextMenuRequested(const QPoint &p);

};

#endif /* MAINWINDOW_H_GUARD */
