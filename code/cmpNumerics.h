#ifndef _cmpNumerics_h_
#define _cmpNumerics_h_

#include <cv.h>

#define CMP_PI 3.14159265358979

namespace cmp {


template<class T>
T cmp_min(const T& a, const T&b)
{
  if (a < b) return a;
  else return b;
}

template<class T>
T cmp_max(const T& a, const T&b)
{
  if (a > b) return a;
  else return b;
}

/** Compute the principal components analysis for a given set of 2D vectors. */
class PCA
{
  
public:
  
  PCA()  {}
  ~PCA() {}

  PCA(const PCA &p)
    {
      mE = p.mE;
      mL = p.mL;
      mA = p.mA;
    }

  /** PCA where vectors are supplied in two lists (component 1 in list 1 and
      component 2 in list 2.  Last parameter is the number of elements in the lists. */
  void operator()(const double *, const double*, unsigned int nsamples);

  /** Get eigenvectors, which are returns as row vectors of a matrix. */
  const cv::Mat &eigenvectors() const
  { return mE; }

  /** Get the eigenvalues, which are the elements of the vector in decreasing
      order of magnitude. */
  const cv::Mat &eigenvalues() const
  { return mL; }

  /** Get the mean sample vector value. */
  const cv::Mat &mean() const
  { return mA; }

  /** Map PCA coordinates (std deviations) to original world coordinates */
  cv::Mat PCAtoWorld(double x, double y) const;
  void PCAtoWorld(double x, double y, double &xa, double &ya) const
  {
    cv::Mat a(1,2,cv::DataType<double>::type);
    a = PCAtoWorld(x,y);
    xa = a.at<double>(0,0);
    ya = a.at<double>(0,1);
  }
 
  
private:
  cv::Mat mE; // eigenvectors
  cv::Mat mL; // eigenvalues
  cv::Mat mA; // mean sample vector
  
}; // end class pca





} // end namespace cmp



#endif
