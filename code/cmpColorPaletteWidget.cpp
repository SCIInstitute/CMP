#include <QtWidgets>

#include "cmpColorPaletteWidget.h"

namespace cmp {

ColorPaletteWidget::ColorPaletteWidget(QWidget *parent,int cols, int rows)
  : QWidget(parent)
{
  srand(time(NULL));

  mSquareSize = 25;
  mColumns = cols;
  mRows = rows;
  mLastKey = -1;
  setMouseTracking(true);
  mLastColor = QColor("red");
  mColorList.resize(cols*rows);

  // Fill color list with random colors
  for (unsigned int i = 0; i < mColorList.size(); i++)
    {
      mColorList[i] = QColor(rand()%255,rand()%255,rand()%255);
    }

  // First colors are some Default colors
  if (mColorList.size() > 8)
    {
      mColorList[0] = QColor("red");
      mColorList[1] = QColor("green");  
      mColorList[2] = QColor("blue");
      mColorList[3] = QColor("brown");
      mColorList[4] = QColor("cyan");
      mColorList[5] = QColor("yellow");
      mColorList[6] = QColor("magenta");
      mColorList[7] = QColor("orange");
      mColorList[8] = QColor("purple");
    }
}

void ColorPaletteWidget::setColor(int idx,const QColor &col)
{
  if (idx < (int)mColorList.size())
    {
      mColorList[idx] = col;
      this->update();
    }
  else // out of bounds
    return;
}

QSize ColorPaletteWidget::sizeHint() const
{
  return QSize(mColumns*mSquareSize + 4, mRows*mSquareSize + 4);
}
  
void ColorPaletteWidget::mouseMoveEvent(QMouseEvent *event)
{
  QPoint widgetPosition = mapFromGlobal(event->globalPos());
  uint key = (widgetPosition.y()/mSquareSize)*mColumns + widgetPosition.x()/mSquareSize;
 
  QString text = mColorList[key].name();
 
  QToolTip::showText(event->globalPos(), text, this);
}

void ColorPaletteWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) 
    {
      mLastKey = (event->y()/mSquareSize)*mColumns + (event->x()/mSquareSize);

      QColor color;
      // if (native->isChecked())
      color = QColorDialog::getColor(Qt::green, this);
	//  else
	//         color = QColorDialog::getColor(Qt::green, this, "Select Color", QColorDialog::DontUseNativeDialog);

     if (color.isValid()) 
       {
	 this->setColor(mLastKey,color);
	 emit colorSelected(mLastKey,color);
       }

      update();
    }
  else
    {
    QWidget::mousePressEvent(event);
    }
}
  
void ColorPaletteWidget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.fillRect(event->rect(), QBrush(Qt::white));
  
  QRect redrawRect = event->rect();
  int beginRow = redrawRect.top()/mSquareSize;
  int endRow = redrawRect.bottom()/mSquareSize;
  int beginColumn = redrawRect.left()/mSquareSize;
  int endColumn = redrawRect.right()/mSquareSize;
  
  painter.setPen(QPen(Qt::gray));
  for (int row = beginRow; row <= endRow; ++row) {
    for (int column = beginColumn; column <= endColumn; ++column) {
      painter.drawRect(column*mSquareSize, row*mSquareSize, mSquareSize, mSquareSize);
    }
  }
  
  //  QFontMetrics fontMetrics(displayFont);
  painter.setPen(QPen(Qt::black));
  for (int row = beginRow; row <= endRow; ++row) {
    
    for (int column = beginColumn; column <= endColumn; ++column) {
      
      int key = row*mColumns + column;
      painter.setClipRect(column*mSquareSize, row*mSquareSize, mSquareSize, mSquareSize);
      
      
      //  if (key == mLastKey)
      // 	painter.fillRect(column*mSquareSize + 1, row*mSquareSize + 1, mSquareSize, mSquareSize, QBrush(mLastColor));
      painter.fillRect(column*mSquareSize + 1, row*mSquareSize + 1, mSquareSize, mSquareSize, QBrush(mColorList[key]));
      
      //    painter.drawText(column*mSquareSize + (mSquareSize / 2) - fontMetrics.width(QChar(key))/2,
      //		       row*mSquareSize + 4 + fontMetrics.ascent(),
      //		       QString(QChar(key)));
    }
  }
}


} //end namespace
  
