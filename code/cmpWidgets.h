//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#ifndef _cmpWidgets_h_
#define _cmpWidgets_h_

#ifdef _WIN32
#pragma warning(disable : 4251 4244 4800)
#pragma warning(push,3)
#endif

#include <QLabel>
#include <QListWidget>
#include <QGraphicsScene>
#include <QGraphicsView>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "cmpUtils.h"

#include <iostream>
class FixedARQMdiSubWindow : public QMdiSubWindow
{
  Q_OBJECT
public:
  FixedARQMdiSubWindow() 
    {
      //    setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred, true) );
    }
  ~FixedARQMdiSubWindow() {}
  
  // int heightForWidth( int w ) const
  //  {
  //   std::cout << "HERE" << std::endl;
  //   float ar = (float)widget()->height() / (float)widget()->width();    
  //   return static_cast<int>((float)w * ar); 
  // }

  //  QSize sizeHint() const 
  // { 
  //   return QSize( 50, heightForWidth(50) );
  // }
  

  //   void resizeEvent(QResizeEvent *e) 
  //  {
  //    int w = e->oldSize().width();
  //    int h = e->oldSize().height();
    
  //     float ar = (float)h / (float)w; 
    
  //     widget()->resize(this->width(), (int)((float)this->width() * ar));
     //   this->adjustSize();
  //   }
  
  
};

class QImageList : public QListWidget
{
  Q_OBJECT
public:
  QImageList(QWidget *parent = NULL);
  ~QImageList();

protected:
  QStringList mimeTypes() const;
  Qt::DropActions supportedDropActions();
  bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
  
signals:
  void imageDropped(QString);
};

class QPreviewScene : public QGraphicsScene
{
  Q_OBJECT
public:
  QPreviewScene(QWidget *parent = 0);
  ~QPreviewScene() {}

  //  void setBackground(const QImage &bg);
  void setQImageToDisplay(const QImage &bg);
  const QGraphicsPixmapItem *getBackground() const
  { return mBackground;}
  QGraphicsPixmapItem *getBackground()
  { return mBackground;}
  

protected:
  void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
  void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
  void dropEvent(QGraphicsSceneDragDropEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *e);

signals:
  void imageDropped(QString);
  void imagePicked(int,int);

private:
  QGraphicsPixmapItem *mBackground;
  QGraphicsSimpleTextItem *mText;

};

class QPreviewWidget : public QGraphicsView
{
  Q_OBJECT
public:
  QPreviewWidget(QWidget *parent = 0);
  ~QPreviewWidget() {}
  
  void resizeEvent(QResizeEvent *event);
  
  //  void setBackground(const QImage &bg);
  void setQImageToDisplay(const QImage &bg);
  QPreviewScene *getScene()  { return mScene; }
  const QPreviewScene * getScene() const { return mScene;}
  
signals:
  void resized(int w, int h);
  
protected:
  QPreviewScene *mScene;
};

#endif
