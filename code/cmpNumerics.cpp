#include "cmpNumerics.h"

namespace cmp {

void PCA::operator()(const double *x, const double *y, unsigned int numsamples)
{ 
  // Compute eigenvalue decomposition of covariance matrix
  cv::Mat C(2,2,cv::DataType<double>::type); // covariance matrix
  mA = cv::Mat(1,2,cv::DataType<double>::type); // mean values
  //  cv::Mat A(1,2,cv::DataType<double>::type); // mean values

  cv::Mat *Vs = new cv::Mat[numsamples]; // for the inputs

  // Construct list of 2D samples
  for (unsigned int i = 0; i < numsamples; i++)
    {
      Vs[i] = cv::Mat(1,2,cv::DataType<double>::type);
      Vs[i].at<double>(0,0) = x[i];
      Vs[i].at<double>(0,1) = y[i];
    }

  // Compute covariance matrix.  Make sure covariance is scaled so the
  // resulting eigenvectors are properly scaled.
  cv::calcCovarMatrix(Vs,numsamples,C,mA,CV_COVAR_NORMAL|CV_COVAR_SCALE,CV_32F);

  // std::cout << "Average value = " << A.at<double>(0,0) << "\t" << A.at<double>(0,1) << std::endl;
  // std::cout << "Covariance matrix = " 
  // 	    << std::endl << C.at<double>(0,0) << "\t" << C.at<double>(0,1) 
  //  	    << std::endl << C.at<double>(1,0) << "\t" << C.at<double>(1,1) << std::endl;  

  mE = cv::Mat(2,2,cv::DataType<double>::type);
  mL = cv::Mat(2,1,cv::DataType<double>::type);
  
  //  cv::Mat E(2,2,cv::DataType<double>::type);
  //  cv::Mat l(2,1,cv::DataType<double>::type);
  cv::eigen(C,mL,mE);  
}


cv::Mat PCA::PCAtoWorld(double x, double y) const
{
  cv::Mat v(1,2,cv::DataType<double>::type);
  cv::Mat l(2,2,cv::DataType<double>::type);
  
  v.at<double>(0,0) = x;
  v.at<double>(0,1) = y;
  
  l.at<double>(0,0) = sqrt(this->eigenvalues().at<double>(0,0)); l.at<double>(0,1) = 0.0;
  l.at<double>(1,0) = 0.0; l.at<double>(1,1) = sqrt(this->eigenvalues().at<double>(0,1)); 

  return (v * (l * this->eigenvectors()) + this->mean());
}


} // end namespace cmp
