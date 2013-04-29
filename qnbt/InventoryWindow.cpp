#include <QtGui>
#include "InventoryWindow.h"
#include "InventoryModel.h"

#include "NBT_Debug.h"
#include "NBT_Tag_List.h"

InventoryWindow::InventoryWindow(NBT_Tag_List *tag, QWidget *parent) :
	QMainWindow(parent), treeView(0)
{
	NBT_Debug("");
	QSettings settings;
	restoreGeometry(settings.value("invWindowGeometry").toByteArray());

	// create docks, toolbars, etc...

	restoreState(settings.value("invWindowState").toByteArray());

	setWindowTitle("QNBT Inventory Editor");

	model = new InventoryModel(tag);
}

InventoryWindow::~InventoryWindow()
{
	NBT_Debug("");
}

void InventoryWindow::closeEvent(QCloseEvent *event)
{
	NBT_Debug("");
	Q_UNUSED(event);

	QSettings settings;
	settings.setValue("invWindowGeometry", saveGeometry());
	settings.setValue("invWindowState", saveState());
}
