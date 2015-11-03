//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#include "cmp.h"
//#include <QtGui/QApplication>
#include <QApplication>

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  cmp::CMPApplication a(argc, argv);
  cmp::CMP w;

  a.loadStyleSheet(":/CMP/qss/default.qss");
  
  QPixmap pixmap(":/CMP/Resources/splash3.png");
  QSplashScreen splash(pixmap,Qt::WindowStaysOnTopHint);
  splash.show();
  //  sleep(1.0);
  a.processEvents();
  w.show();
  w.raise();
  splash.finish(&w);
  
  // The following sets the paths for dynamic library loading
  // (plugins) at runtime.  The addLibraryPath command does not appear
  // to work in Qt 4.7.  The following will put the package distribution
  // plugins directory (on macs) at the top of the existing default list of paths.
  // Won't hurt anything if there is no plugins directory. --jc
  QStringList ldpath = a.libraryPaths();
  ldpath.push_front(QString(a.applicationDirPath() + "/plugins"));
  a.setLibraryPaths(ldpath);
  // a.addLibraryPath(a.applicationDirPath() + "/plugins");
  //  QStringList::ConstIterator it = a.libraryPaths().constBegin();
  // while (it != a.libraryPaths().constEnd())
  //   {
  //	std::cout << "library path = " << it->toStdString() << std::endl;
  //	it++;
  //   }

  return a.exec();
}
