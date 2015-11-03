//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#include "cmpImage.h"
#include <iostream>
#include <highgui.h>
#include "cmpUtils.h"
#include <limits>
#include <ctime>

namespace cmp {
  
static CvRNG rng = cvRNG(time(NULL));
int Cluster::clusterId = 0;


// ---------  CLUSTER
Cluster::Cluster(int id, int width, int height):
  mId(id),
  mWidth(width),
  mHeight(height),
  mMask(false)
{
  mImage.resize(mWidth*mHeight, false);
  mColor = QColor(cvRandInt(&rng)%255,cvRandInt(&rng)%255,cvRandInt(&rng)%255);
  mName = QString("Cluster-%1").arg(mId+1);
}

Cluster::~Cluster()
{
}

IplImage *Cluster::asIplImage() const
{
  IplImage *ans = cvCreateImage( cvSize(mWidth,mHeight), IPL_DEPTH_8U, 1 );
  
  std::vector<bool>::const_iterator it = mImage.begin();  
  for (int i = 0; i < mHeight; i++)
    {
    for (int j = 0; j < mWidth; j++)
      {
      if ((*it) == true)
        {
        ((uchar *)(ans->imageData + i*ans->widthStep))[j]=1;
        }
      else
        {
        ((uchar *)(ans->imageData + i*ans->widthStep))[j]=0;
        }
      it++;
      }
    }
  
  return ans;
}

QString Cluster::name()
{
  return mName;
}

void Cluster::setName(const QString &name)
{
  mName = name;
}

void Cluster::resetImage()
{
  mImage.clear();
  mImage.resize(mWidth*mHeight, false);
}

bool Cluster::image(int x, int y) const
{
  if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
    return false;
  return mImage[y*mWidth+x];
}

void Cluster::setImage(int x, int y, bool val)
{
  mImage[y*mWidth+x] = val;
}

// ------------- IMAGE 
Image::Image():
  mOriginal(NULL),
  mModified(NULL),
  mName("")
{
  mColor = QColor(cvRandInt(&rng)%255,cvRandInt(&rng)%255,cvRandInt(&rng)%255);
}

Image::~Image()
{
  if (mOriginal)
    cvReleaseImage(&(mOriginal));
  if (mModified)
    cvReleaseImage(&(mModified));
}

bool Image::load(QString fname)
{
  if (mOriginal) cvReleaseImage(&(mOriginal));

  // Load image, forcing it to be a 3-channel grayscale image (flag > 1);
  //  mOriginal = cvLoadImage(fname.toAscii(),1);
    mOriginal = cvLoadImage(fname.toLatin1(),1);
  if (!mOriginal) return false;

  resetModified();
  
  QFileInfo fi(fname);
  mName = fi.fileName();
  
  return true;
}

void Image::minMax(double &min, double &max) const
{
  // If 3-channels, need to convert to grayscale
  if (mOriginal->nChannels > 1)
    {
      IplImage *gray = cvCreateImage( cvGetSize(mOriginal), 8, 1 );
      cvCvtColor( mOriginal, gray, CV_BGR2GRAY );
      cvMinMaxLoc(gray, &min, &max, NULL, NULL, 0);
      cvReleaseImage(&gray);
    }
  else
    {
      cvMinMaxLoc(mOriginal, &min, &max, NULL, NULL, 0);
    }
}

double *Image::sampleList8u(const std::vector<CvPoint> &coords) const
{
  unsigned int sz = coords.size();
  double *ans = new double[sz];

  // If image is not single-channel, then create a grayscale version of the image.
  IplImage *gray = NULL;
  if (this->nChannels() > 1 || this->depth() > 1)
    {
    gray = cvCreateImage( cvGetSize(mOriginal), 8, 1 );
    cvCvtColor( mOriginal, gray, CV_BGR2GRAY );

    for (unsigned int i = 0; i < coords.size(); i++)
      {
	ans[i] = static_cast<double>(((uchar*)(gray->imageData + gray->widthStep * coords[i].y))[coords[i].x]) ;
      }
    cvReleaseImage(&gray);
    }
  else
    {
      for (unsigned int i = 0; i < coords.size(); i++)
	{
	  ans[i] = static_cast<double>(((uchar*)(mOriginal->imageData + mOriginal->widthStep*coords[i].y))[coords[i].x]) ;
	}
    }

  
  return ans;
}

std::vector<CvPoint> Image::randomCoordinates(unsigned int numsamples) const
{
  CvRNG rngtmp = cvRNG(time(NULL));
  int xmax = this->width();
  int ymax = this->height();
  
  std::vector<CvPoint> ans(numsamples);

  for (unsigned int i = 0; i < numsamples; i++)
    {
      CvPoint tmp;
      tmp.x = cvRandInt(&rngtmp)%xmax;
      tmp.y = cvRandInt(&rngtmp)%ymax;
      ans[i] = tmp;
    }

  return ans;
}

std::vector<CvPoint> Image::randomCoordinates(unsigned int numsamples,
                                              const Cluster *cluster) const
{
  CvRNG rngtmp = cvRNG(time(NULL));
  int xmax = this->width();
  int ymax = this->height();
  
  std::vector<CvPoint> ans(numsamples);
  
  unsigned int i = numsamples;
  while (i > 0)
    {
    CvPoint tmp;
    tmp.x = cvRandInt(&rngtmp)%xmax;
    tmp.y = cvRandInt(&rngtmp)%ymax;
    if (cluster->image(tmp.x,tmp.y) == true)
      {
      ans[i] = tmp;
      i--;
      }
    }

  return ans;
}


CvHistogram *Image::computeHistogram(bool mod, int bins, const Cluster *mask) const
{
  // Pick image to use--modified or original
  IplImage *img;
  if (mod != true) img = mModified;
  else img = mOriginal;
  
  // Compute range of image and set the min/max accordingly
  float ranges[2];
  double min, max;
  this->minMax(min,max);
  ranges[0] = (float) min;
  ranges[1] = (float) max;

  // If image is not single-channel, then create a grayscale version of the image.
  IplImage *gray;
  if (img->nChannels > 1)
    {
    gray = cvCreateImage( cvGetSize(img), 8, 1 );
    cvCvtColor( img, gray, CV_BGR2GRAY );
    }

  // Need an array of bin sizes and an array of images
  //  int binsArr[] = {bins};
  int binsArr[1];
  binsArr[0] = bins;
  IplImage *imgArr[1];
  if (img->nChannels>1)  { imgArr[0] = gray; }
  else  { imgArr[0] = img; }

  float *rangesArr[1];
  rangesArr[0] = ranges;

  // Create and compute histogram
  CvHistogram *hist = cvCreateHist( 1, binsArr, CV_HIST_ARRAY, rangesArr,1);


  IplImage *mymask;
  if (mask != NULL) mymask = mask->asIplImage();
  
  //  std::cout << "mymask = " << (void *)mymask << std::endl;

  if (mask == NULL)  cvCalcHist( imgArr, hist, 0, NULL);
  else cvCalcHist( imgArr, hist, 0, mymask);

  // USE cvReleaseHistogram(& CvHistogram*) to delete cvHistogram data!
  if (img->nChannels>1) cvReleaseImage(&gray);
  
  if (mask != NULL) cvReleaseImage(&mymask);
  
  return hist;
}

CvHistogram *Image::compute2DHistogram(bool mod, int bins, const Image *otherImage, 
				       const Cluster *mask) const
{
  // Pick image to use--modified or original
  IplImage *img1;
  IplImage *img2;
  if (mod != true) 
    {
      img1 = mModified;
      img2 = otherImage->mModified;
    }
  else 
    {
      img1 = mOriginal;
      img2 = otherImage->mModified;
    }
  
  // Compute range of images and set the min/max accordingly
  float ranges1[2];
  double min, max;
  this->minMax(min,max);
  ranges1[0] = (float) min;
  ranges1[1] = (float) max;

  float ranges2[2];
  otherImage->minMax(min,max);
  ranges2[0] = (float) min;
  ranges2[1] = (float) max;

  // If image is not single-channel, then create a grayscale version of the image.
  IplImage *gray1;
  if (img1->nChannels > 1)
    {
    gray1 = cvCreateImage( cvGetSize(img1), 8, 1 );
    cvCvtColor( img1, gray1, CV_BGR2GRAY );
    }

  IplImage *gray2;
  if (img2->nChannels > 1)
    {
    gray2 = cvCreateImage( cvGetSize(img2), 8, 1 );
    cvCvtColor( img2, gray2, CV_BGR2GRAY );
    }

  // Need an array of bin sizes and an array of images
  //  int binsArr[] = {bins};
  int binsArr[2];
  binsArr[0] = bins;
  binsArr[1] = bins;

  IplImage *imgArr[2];
  if (img1->nChannels>1)  { imgArr[0] = gray1; }
  else  { imgArr[0] = img1; }
  if (img2->nChannels>1)  { imgArr[1] = gray2; }
  else  { imgArr[1] = img2; }

  float *rangesArr[2];
  rangesArr[0] = ranges1;
  rangesArr[1] = ranges2;

  // Create and compute histogram
  CvHistogram *hist = cvCreateHist( 2, binsArr, CV_HIST_ARRAY, rangesArr,1);

  IplImage *mymask;
  if (mask != NULL) mymask = mask->asIplImage();
  
  //  std::cout << "mymask = " << (void *)mymask << std::endl;

  if (mask == NULL)  cvCalcHist( imgArr, hist, 0, NULL);
  else cvCalcHist( imgArr, hist, 0, mymask);

  // USE cvReleaseHistogram(& CvHistogram*) to delete cvHistogram data!
  if (img1->nChannels>1) cvReleaseImage(&gray1);
  if (img2->nChannels>2) cvReleaseImage(&gray2);
  
  if (mask != NULL) cvReleaseImage(&mymask);
  
  return hist;
}


//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void Image::resetModified()
{
  if (!mOriginal)
    return;
  if (mModified)
    cvReleaseImage(&(mModified));
  mModified = cvCreateImage(cvSize(mOriginal->width, mOriginal->height), 
                            mOriginal->depth, mOriginal->nChannels);
  cvCopy(mOriginal,mModified,NULL);
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
ImageStack::ImageStack():
  mSelectedIndex(-1)
{
  mClusterBackgroundColor = QColor("gray");
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
ImageStack::~ImageStack()
{
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
bool ImageStack::addImage(QString fname)
{
  Image *img = new Image();
  if (!img->load(fname))
    return false;
  mImages.push_back(img);
  mIconTypes.push_back(0);

  mSelectedIndex = mImages.size()-1;
  clearClusters();
  return true;
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
bool ImageStack::removeImage(QString fname)
{
  for (unsigned int i = 0; i < mImages.size(); i++)
  {
    if (fname.compare(mImages[i]->name())==0)
    {
      delete mImages[i];
      mImages.erase(mImages.begin()+i);
      mIconTypes.erase(mIconTypes.begin()+i);
      if (mSelectedIndex >= 0 && (unsigned int)mSelectedIndex == i)
        {
        mSelectedIndex = -1;
        }
      clearClusters();
      return true;
    }
  }

  return false;
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
QImage ImageStack::selectedImage(bool original)
{
  if (mSelectedIndex >= 0)
  {
    if (original)
      {
      return IplImageToQImage(mImages[mSelectedIndex]->original());
      }
    else
      return IplImageToQImage(mImages[mSelectedIndex]->modified());
  }
  return QImage();
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
QImage ImageStack::image(unsigned int i, bool original)
{
  if (i < mImages.size())
  {
    if (original)
      return IplImageToQImage(mImages[i]->original());
    else
      return IplImageToQImage(mImages[i]->modified());
  }
  return QImage();
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
QColor ImageStack::imageColor(unsigned int i)
{
  if (mImages.size() > i)
    return mImages[i]->color();
  else
    return QColor(0,0,0);
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
QString ImageStack::name(unsigned int i)
{
  if (i < mImages.size())
  {
    return mImages[i]->name();
  }
  return QString();
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
QString ImageStack::selectedName()
{
  if (mSelectedIndex >= 0)
  {
    return mImages[mSelectedIndex]->name();
  }
  return QString();
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void ImageStack::setSelectedByName(QString fname)
{
  for (unsigned int i = 0; i < mImages.size(); i++)
  {
    if (fname.compare(mImages[i]->name())==0)
    {
    mSelectedIndex = (int)i;
    }
  }
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void ImageStack::clearSelection()
{
  mSelectedIndex = -1;
}


//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
bool ImageStack::clusterMasked(unsigned int i)
{
  if (mClusters.size() > i)
    return mClusters[i]->mask();
  else
    return false;
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void ImageStack::setClusterMasked(unsigned int i, bool val)
{
  if (mClusters.size() > i)
    mClusters[i]->setMask(val);
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
QString ImageStack::clusterName(unsigned int i)
{
  if (mClusters.size() > i)
    return mClusters[i]->name();
  else
    return QString("");
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void ImageStack::setClusterName(unsigned int i, const QString &name)
{
  if (mClusters.size() > i)
    mClusters[i]->setName(name);
}


//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
QColor ImageStack::clusterColor(unsigned int i)
{
  if (mClusters.size() > i)
    return mClusters[i]->color();
  else
    return QColor(0,0,0);
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
bool ImageStack::pixelMasked(int x, int y)
{
  for (unsigned int i = 0; i < mClusters.size(); i++)
  {
    if (mClusters[i]->mask() && mClusters[i]->image(x,y))
      return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
int ImageStack::pixelCluster(int x, int y)
{
  for (unsigned int i = 0; i < mClusters.size(); i++)
  {
    if (mClusters[i]->image(x,y))
      return i;
  }
  return -1;
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void ImageStack::cleanupClusters()
{
  //  int size = mClusters.size();
  for (int i = mClusters.size()-1; i >= 0; i--)
  {
    if (!mClusters[i]->mask())
    {
      delete mClusters[i];
      mClusters.erase(mClusters.begin()+i);
    }
  }
}


//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void ImageStack::clearClusters()
{
  for (unsigned int i = 0; i < mClusters.size(); i++)
    delete mClusters[i];
  mClusters.clear();
  mClusterImage = QImage();
}


//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void ImageStack::bilateralFilter(int kernel, double colorSigma, double spaceSigma)
{
  emit progressChanged(0);
  for (unsigned int i = 0; i < mImages.size(); i++)
  {
    cvSmooth(mImages[i]->original(), mImages[i]->modified(), CV_BILATERAL, kernel, kernel, 
             colorSigma, spaceSigma);
    emit progressChanged(int(double(i+1)/double(mImages.size())*100.0));
  }
  clearClusters();
}

void ImageStack::resetFilters()
{
  for (unsigned int i = 0; i < mImages.size(); i++)
  {
    mImages[i]->resetModified();
  }
}

int ImageStack::kMeansCluster(unsigned int clusterCount,
			      int maxIters,
			      double accuracy, 
			      unsigned int numTrials,
			      const std::vector<QColor> &colors,
			      std::vector<std::vector<double> > &outputCenters,
			      double &compactness)
{
  if (!mImages.size())  return -1;
  
  // Error checking
  for (unsigned int i = 0; i < mImages.size(); i++)
  {
    assert(mImages[i]->depth() == IPL_DEPTH_8U &&
           mImages[i]->nChannels() == 3);
  }
  
  emit progressChanged(0);

  // Generate a list of used colors.  Marks the indices in the list
  // of colors that have been used. Kind of slow, but should be fine
  // unless the color list is huge.
  unsigned int numcols = colors.size();
  std::vector<bool> usedColors(numcols);
  for (unsigned int j = 0; j < numcols; j++)
    {
    usedColors[j] = false;
    for (unsigned int i = 0; i < mClusters.size(); i++)
      {
	  if (mClusters[i]->mask() == true) 
	    {
        if (colors[j].name() == mClusters[i]->color().name()) usedColors[j] = true;
	    }
      }
    }
  
  // Clear out any unmasked clusters
  this->cleanupClusters();
  
  // We don't need to do anything!
  if (mClusters.size() >= clusterCount) return -1;
  
  clusterCount-=mClusters.size();
  
  IplImage *img = mImages[0]->modified();
  int width = img->width;
  int height = img->height;
  int numImages = mImages.size();
  
  //unsigned long int sampleCount = 0;
  int sampleCount = 0;
  
  // Build mask
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      if (!pixelMasked(x,y))
      {
        sampleCount++;
 
        // Handle case with too many pixels
        if (sampleCount == INT_MAX)
          //        if (sampleCount == std::numeric_limits<int>::max()) 
          { 
          throw cmp::Exception("CMP- Overflow in pixel count during clusting.  This means the images are too big or there are too many images."); 
          }
      }
    }
  }
  
  // This only should happen if everything is already clustered
  if (sampleCount == 0) return -1;
  
  // Allocate n-dimensional vector and result image
  CvMat *points   = cvCreateMat(sampleCount, numImages, CV_32FC1);
  CvMat *clusters = cvCreateMat(sampleCount, 1, CV_32SC1);
  CvMat *centers  = cvCreateMat(clusterCount,numImages,CV_32FC1);

  int count = 0;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      if (!pixelMasked(x,y))
      {
        for (int i = 0; i < numImages; i++)
        {
	   cvmSet(points, count, i, ((uchar*)(mImages[i]->modified()->imageData + 
					      mImages[i]->modified()->widthStep*y))[x*3+0]);
        }
        count++;
      }
    }
  }

  emit progressChanged(25);
  
  // Perform the clustering.
  // http://opencv.willowgarage.com/documentation/clustering_and_search_in_multi-dimensional_spaces.html#cvKMeans2
  int result = 
    cvKMeans2(points,         // Floating point matrix of input samples, one row per sample
	      clusterCount,   // Number of clusters
	      clusters,       // Output labels
	      cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, maxIters, accuracy),            
	      numTrials,              // Attempts
	      new CvRNG(time(NULL)), // New seed for random number generation
	      0,              // Flags
	      centers,        // Output matrix of the cluster centers            
	      &compactness     // Output double indicating compactness (sum of
	      // squared distances of samples from centers)                          
	      );
  emit progressChanged(50);
  
  // Store info on results
  if (outputCenters.size() != 0)
    {
    for (unsigned int i = 0; i < outputCenters.size(); i++) outputCenters[i].clear();
    }
  outputCenters.clear();

  outputCenters.resize(clusterCount);  
  for (unsigned int i = 0; i < (unsigned int)clusterCount; i++)
    {
    outputCenters[i].resize(numImages);
    for (unsigned int k = 0; k < (unsigned int) numImages; k++)
      {	
	outputCenters[i][k] = (double)cvmGet(centers,i,k);
      }    
    }
  
  
  // Store results in cluster array
  int prevNumClusters = mClusters.size();
  for (unsigned int i = 0; i < clusterCount; i++)
    {
      Cluster *c = new Cluster(Cluster::clusterId++, width, height);
      mClusters.push_back(c);

      // Find the next available color
      bool tmpSet = false;
      for (unsigned int j = 0; j < colors.size(); j++)
	{
	  if (usedColors[j] == false) 
	    {
	      c->setColor(colors[j]);
	      tmpSet = true;
	      usedColors[j] = true;
	      break;
	    }
	}
      if (tmpSet == false) 	  // Case where we ran out of colors
	{
	  c->setColor(QColor(cvRandInt(&rng)%255,cvRandInt(&rng)%255,cvRandInt(&rng)%255));
	}
      
    } // end for i
  count = 0;
  for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
	{
	  if (!pixelMasked(x,y))
	    {
	      int index = (clusters->data.ptr + clusters->step * count)[0];
	      mClusters[prevNumClusters+index]->setImage(x,y,true);
	      count++;
	    }
	}
    }
  emit progressChanged(75);
  // Create cluster image
  mClusterImage = QImage(width, height, QImage::Format_Indexed8);
  
  for (unsigned int i = 0; i < mClusters.size(); i++)
    {
      mClusterImage.setColor(i, mClusters[i]->color().rgb()); 
    }
  for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
	{
	  int idx = pixelCluster(x,y);
	  mClusterImage.setPixel(x,y,idx);
	}
    }

  emit progressChanged(100);
  cvReleaseMat(&clusters);
  cvReleaseMat(&points);
  return result;
}

QImage ImageStack::maskedClusterImage()
{
  QImage img(mClusterImage);

  for (unsigned int i = 0; i < mClusters.size(); i++)
    {
      if (mClusters[i]->mask())
	{
	  // QColor maskColor(mClusters[i]->color());
	  // maskColor.setAlpha(0);
	  // maskColor.setRed(200);
	  // maskColor.setGreen(200);
	  // maskColor.setBlue(200);
	  // img.setColor(i, maskColor.rgba());      
	  //	  img.setColor(i, QColor("lightgray").rgba());
	  img.setColor(i, mClusterBackgroundColor.rgba());
	}
    }
  
  // NOTE -- Comment out preceding and uncomment next line to have a textured background
  // this->stripeClusters(img);

  return img;
}


void ImageStack::stripeClusters(QImage &img) const
{
  //  img = img.convertToFormat(QImage::Format_ARGB32);
  
  //return;
  static const uchar sprite[25] = { 1, 1, 0, 0, 1,
                                    0, 1, 1, 1, 0,
                                    0, 0, 1, 1, 0,
                                    0, 1, 0, 1, 1,
                                    1, 0, 0, 0, 1};
  

  const int n = 5;
  
  //static const uchar sprite[16] = { 0, 0, 0, 0,
  //                                  0, 1, 1, 0,
  //                                  0, 1, 1, 0,
  //                                  0, 0, 0, 0};
  
  int yIdx = 0;
  int xIdx = 0;
  QColor colBG("black");
  QColor colFG;
  colFG.setRed(200);
  colFG.setBlue(200);
  colFG.setGreen(200);
  colFG.setAlpha(255);
  colBG.setAlpha(255);

  int fg = mClusters.size();
  int bg = mClusters.size()+1;
  img.setColorCount(mClusters.size()+2);
  img.setColor(fg,colBG.rgba());
  img.setColor(bg,colFG.rgba());
  
  for (int y = 0; y < img.height(); y++)
    {
    for (int x = 0; x < img.width(); x++)
      {
      for (unsigned int i = 0; i < mClusters.size(); i++)
        {
        if (mClusters[i]->mask() && mClusters[i]->image(x,y))
          {      
          int idx = n*yIdx + xIdx;
          if (sprite[idx]==1)
            {
            img.setPixel(x,y,bg);
            //  img.setPixel(x,y,colFG.rgba());
            }
          // else
          //  {
          //  img.setPixel(x,y,bg);
          //  img.setPixel(x,y,colBG.rgba());
          //  }
          break;
          }
        } // end for i
      xIdx = (xIdx+1) % n;      
      } // end for x
    yIdx = (yIdx+1) % n;      
    }  // end for y
}

QImage ImageStack::selectedClusterImage(std::vector<bool> &clusters)
{
  QImage img(mClusterImage);
  for (unsigned int i = 0; i < clusters.size(); i++)
  {
    if (!clusters[i])
    {
      QColor c(0,0,0,0);
      img.setColor(i, c.rgba());
    }
  }
  return img;
}

QImage ImageStack::imageInMask(int i, int cluster)
{
  QImage img = image(i,false);
  for (int y = 0; y < img.height(); y++)
  {
    for(int x = 0; x < img.width(); x++)
    {
      if (!mClusters[cluster]->image(x,y))
        img.setPixel(x,y,QColor(0,0,0).rgb());
    }
  }
  return img;
}

} // end namespace cmp
