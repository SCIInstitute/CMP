//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#ifndef _cmpImage_h_
#define _cmpImage_h_

#ifdef _WIN32
#pragma warning(push,3)
#endif

#include <QtGui>
#include <QColor>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include <cv.h>
#include "cmpException.h"

#define cmpRED   1
#define cmpGREEN 2
#define cmpBLUE  4

namespace cmp {

/** */
class Cluster
{
public:
  Cluster(int id, int width, int height);
  ~Cluster();

  QString name();
  void setName(const QString &name);
  QColor color() { return mColor; }
  //  int ColorId() { return mColorId; }
  void setColor(const QColor &c)
    { mColor = c; }
  void resetImage();

  /** Return the on/off value of the given pixel location in the cluster mask. */
  bool image(int x, int y) const;
  void setImage(int x, int y, bool val);
  bool mask() { return mMask; }
  void setMask(bool val) { mMask = val; }
  //  const colorIdx() const {return mColorIdx; }
  // setColorIdx(int i) { mColorIdx = i; }

  const std::vector<bool> &image() const {return mImage;}
  std::vector<bool> &image() {return mImage; }
  
  /** Return an IplImage version of the mask -- expand from bit data. Calling
      function must delete the resulting image*/
  IplImage *asIplImage() const;
  
  int width() { return mWidth; }
  int height() { return mHeight; }

  /** A static counter for labeling clusters */
  static int clusterId;

private:
  std::vector<bool> mImage;
  int mId;
  int mWidth;
  int mHeight;
  bool mMask;
  QColor mColor;
  QString mName;
  //  int mColorIdx;
};

/** */
class Image
{
public:
  Image();
  ~Image();

  bool load(QString fname);
  void resetModified();
  
  int depth()     const { if (mOriginal) return mOriginal->depth;     else return 0; }
  int width()     const { if (mOriginal) return mOriginal->width;     else return 0; }
  int height()    const { if (mOriginal) return mOriginal->height;    else return 0; }
  int nChannels() const { if (mOriginal) return mOriginal->nChannels; else return 0; }
  
  /** Compute a histogram of the image.  Optionally, mask the image data with the given Cluster data. */
  CvHistogram *computeHistogram(bool,int, const Cluster *mask = NULL) const;

  /** Compute a 2D histogram of the given images.  Optionally, mask the image data with the given Cluster data. */
  CvHistogram *compute2DHistogram(bool,int, const Image *,const Cluster *mask = NULL) const;

  IplImage *original() { return mOriginal; }
  IplImage *modified() { return mModified; }
  const IplImage *original() const { return mOriginal; }
  const IplImage *modified() const { return mModified; }

  /** Randomly sample <numsamples> number of coordinates.  Note that this
      function does not actually return any values, but only a set of
      coordinates within the image.  To return the list of associated values,
      use the sampleList functions. */
  std::vector<CvPoint> randomCoordinates(unsigned int numsamples) const;
  
  /** Randomly sample <numsamples> number of coordinates under the given Cluster mask.
      \sa randomCoordinates
  */
  std::vector<CvPoint> randomCoordinates(unsigned int numsamples, const Cluster *) const;

  /** Allocate and return an array of values at the given coordinates in the image. */
  double *sampleList8u(const std::vector<CvPoint> &coords) const;
  
  QColor color() { return mColor; }
  QString name() { return mName; }

  /** Compute the minimum and maximum values in the image. */
  void minMax(double &min, double &max) const; 

private:
  IplImage *mOriginal;
  IplImage *mModified;
  QColor mColor;
  QString mName;
};


class ImageStack : public QObject
{
Q_OBJECT

public:
  ImageStack();
  ~ImageStack();

  // Images
  int numImages() { return mImages.size(); }
  bool addImage(QString fname);
  bool removeImage(QString fname);
  QImage image(unsigned int i, bool original);
  IplImage *iplImage(unsigned int i, bool original)
  {
    if (original) return mImages[i]->original();
    else return mImages[i]->modified();
  }
  const IplImage *iplImage(unsigned int i, bool original) const
  {
    if (original) return mImages[i]->original();
    else return mImages[i]->modified();
  }
  const Image *cmpImage(unsigned int i) const
  { return mImages[i]; }
  Image *cmpImage(unsigned int i) 
  { return mImages[i]; }
  QColor imageColor(unsigned int i);
  QString name(unsigned int i);

  // Selection
  // TODO:  Need const version of most of these functions -jc
  QString selectedName();
  void setSelectedByName(QString fname);
  void clearSelection();
  Image *selectedCmpImage() { return mImages[mSelectedIndex]; }
  const Image *selectedCmpImage() const { return mImages[mSelectedIndex]; }
  QImage selectedImage(bool original);
  IplImage *selectedIplImage(bool original)
  {
    if (original==true) return mImages[mSelectedIndex]->original();
    else return mImages[mSelectedIndex]->modified();
  }

  int selectedIndex() const
  {
    return mSelectedIndex;
  }
  
  // Clusters
  int numClusters() { return mClusters.size(); }
  bool pixelMasked(int x, int y);
  int pixelCluster(int x, int y);
  bool clusterMasked(unsigned int i);
  void setClusterMasked(unsigned int i, bool state);
  void cleanupClusters();
  void clearClusters();
  QColor clusterColor(unsigned int i);
  QString clusterName(unsigned int i);
  void setClusterName(unsigned int i, const QString &name);
  
  /** Get a cluster from the list */
  Cluster *cluster(unsigned int i)  { return mClusters[i]; }
  const Cluster *cluster(unsigned int i) const  { return mClusters[i];  }
  QImage clusterImage() { return mClusterImage; }
  QImage maskedClusterImage();
  QImage selectedClusterImage(std::vector<bool> &clusterSelected);
  QImage imageInMask(int i, int cluster);
  //  void loadClusterColorTable();

  /** Access the list of QColors that the clustering algorithm will use to assign colors to clusters */
  //  const std::vector<QColor>& clusterColorTable() const
  // { return mClusterColorTable;}
  // std::vector<QColor>& clusterColorTable()
  // { return mClusterColorTable;}

  // Icons
  unsigned char &iconType(int i)
  { return mIconTypes[i]; }
  const unsigned char &iconType(int i) const
  { return mIconTypes[i]; }
  
  // Processing
  void bilateralFilter(int kernel, double colorSigma, double spaceSigma);
  void resetFilters();
  int kMeansCluster(unsigned int clusterCount,
		    int maxIters,
		    double accuracy,
		    unsigned int numTrials,
		    const std::vector<QColor>& colors,
		    std::vector<std::vector<double> > &outputCenters,
		    double &compactness);
  
  /** Adds a background striping pattern to the specified clustered regions in
      the given image. */
  void stripeClusters(QImage &) const;
  
  /** Get/set the cluster background color.  This is the color that is
      used when a cluster is unselected and not active. */
  const QColor &clusterBackgroundColor() const
  { return mClusterBackgroundColor; }
  void setClusterBackgroundColor(const QColor &c)
  { mClusterBackgroundColor = c; }

signals:
  void progressChanged(int p);

private:
  int mSelectedIndex;
  std::vector<Image *> mImages;
  std::vector<Cluster *> mClusters;
  std::vector<unsigned char> mIconTypes;
  QImage mClusterImage;
  QColor mClusterBackgroundColor;

  /** The list of colors that are used to assign colors to the clusters */
  std::vector<QColor> mClusterColorTable;
};


} // end namespace cmp
#endif
