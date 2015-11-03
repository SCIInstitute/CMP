#ifndef _cmpQwtDataPlot_h
#define _cmpQwtDataPlot_h

#include <iostream>
#include <fstream>


#ifdef _WIN32
#pragma warning(disable : 4251 4244 4800)
#pragma warning(push,3)
#endif

#include "qwt_plot.h"
#include <QPrinter>
#include <qprintdialog.h>
#include <qwt_color_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "cmpImage.h"
#include <cv.h>

#include <QMdiSubWindow>


namespace cmp {

/** A class that adds metadata information to a Qwt plot.  Each subclass knows
    how to write its metadata to a text file with delimeters. */
class QwtDataPlot : public QwtPlot
{
public:
  QwtDataPlot(QWidget *parent = NULL) : QwtPlot(parent)
  {
    plotType = CMP_NULL;
    mDelimeter = ',';
  }
  
  typedef enum {CMP_HISTOGRAM,
                CMP_HISTOGRAM2D,
                CMP_SCATTER,
                CMP_PAWPRINT,
                CMP_NULL} plotTypeID;
  
  /** Print the contents of the metadata to an output stream.*/
  virtual void write(std::ostream &) const = 0;

  /** Write the metadata to a file with the given name.  Returns true on
      success and false on fail. Overwrites anything already in the file.*/
  bool save(const std::string &fn) const;
  
  plotTypeID plotType;
  
  void setDelimeter(char s)
  { mDelimeter = s; }
  
protected:
  char mDelimeter;
  
};

class QwtHistogramDataPlot : public QwtDataPlot
{
public:
  QwtHistogramDataPlot(QWidget *parent = NULL) : QwtDataPlot(parent)
  { plotType = CMP_HISTOGRAM; }
  
  /** Print the contents of the metadata to an output stream.*/
  virtual void write(std::ostream &) const;
  
  /** Add pointers to lists of edges and counts to the metadata. Third
      parameter is the size of the lists (which must be equal).  Note
      that this data is just pointers to memory managed elsewhere and
      does not get deleted when destroying this class. */  
  void addData(const std::string &s, const double *edges,
               const double *counts, unsigned int sz);
  
  /** Clear all metadata pointers. */
  void clearData() 
  {
    mStrings.clear();
    mEdges.clear();
    mCounts.clear();
    mNumBins.clear();
  }
  
private:
  std::vector<std::string>    mStrings;
  std::vector< std::vector<double> > mEdges;
  std::vector< std::vector<double> > mCounts;
  std::vector<unsigned int>   mNumBins;
};


class QwtScatterDataPlot : public QwtDataPlot
{
public:
  QwtScatterDataPlot(QWidget *parent = NULL) : QwtDataPlot(parent)
  { plotType = CMP_SCATTER; }
  
  /** Print the contents of the metadata to an output stream.*/
  virtual void write(std::ostream &) const;
  
  /** Add pointers to the scatter plot data*/
  void addData(const std::string &s1, const std::string &s2,
               const double *xdat, const double *ydat, unsigned int sz);
  
  /** Clear all metadata pointers. */
  void clearData() 
  {
    for (unsigned int i = 0; i < mXdata.size(); i++)
      {
      mXdata[i].clear();
      mYdata[i].clear();
      }
    mNumsamples = 0;
  }
  
private:
  std::vector< std::string> mFilenameX;
  std::vector< std::string> mFilenameY;
  std::vector< std::vector< double > > mXdata;
  std::vector< std::vector< double > > mYdata;
  unsigned int mNumsamples;
};


class QwtPawprintDataPlot : public QwtDataPlot
{
public:
  QwtPawprintDataPlot(QWidget *parent = NULL) : QwtDataPlot(parent)
  { 
    plotType = CMP_PAWPRINT; 
    //    this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed, true);
        this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // std::cout << "Constructing Pawprint widget" << std::endl;
    // setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred, TRUE) );
  }
  
  /** Print the contents of the metadata to an output stream.*/
  virtual void write(std::ostream &) const;

