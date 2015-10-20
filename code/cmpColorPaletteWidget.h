#ifndef _cmpColorPaletteWidget_h_
#define _cmpColorPaletteWidget_h_

#ifdef _WIN32
#pragma warning(push,3)
#endif

#include <QFont>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QWidget>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include <cstdlib>
#include <ctime>

class QMouseEvent;
class QPaintEvent;

namespace cmp {

/** A widget for storing, displaying, and selecting a palette of colors. */
class ColorPaletteWidget : public QWidget
{
  Q_OBJECT
    
 public:
  ColorPaletteWidget(QWidget *parent = 0,int cols=5, int rows = 5);
  QSize sizeHint() const;
  
  /** Directly set the list of colors.  This method bypasses the
      signals/slots and does not emit any colorSelected signals */
  void setColorList(const std::vector<QColor> &cols)
  {    mColorList = cols;  }

  /** Return the number of colors in the palette. */
  unsigned int numberOfColors() const 
  { return mColorList.size(); }

  /** Return the full list of colors. */
  const std::vector<QColor> &colorList() const { return mColorList; }
  
  /** Get the ith color in the list */
  const QColor &color(unsigned int i) const  { return mColorList[i]; }
  QColor &color(unsigned int i)              { return mColorList[i]; }

  public slots:
  void setColor(int idx,const QColor &col);
  
 signals:
  void colorSelected(int i, const QColor &col);
  
 protected:
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *event);
  
 private:
  //  QFont displayFont;
  int mColumns;
  int mRows;
  int mLastKey;
  int mSquareSize;
  QColor mLastColor;
  std::vector<QColor> mColorList;
};

} // end namespace cmp

#endif
