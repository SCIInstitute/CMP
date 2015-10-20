//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------

#ifndef _cmpUtils_h_
#define _cmpUtils_h_

#ifdef _WIN32
#pragma warning(push,3)
#endif

#include <QtGui>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include <cv.h>

/** Converts the image in a vtkRenderWindow to a QImage.  This function can be
    used to convert to a QImage for either writing to disk or display purposes. */
QImage IplImageToQImage(const IplImage *iplImage);
IplImage *QImageToIplImage(const QImage &qImage);

#endif
