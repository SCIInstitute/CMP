#ifndef _cmpQwtEllipsePlotItem_h_
#define _cmpQwtEllipsePlotItem_h_

#include "cmp.h"
#include "cmpNumerics.h"

#ifdef _WIN32
#pragma warning(push,3)
#endif

#include <qpainter.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_interval.h>
#include <qwt_painter.h>
#include <qwt_plot_item.h>

#ifdef _WIN32
#pragma warning(pop)
#endif

namespace cmp {

/**
   A subclass of QwtPlotItem for adding rotated ellipses to Qwt plot windows.
   Coordinates are expressed in the coordinates of the plot and translated /
   scaled automatically to the correct coordinates for drawing.
 */
class QwtEllipsePlotItem: public QwtPlotItem
{
public:
 QwtEllipsePlotItem(const PCA &p, qreal rx, qreal ry) : mPCA(p)
  {
    //    mCenter = QPointF(cx,cy);
    mRadX = rx;
    mRadY = ry;
    // mE1 = e1;
    // mE2 = e2;
    //   mRotation = rot;
  }

  /** */
  void setPen(const QPen &pen)
  {
    if ( pen != mPen )
      {
      mPen = pen;
      itemChanged();
      }
  }
  
  /** */
  void setBrush(const QBrush &brush)
  {
    if ( brush != mBrush )
      {
      mBrush = brush;
      itemChanged();
      }
  }
  
  /** Set the center of the ellipse (in plot coordinates). */
  /* void setCenter(const QPointF &p) */
  /* { */
  /*   if ( mCenter != p ) */
  /*     { */
  /* 	  mCenter = p; */
  /* 	  itemChanged(); */
  /*     } */
  /* } */
  
  
  /** Get the ellipse parameter */
  qreal radiusX() const
  { return mRadX;}
  qreal radiusY() const
  { return mRadY;}
  // qreal rotation() const
  //  { return mRotation; }
  // const QPointF &center() const
  //  { return mCenter; }
  
  
  /** Set the CLOCKWISE rotation of the ellipse from the X-AXIS in DEGREES */
  //   void setRotation(const qreal &rot)
  //  {
  //  if (mRotation != rot)
  //     /
  //	mRotation = rot;
  //	itemChanged();
  //   }
  // }

  /** Set the radii of the ellipse on the x and y axes */
  void setRadii(const qreal &x, const qreal &y)
  {
    this->setRadiusX(x);
    this->setRadiusY(y);
  }
  
  /** Set the radius of the ellipse on the x-axis */
  void setRadiusX(const qreal &r)
  {
    if (mRadX != r)
      {
	  mRadX = r;
	  itemChanged();
      }
  }

  /** Set the radius of the ellipse on the y-axis */
  void setRadiusY(const qreal &r)
    {
      if (mRadY != r)
	{
	  mRadY = r;
	  itemChanged();
	}
    }

  /** Return the bounding rectangle of the ellipse (before rotation, in plot
      coordinates). */
  // virtual QRectF boundingRect() const
  //  {
  //   return QRectF();
    //   return QRectF(mCenter.x() - mRadX, mCenter.y() - mRadY, mRadX *2.0, mRadY *2.0);
    //    return QRectF(mCenter + QPointF(-mRadX,-mRadY), mCenter + QPointF(mRadX,mRadY));
  // }
  
  /** Draw the ellipse */
  virtual void draw(QPainter *painter, const QwtScaleMap &xMap, 
                    const QwtScaleMap &yMap, const QRectF &) const;
private:
  PCA mPCA;
  QPen mPen;
  QBrush mBrush;
  qreal mRadX;
  qreal mRadY;
};
} // end namespace

#endif
