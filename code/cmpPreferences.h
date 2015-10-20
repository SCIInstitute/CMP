#ifndef _cmpPreferences_h_
#define _cmpPreferences_h_

#ifdef _WIN32
#pragma warning(push,3)
#endif

#include <QSettings>
#include <QColor>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include <iostream>

namespace cmp
{

/** This class holds the various preferences required for the CMP Cellkit app*/
class Preferences : public QSettings
{
Q_OBJECT
  public:
  Preferences(const QString &org, const QString &app, QObject *parent=0);    
  ~Preferences() {}

  /** Gets one of the stored colors.  This method will return a null
      color if mNumberOfColors is exceeded. */
  QColor color(int i) const;
  QColor clusterColor(int i) const;
  
  /** The number of x-y plotting pairs that will be allowed in the
      scatter plot and pawprint dialogs. */
  int numberOfPlotPairs() const 
  {  
    return this->value("plots/number_of_plot_pairs").toInt();
  }

  /** The number of bins for histogramming. */
  int numberOfBins() const
  {
    return this->value("plots/quick_histogram/number_of_bins").toInt();
  }

  /** The number of samples for scatterplots. */
  int numberOfSamples() const
  {
    return this->value("plots/quick_scatterplot/number_of_samples").toInt();
  }

  /** The max number of colors available in this set of preferences.
      These are the color preferences that may be chosen for coloring
      masks or other elements. */
  int numberOfColors() const { return mNumberOfColors; }

  /** Get the background color variable frmo the preference file */
  QColor backgroundColor() const
  { return this->value("main/background_color").value<QColor>(); }

 /** Get the background color variable from the preference file */
  QColor clusterBackgroundColor() const
  { return this->value("main/cluster_background_color").value<QColor>(); }

 public slots:
  //  void setIntValue(const char *key, int v)  { this->setValue(key,v); } 
  
  /** Save the background color variable in preference file */
  void setBackgroundColor(const QColor &col)
  { this->setValue("main/background_color",col);  }
  
 /** Save the background color variable in preference file */
  void setClusterBackgroundColor(const QColor &col)
  { this->setValue("main/cluster_background_color",col);  }

  /** Save parameter in preference file. */
  void setNumberOfBins(int v)  { this->setValue("plots/quick_histogram/number_of_bins",v); }
  void setNumberOfSamples(int v)  { this->setValue("plots/quick_scatterplot/number_of_samples",v); }

  /** Set number of plotting pairs. */
  void setNumberOfPlotPairs(int v) { this->setValue("plots/number_of_plot_pairs",v); }

  /** Assign a color to one of the favorite color slots */
  void setColor(int i, const QColor &col);

  /** Assign a color to one of the cluster color slots */
  void setClusterColor(int i, const QColor &col);
  
 protected:
  int mNumberOfColors;
  int mNumberOfPlotPairs;
};
 

} // end namespace cmp

#endif
