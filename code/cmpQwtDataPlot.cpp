#include "cmpQwtDataPlot.h"

namespace cmp {

bool QwtDataPlot::save(const std::string &fn) const
{
  // Open output text file for writing.  Erase anything already in the file.
  std::ofstream o(fn.c_str(),std::ios::out|std::ios::trunc);
  if (! o.is_open())
    {
    // Couldn't open file.
    return false;
    }
  
  this->write(o);
  o.close();
  return true;
}

void QwtHistogramDataPlot::addData(const std::string &s, const double *edges, 
				   const double *counts, unsigned int sz)
{
  // Make a deep copy of the data.
  mStrings.push_back(s);
  std::vector<double> etmp(sz);
  std::vector<double> ctmp(sz);    
  for (unsigned int i = 0; i < sz; i++) etmp[i] = edges[i];
  for (unsigned int i = 0; i < sz; i++) ctmp[i] = counts[i];    
  mEdges.push_back(etmp);
  mCounts.push_back(ctmp);
  mNumBins.push_back(sz);
}

void QwtHistogramDataPlot::write(std::ostream &s) const
{
  // Loop over the list of histogram edges and counts  
  for (unsigned int k = 0; k < mEdges.size(); k++)
    {
    // Print the bin edges
    s << mStrings[k] + std::string(" edges") << mDelimeter;
    for (unsigned int i = 0; i < mNumBins[k]; i++)
      {
      s << mEdges[k][i] << mDelimeter;
      }
    // Print the counts
    s << std::endl <<  mStrings[k] + std::string(" counts") << mDelimeter;
    for (unsigned int i = 0; i < mNumBins[k]; i++)
      {
      s << mCounts[k][i] << mDelimeter;
      }
    s << std::endl;
    }
}

void QwtScatterDataPlot::addData(const std::string &s1, const std::string&s2,
                                 const double *xData, const double *yData,
                                 unsigned int sz)
{
  // Make a deep copy of the data.
  mFilenameX.push_back(s1);
  mFilenameY.push_back(s2);
  
  std::vector<double> tmpx;
  tmpx.resize(sz);
  std::vector<double> tmpy;
  tmpy.resize(sz);
  
  for (unsigned int i = 0; i < sz; i++)
    {
    tmpx[i] = xData[i];
    tmpy[i] = yData[i];
    }
  mXdata.push_back(tmpx);
  mYdata.push_back(tmpy);
  
  mNumsamples = sz;
}

void QwtScatterDataPlot::write(std::ostream &s) const
{
  // Loop through the x and y data lists
  // Print the bin edges
  for (unsigned int k = 0; k < mFilenameX.size(); k++)
    {
    s << mFilenameX[k] << " (x-axis)" << mDelimeter;
    for (unsigned int i = 0; i < mXdata[k].size(); i++)
      {
      s << mXdata[k][i] << mDelimeter;
      }
    s << std::endl;
    
    s << mFilenameY[k] << " (y-axis)" << mDelimeter;
    for (unsigned int i = 0; i < mYdata[k].size(); i++)
      {
      s << mYdata[k][i] << mDelimeter;
      }
    s << std::endl;    
    }
}

void QwtPawprintDataPlot::addData(const std::string &s,
                                  const std::string &fn,
                                  const cv::Mat &mE,
                                  const cv::Mat &mV,
                                  const cv::Mat &mA)
{
  mReferenceFilename = s;
  mFilenames.push_back(fn);
  mEigenvectors.push_back(mE);
  mEigenvalues.push_back(mV);
  mMeans.push_back(mA);
}

void QwtPawprintDataPlot::write(std::ostream &s) const
{
  // Write the file names
  s << "x-axis file" << mDelimeter << mReferenceFilename << std::endl;
  s << "y-axis files" << mDelimeter;
  for (unsigned int i= 0; i < mFilenames.size(); i++)  s << mFilenames[i] << mDelimeter;
  s << std::endl;

  // Write the data
  for (unsigned int i = 0; i < mFilenames.size(); i++)
    {
    s << mFilenames[i] << std::endl;
    s << "Mean sample"  << mDelimeter << mMeans[i].at<double>(0,0) << mDelimeter << mMeans[i].at<double>(0,1) << std::endl;
    s << "Eigenvalues"  << mDelimeter << mEigenvalues[i].at<double>(0,0) << mDelimeter << mEigenvalues[i].at<double>(0,1) << std::endl;
    s << "Eigenvectors" << mDelimeter << mEigenvectors[i].at<double>(0,0) << mDelimeter << mEigenvalues[i].at<double>(0,1) << std::endl;
    s << mDelimeter << mEigenvectors[i].at<double>(1,0) << mDelimeter << mEigenvalues[i].at<double>(1,1) << std::endl;   
    }
}

QwtLinearColorMap *cvHist2DRasterData::colorMap() const
{
  float dc = (mImgmax - mImgmin) / 4.0;
  QwtLinearColorMap *colormap = new QwtLinearColorMap(Qt::darkCyan, Qt::red);
  colormap->addColorStop(mImgmin + dc, Qt::cyan);
  colormap->addColorStop(mImgmin + (dc*2), Qt::green);
  colormap->addColorStop(mImgmin + (dc*3), Qt::yellow); 

  return colormap;
}
  
cvHist2DRasterData::cvHist2DRasterData(CvHistogram *hist, int numbins, 
					 float img_min, float img_max)
{
  mHistogram = hist;

  float min_value, max_value;
  cvGetMinMaxHistValue( hist, &min_value, &max_value);

  mNumbins = (float)numbins;
  mImgmin = img_min;
  mImgmax = img_max;

  this->setInterval( Qt::XAxis, QwtInterval( img_min, img_max ) );
  this->setInterval( Qt::YAxis, QwtInterval( img_min, img_max ) );
  this->setInterval( Qt::ZAxis, QwtInterval( min_value, max_value ) ); 
}

double cvHist2DRasterData::value(double x, double y) const
{
  // Map coordinates into 2D bin numbers
  x = (x - mImgmin)  * (mNumbins / (mImgmax-mImgmin) );
  y = (y - mImgmin)  * (mNumbins / (mImgmax-mImgmin) );

  int idx0 = static_cast<int>(floor(x+0.5));
  int idx1 = static_cast<int>(floor(y+0.5));
  
  if (idx0 <0 || idx0 >= mNumbins || idx1 < 0 || idx1 >=mNumbins )
    { return 0.0; }
  else
    { return static_cast<double>(cvQueryHistValue_2D(mHistogram, idx0, idx1));  }
}
  

QwtText Hist2DZoomer::trackerTextF(const QPointF &pos) const
{
  if (mData == NULL) return QwtText();
  QColor bg(Qt::white);
  bg.setAlpha(200);
  
  // QwtText text = QwtPlotZoomer::trackerTextF(pos);
  QwtText text = QString("%1,%2: %3").arg(pos.x()).arg(pos.y()).arg(mData->value(pos.x(),pos.y()));
  text.setBackgroundBrush( QBrush( bg ));
  return text;
}

void QwtHistogram2DDataPlot::write(std::ostream &s) const
{  
  // Write the filenames
  s << "Files: " << mDelimeter << mFilename1 << mDelimeter << mFilename2 << std::endl;
  
  unsigned int numbins  = (unsigned int)this->rasterData()->mNumbins;
  float numbinsf = this->rasterData()->mNumbins;
  float imgmin   = this->rasterData()->mImgmin;
  float imgmax   = this->rasterData()->mImgmax;
  
  
  // Write the edges
  s << "Edges (x and y axes): " << mDelimeter;
  float x  = imgmin;
  float dx = (imgmax - imgmin) / numbinsf;
  for (unsigned int i = 0; i < numbins; i++, x+=dx)
    {
    s << x << mDelimeter;
    }
  s << std::endl;
      
  // Write the speadsheet of values
  s << "Histogram counts" << std::endl;
       
  x = imgmin;
  float y = imgmin;
  for (unsigned int iy = 0; iy < numbins; iy++,y+=dx)
    {
    for (unsigned int ix = 0; ix < numbins; ix++, x+=dx)
      {
      s <<  cvQueryHistValue_2D(this->rasterData()->mHistogram, ix,iy) << mDelimeter;
      }
    s << std::endl;
    }   
}

void QwtHistogram2DDataPlot::setHistogramData(const std::string &fn1,
                                              const std::string &fn2,
                                              CvHistogram *hist,
                                              int numbins, float img_min,
                                              float img_max)
{
  mFilename1 = fn1;
  mFilename2 = fn2;
  
  mSpectrogram = new QwtPlotSpectrogram();
  mSpectrogram->setRenderThreadCount(0); // use system specific thread count
 
  float min_value, max_value;
  cvGetMinMaxHistValue( hist, &min_value, &max_value);
  
  // Setup raster data
  mRasterData = new cvHist2DRasterData(hist,numbins,img_min,img_max);
  mSpectrogram->setData(mRasterData);
  mSpectrogram->attach(this);

  // Setup color map
  mSpectrogram->setColorMap(mRasterData->colorMap());

  QList<double> contourLevels;
  for ( double level = 0.5; level < 10.0; level += 1.0 )
    contourLevels += level;
  mSpectrogram->setContourLevels(contourLevels);
  
  const QwtInterval zInterval = mSpectrogram->data()->interval( Qt::ZAxis );
  // A color bar on the right axis
  QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
  rightAxis->setTitle("Histogram Count");
  rightAxis->setColorBarEnabled(true);
  rightAxis->setColorMap( zInterval, mRasterData->colorMap());
  
  setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
  enableAxis(QwtPlot::yRight);
  
  plotLayout()->setAlignCanvasToScales(true);
  replot();
  
  // LeftButton for the zooming
  // MidButton for the panning
  // RightButton: zoom out by 1
  // Ctrl+RighButton: zoom out to full size
  
  Hist2DZoomer* zoomer = new Hist2DZoomer(canvas());
  zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
			  Qt::RightButton, Qt::ControlModifier);
  zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
			  Qt::RightButton);
  zoomer->setData(mRasterData);

  QwtPlotPanner *panner = new QwtPlotPanner(canvas());
  panner->setAxisEnabled(QwtPlot::yRight, false);
  panner->setMouseButton(Qt::MidButton);
  
  // Avoid jumping when labels with more/less digits
  // appear/disappear when scrolling vertically
  
  const QFontMetrics fm(axisWidget(QwtPlot::yLeft)->font());
  QwtScaleDraw *sd = axisScaleDraw(QwtPlot::yLeft);
  sd->setMinimumExtent( fm.width("100.00") );
  
  const QColor c(Qt::darkBlue);
  zoomer->setRubberBandPen(c);
  zoomer->setTrackerPen(c);
}
  
void QwtHistogram2DDataPlot::showContour(bool on)
{
    mSpectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, on);
    replot();
}

void QwtHistogram2DDataPlot::showSpectrogram(bool on)
{
    mSpectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, on);
    mSpectrogram->setDefaultContourPen(on ? QPen() : QPen(Qt::NoPen));
    replot();
}


} // end namespace cmp
