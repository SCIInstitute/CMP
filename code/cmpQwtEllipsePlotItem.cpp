#include "cmpQwtEllipsePlotItem.h"
#include <iostream>

namespace cmp {

void QwtEllipsePlotItem::draw(QPainter *painter, const QwtScaleMap &xMap,
                              const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
  // Compute angle  -- Note this is redundant code I need to clean up.  It's this way for experimental reasons --JOSH
  QRectF qr(QPointF(0,0),QPointF(mPCA.eigenvectors().at<double>(0,0)*10,
				 mPCA.eigenvectors().at<double>(0,1)*10));
  qr = QwtScaleMap::transform(xMap,yMap, qr);
  qreal x1,x2,y1,y2;
  qr.getCoords(&x1,&y1,&x2,&y2);
  qreal angle = (qreal)atan(fabs(y1 - y2) / (fabs(x2 - x1) + 1.0e-10));
  angle = angle * (180.0 / CMP_PI); // convert radians ato degrees
  
  // Compute ellipse params 
  double p0x,p1x,p2x;
  double p0y,p1y,p2y;
  mPCA.PCAtoWorld(mRadX,0,p1x,p1y);
  mPCA.PCAtoWorld(0,mRadY,p2x,p2y);
  mPCA.PCAtoWorld(0,0,p0x,p0y);

  p0x = xMap.transform(p0x);
  p0y = yMap.transform(p0y);
  p1x = xMap.transform(p1x);
  p1y = yMap.transform(p1y);
  p2x = xMap.transform(p2x);
  p2y = yMap.transform(p2y);

  // QRectF a1(QPointF(p0x-5,p0y-5), QPointF(p0x+5,p0y+5));
  // QRectF b1(QPointF(p1x-5,p1y-5), QPointF(p1x+5,p1y+5));
  // QRectF c1(QPointF(p2x-5,p2y-5), QPointF(p2x+5,p2y+5));

  // QwtPainter::drawEllipse(painter,a1);
  // QwtPainter::drawEllipse(painter,b1);
  // QwtPainter::drawEllipse(painter,c1);
  
  double rx = sqrt((p1x - p0x) * (p1x - p0x) + (p1y - p0y) * (p1y - p0y));
  double ry = sqrt((p2x - p0x) * (p2x - p0x) + (p2y - p0y) * (p2y - p0y));

  QRectF rect(p0x-rx,p0y-ry,2*rx,2*ry);

  painter->translate(QPointF(p0x,p0y));
  painter->rotate(-angle);
  painter->translate(-QPointF(p0x,p0y));

  // Draw ellipse
  painter->setPen(mPen);
  painter->setBrush(mBrush);
  QwtPainter::drawEllipse(painter,rect);
}



} // end namespace
