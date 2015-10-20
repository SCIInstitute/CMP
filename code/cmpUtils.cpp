//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------

#include "cmpUtils.h"
//#include <vtkImageData.h>
//#include <vtkUnsignedCharArray.h>

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
QImage IplImageToQImage(const IplImage *iplImage)
{
  if (iplImage->depth != IPL_DEPTH_8U)
  {
    throw "Error: Only images with depths of 8 are supported!\n";
  }
  
  uchar *qImageBuffer = NULL;
  int width = iplImage->width;
  int height = iplImage->height;
  int widthStep = iplImage->widthStep;
  QImage::Format format;

  if (iplImage->nChannels == 1)
  {
    qImageBuffer = new uchar[width * height];
    uchar *qptr = qImageBuffer;
    const uchar *iptr = (const uchar *)iplImage->imageData;
    for (int y = 0; y < height; y++)
    {
      memcpy(qptr, iptr, width);
      qptr += width;
      iptr += widthStep;
    }
    format = QImage::Format_Indexed8;
  }
  else if (iplImage->nChannels == 3)
  {
    qImageBuffer = new uchar[width * height * 4];
    uchar *qptr = qImageBuffer;
    const uchar *iptr = (const uchar *)iplImage->imageData;
    for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
      {
        qptr[0] = iptr[0];
        qptr[1] = iptr[1];
        qptr[2] = iptr[2];
        qptr[3] = 0;
        
        qptr+=4;
        iptr+=3;
      }
      iptr += widthStep - 3 * width;
    }
    format = QImage::Format_RGB32;
  }
  else
  {
    throw "Error: Only images with 1 or 3 channels are supported!\n";
  }

  QImage qimage(qImageBuffer, width, height, format);
  return qimage;
}


IplImage *QImageToIplImage(const QImage &qImage)
{
  // see http://osdir.com/ml/lib.opencv/2005-12/msg00019.html
  IplImage *cvImage = cvCreateImageHeader(cvSize(qImage.width(),qImage.height()), IPL_DEPTH_8U,4);
  cvImage->imageData = (char*)qImage.bits();
  return cvImage;
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
// bool QImageToVTKImage(const QImage &img, vtkImageData* vtkimage)
// {
//   int height = img.height();
//   int width = img.width();
  
//   // TODO -- Make sure pixel access is working correctly in this case.
//   int numcomponents = img.hasAlphaChannel() ? 4 : 3;
//   //int numcomponents = sizeof(QRgb);
  
//   vtkimage->SetWholeExtent(0, width-1, 0, height-1, 0, 0); 
//   vtkimage->SetSpacing(1.0, 1.0, 1.0);
//   vtkimage->SetOrigin(0.0, 0.0, 0.0);
//   vtkimage->SetNumberOfScalarComponents(numcomponents);
//   vtkimage->SetScalarType(VTK_UNSIGNED_CHAR);
//   vtkimage->SetExtent(vtkimage->GetWholeExtent());
//   vtkimage->AllocateScalars();
//   for(int i=0; i<height; i++)
//     {
//     unsigned char* row;
//     row = static_cast<unsigned char*>(vtkimage->GetScalarPointer(0, height-i-1, 0));
//     const QRgb* linePixels = reinterpret_cast<const QRgb*>(img.scanLine(i));
//     for(int j=0; j<width; j++)
//     {
//       const QRgb& col = linePixels[j];
//       row[j*numcomponents] = qRed(col);
//       row[j*numcomponents+1] = qGreen(col);
//       row[j*numcomponents+2] = qBlue(col);
//       if(numcomponents == 4)
//         {
//         row[j*numcomponents+3] = qAlpha(col);
//         }
//     }
//   }
//   return true;
// }


// //-------------------------------------------------------------------------------------
// //
// //-------------------------------------------------------------------------------------
// QImage VTKImageToQImage(vtkRenderWindow *rWin)
// {  
//   int h = rWin->GetSize()[1];
//   int w = rWin->GetSize()[0];

//   QImage img(w, h, QImage::Format_RGB32);
//   vtkUnsignedCharArray* pixels = vtkUnsignedCharArray::New();
//   pixels->SetArray(img.bits(), w*h*4, 1);
//   rWin->GetRGBACharPixelData(0, 0, w-1, h-1, 1, pixels);
//   pixels->Delete();
//   img = img.rgbSwapped();
//   img = img.mirrored();
  
//   return img;
// }

// //-------------------------------------------------------------------------------------
// //
// //-------------------------------------------------------------------------------------
// void QColorToVTKColor(const QColor &qc, double *c)
// {
//   c[0] = double(qc.red())/255.0;
//   c[1] = double(qc.green())/255.0;
//   c[2] = double(qc.blue())/255.0;
// }
