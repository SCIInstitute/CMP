#include "cmpPreferences.h"


namespace cmp
{

Preferences::Preferences(const QString &org, const QString &app, QObject *parent) : mNumberOfColors(20), mNumberOfPlotPairs(5)
{
  QSettings(org,app,parent);
}
  
QColor Preferences::color(int i) const
{
  if (i > mNumberOfColors) return QColor();
  QString str = QString("colors/favorite%1").arg(i);
  //  std::cout << "Getting color " << str.toAscii().data() << std::endl;
  return this->value(str).value<QColor>();
}
  
void Preferences::setColor(int i, const QColor &col) 
{
  if (i > mNumberOfColors) return;
  QString str = QString("colors/favorite%1").arg(i);
  this->setValue(str,col);
}
  
QColor Preferences::clusterColor(int i) const
{
  if (i > mNumberOfColors) return QColor();
  QString str = QString("colors/cluster%1").arg(i);
  return this->value(str).value<QColor>();
}

void Preferences::setClusterColor(int i, const QColor &col) 
{
  if (i > mNumberOfColors) return;
  QString str = QString("colors/cluster%1").arg(i);
  this->setValue(str,col);
}

} // end namespace cmp
