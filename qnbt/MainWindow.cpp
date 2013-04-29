#include "MainWindow.h"

#include <QtGui>

#include "NBT_Debug.h"
#include "NBT_Tag.h"
#include "NBT_Tag_Compound.h"
#include "NBTModel.h"
#include "NBTItemDelegate.h"

#include "InventoryWindow.h"

MainWindow::MainWindow(QWidget *parent, Qt::WFlags f) : QMainWindow(parent, f)
{
	nbt_model = 0;
	invWindow = 0;

	QCoreApplication::setOrganizationDomain("tomasu.org");
	QCoreApplication::setOrganizationName("Tomasu");
	QCoreApplication::setApplicationName("QNNBT");
	QCoreApplication::setApplicationVersion("0.0.1");

	QSettings settings;
	restoreGeometry(settings.value("mainWindowGeometry").toByteArray());

	// create docks, toolbars, etc...

	restoreState(settings.value("mainWindowState").toByteArray());

	treeView = new QTreeView();
	treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(treeViewContextMenuRequested(QPoint)));

	setCentralWidget(treeView);

	setWindowTitle("QNBT Editor");
	setupMenuBar();
}

MainWindow::~MainWindow()
{
	delete nbt_model;
	delete invWindow;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);

	QSettings settings;
	settings.setValue("mainWindowGeometry", saveGeometry());
	settings.setValue("mainWindowState", saveState());
}

void MainWindow::setupMenuBar()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	QAction *fileOpenAction = fileMenu->addAction(tr("&Open"));
	connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));

	fileMenu->addSeparator();

	QAction *fileExitAction = fileMenu->addAction(tr("E&xit"));
	connect(fileExitAction, SIGNAL(triggered()), this, SLOT(fileExit()));
}

void MainWindow::treeViewContextMenuRequested(const QPoint &p)
{
	NBT_Debug("begin");
	QModelIndex idx = treeView->indexAt(p);
	NBT_Tag *tag = static_cast<NBT_Tag *>(idx.internalPointer());
	if(!tag)
	{
		NBT_Debug("no tag at possition?");
		return;
	}

	NBT_Debug("%s %s", NBT_Tag::tagNames[tag->type()], tag->name().c_str());
	if(tag->type() == TAG_List)
	{
		NBT_Debug("Got List tag!");
		NBT_Tag_Compound *ctag = static_cast<NBT_Tag_Compound *>(tag);
		if(ctag->name() == "Inventory")
		{
			NBT_Debug("got inventory!");
			QMenu menu(this);
			menu.addAction("Edit &Inventory", this, SLOT(editInventory()));
			menu.exec(QCursor::pos());
		}
	}

	NBT_Debug("end");
}

void MainWindow::fileExit()
{
	qApp->exit();
}

void MainWindow::fileOpen()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open NBT File");
	printf("open %s\n", qPrintable(fileName));

	nbt_model = new NBTModel();

	if(!nbt_model->open(fileName))
	{
		NBT_Error("failed :(");
		return;
	}

	treeView->setModel(nbt_model);
	treeView->resizeColumnToContents(0);
	treeView->resizeColumnToContents(1);
	treeView->setRootIsDecorated(true);

	NBTItemDelegate *delegate = new NBTItemDelegate(treeView);
	treeView->setItemDelegate(delegate);
}

void MainWindow::editInventory()
{
	NBT_Debug("EDIT INV!");
	if(invWindow)
		return;

	//invWindow = new InventoryWindow((NBT_Tag*)nbt_model->getRootTag(), this);
	//invWindow->show();
}

