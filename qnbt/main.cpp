#include <QtCore>
#include <QtGui>

#include "MainWindow.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	MainWindow *window = new MainWindow();
	window->show();
	
	int ret = app.exec();
	delete window;
	
	return ret;
}