  int heightForWidth( int w ) const
  {
    return w;

    // QSize p = sizeHint();
    // float ar = (float)p.height() / (float)p.width();
    // return static_cast<int>((float)w * ar); 
  }

  virtual void replot() 
  {
    QwtPlot::replot();
    this->setMinimumSize(QwtPlot::sizeHint()* 2.0);
    if (parentWidget() != NULL)  this->parentWidget()->adjustSize();
  }
  
  /** Add pointers to the scatter plot data*/
  void addData(const std::string &s, const std::string &fn,
               const cv::Mat &mE,const cv::Mat &mV,const cv::Mat &mA);
  
  /** Clear all metadata pointers. */
  void clearData() 
  {
    mReferenceFilename = "";
    mFilenames.clear();
    mEigenvectors.clear();
    mEigenvalues.clear();
    mMeans.clear();
  }

private:
  std::string mReferenceFilename;
  std::vector<std::string> mFilenames;
  std::vector< cv::Mat >   mEigenvectors;
  std::vector< cv::Mat >   mEigenvalues;
  std::vector< cv::Mat >   mMeans;
};




/** A Qwt raster data class that indexes into a 2D cvHistogram.  NOTE:
    This class will free the histogram memory when it is deleted. */
class cvHist2DRasterData : public QwtRasterData
{
public:
  cvHist2DRasterData(CvHistogram *h,int,float,float);
  ~cvHist2DRasterData() 
  { 
	// Clean histogram memory on destruction
	cvReleaseHist(&mHistogram); 
  }
  virtual double value(double x, double y) const;
  
  /** Allocates and returns a new color map based on the histogram data */
  QwtLinearColorMap *colorMap() const;
  
  CvHistogram *mHistogram;
  
  float mNumbins;
  float mImgmin;
  float mImgmax;
};

/** Plot picker/zoomer that knows about cvHistogram raster data
    (cvHist2DRasterData) */
class Hist2DZoomer: public QwtPlotZoomer
{
 public:
 Hist2DZoomer(QwtPlotCanvas *canvas): QwtPlotZoomer(canvas)
  {
    mData = NULL;
    setTrackerMode(AlwaysOn);
  }
  
  virtual QwtText trackerTextF(const QPointF &pos) const;
 
 /** Set/Get the histogram raster data */
 void setData(cvHist2DRasterData *d)
 {
   mData = d;
 }
 const cvHist2DRasterData *Data() const
 {return mData; }
 cvHist2DRasterData *Data()
 {return mData; }

 private:
 cvHist2DRasterData *mData;
};
  

/**  A Qwt plot object for 2D histograms.  Uses a CvHistogram to set
     up plot parameters and raster data. */
class QwtHistogram2DDataPlot: public QwtDataPlot
{
  Q_OBJECT
  
public:
  QwtHistogram2DDataPlot(QWidget *parent = NULL) : QwtDataPlot(parent)
  { plotType = CMP_HISTOGRAM2D; }

  void setHistogramData(const std::string &fn1,const std::string &fn2,
                        CvHistogram *hist,int,float,float);
  
  /** */
  const QwtPlotSpectrogram *spectrogram() const
  { return mSpectrogram; }
  QwtPlotSpectrogram *spectrogram()
  { return mSpectrogram; }
  
  /** */
  const cvHist2DRasterData *rasterData() const
  { return mRasterData; }
  cvHist2DRasterData *rasterData()
  { return mRasterData; }
  
    virtual void write(std::ostream &) const;
public Q_SLOTS:
  void showContour(bool on);
  void showSpectrogram(bool on);
  
private:
  QwtPlotSpectrogram *mSpectrogram;
  cvHist2DRasterData *mRasterData;
  std::string mFilename1;
  std::string mFilename2;
};


inline std::ostream& operator<<(std::ostream& s, const QwtHistogramDataPlot &plot)
{ plot.write(s); return s; }
  
  
} // end namespace cmp

#endif
