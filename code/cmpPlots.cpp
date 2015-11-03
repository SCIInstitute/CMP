#include "cmp.h"

namespace cmp {

void CMP::setupPairPlotOptions(QtPairPlotDialog *D) const
{
  // Update cluster list widget with all possible clusters.
  D->mClusterNumberCombo->clear();
 
  for (int i = 0; i < mImageStack->numClusters(); i++)
    {
    QPixmap pix(20,20);
    pix.fill(mImageStack->clusterColor(i));
    D->mClusterNumberCombo->insertItem(D->mClusterNumberCombo->count(),
                                       QIcon(pix), mImageStack->clusterName(i));
    }
  D->mClusterNumberCombo->insertItem(D->mClusterNumberCombo->count(), mNullMaskString); 
  
  CvRNG rng_l(time(NULL));
  
  D->mColors.clear();
  for (unsigned int k = 0; k < D->mNumberOfPairs; k++)
    {
    D->mYAxisCombos[k]->clear();
    D->mXAxisCombos[k]->clear();
        
    QPixmap pix(20,20);   
    if (k < (unsigned int) g_settings->numberOfColors())
      {
	  pix.fill(g_settings->color(k));
      D->mColors.push_back(g_settings->color(k));
      }
    else
      {
      QColor tmpCol(cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255);
      pix.fill(tmpCol);
      D->mColors.push_back(tmpCol);
      }
    D->mComboLabels[k]->setPixmap(pix);
      
    for (int i = 0; i < mImageStack->numImages(); i++)
      {
      D->mYAxisCombos[k]->insertItem(D->mYAxisCombos[k]->count(), mImageStack->name(i));
      D->mXAxisCombos[k]->insertItem(D->mXAxisCombos[k]->count(), mImageStack->name(i));
      }
    D->mYAxisCombos[k]->insertItem(D->mYAxisCombos[k]->count(), mNullImageString);
    D->mXAxisCombos[k]->insertItem(D->mXAxisCombos[k]->count(), mNullImageString);
    D->mYAxisCombos[k]->setCurrentIndex(D->mYAxisCombos[k]->count()-1);
    D->mXAxisCombos[k]->setCurrentIndex(D->mXAxisCombos[k]->count()-1);    
    }
  
 // Setup number of bins and samples spinners.  Provide some default plot
 // settings if they are not present.
  if (g_settings->value("plots/advanced_histogram/number_of_bins").isNull())
    {
      g_settings->setValue("plots/advanced_histogram/number_of_bins", 100);
    }
  if (g_settings->value("plots/advanced_scatterplot/number_of_samples").isNull())
    {
      g_settings->setValue("plots/advanced_scatterplot/number_of_samples", 200);
    }
  D->mNumBins->setValue(g_settings->numberOfBins());
  D->mNumSamples->setValue(g_settings->numberOfSamples());
  
  D->adjustSize();
}


void CMP::plotQuickScatterplot()
{
  // NOTE: this function assumes 8-bit, single-channel data.  
  int numsamples = uiPrefs.plotPrefsNumSamples->value();//g_settings->value("plots/quick_scatterplot/number_of_samples").toInt();
  
  std::vector<int> imgList;
  QList<QListWidgetItem *> items = ui.imageListWidget->selectedItems();
  if (items.size() != 2)
    {
       QMessageBox::warning(this, tr("CMP"),
                            tr("Please select 2 images."),
                            QMessageBox::Ok);        
       return;
    }

  // Retrieve images
  int img1idx = ui.imageListWidget->row(items.at(0));
  int img2idx = ui.imageListWidget->row(items.at(1));
  const Image *img1 = mImageStack->cmpImage(img1idx);
  const Image *img2 = mImageStack->cmpImage(img2idx);
  
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing scatter plot ..."));
  mProgressBar->show();

  double img_min = DBL_MAX;
  double img_max = DBL_MIN;

  // Compute min & max of image values
  double tmp_img_min, tmp_img_max;
  img1->minMax(tmp_img_min,tmp_img_max);
  img2->minMax(img_min,img_max);
  if (tmp_img_min < img_min) img_min = tmp_img_min;
  if (tmp_img_max > img_max) img_max = tmp_img_max;

  // Sample random values from the images
  std::vector<CvPoint> coords = img1->randomCoordinates(numsamples);

  // NOTE: Converts image to an 8-bit unsigned char and single channel before sampling
  double *x = img1->sampleList8u(coords);
  double *y = img2->sampleList8u(coords);
  
 // Construct the plot object
  QwtScatterDataPlot *plot = new QwtScatterDataPlot();
  plot->setTitle("CMP Scatter Plot");
  //  plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
  plot->setAxisTitle(QwtPlot::xBottom, items.at(0)->text());
  plot->setAxisTitle(QwtPlot::yLeft,   items.at(1)->text());
  
  QwtPlotCurve *points = new QwtPlotCurve("text");//item->text());
  QwtSymbol *symb = new QwtSymbol(QwtSymbol::Ellipse,QBrush(Qt::SolidPattern),QPen(Qt::SolidLine),QSize(5,5));
  points->setSymbol(symb);
  points->setStyle(QwtPlotCurve::NoCurve);
  points->setRenderHint(QwtPlotItem::RenderAntialiased);
  points->setLegendAttribute(QwtPlotCurve::LegendShowLine, false);
  points->setPen(QPen(QColor("black")));
  points->attach(plot);
  points->setSamples(x,y,numsamples);

  plot->addData(items.at(0)->text().toStdString(),
                items.at(1)->text().toStdString(),
                x,y,numsamples);

  QMdiSubWindow *sub = new QMdiSubWindow();
  sub->setAttribute(Qt::WA_DeleteOnClose);
  sub->setWidget(plot);
  ui.ImageWindowArea->addSubWindow(sub);
  sub->resize(QSize(ui.ImageWindowArea->width() / 2,ui.ImageWindowArea->height() /2));
  sub->show();
  
  delete[] x;
  delete[] y;
  
  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage();
}

void CMP::plotAdvancedScatterplot()
{
  QtPairPlotDialog ppDialog(g_settings->numberOfPlotPairs());
  ppDialog.setWindowTitle(QApplication::translate("this", "CMP Scatter Plot", 0));
  this->setupPairPlotOptions(&ppDialog);
  if (ppDialog.exec() == 0) return; // if dialog is rejected, do nothing

  // Display the status bar and wait cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing scatterplots ..."));
  mProgressBar->show();

  // Read the number of random samples.
  int numsamples = ppDialog.mNumSamples->value();
  
  // Compile a list of image pairs and colors.
  std::vector<QColor> colList;
  std::vector<std::pair<int,int> > imageIds;
  for (unsigned int i = 0; i < ppDialog.mNumberOfPairs; i++)
    {
    if (ppDialog.mXAxisCombos[i]->currentText() != mNullImageString &&
        ppDialog.mYAxisCombos[i]->currentText() != mNullImageString)
      {
      std::pair<int,int> tmpPair;
      tmpPair.first  = ppDialog.mXAxisCombos[i]->currentIndex();
      tmpPair.second = ppDialog.mYAxisCombos[i]->currentIndex();
      imageIds.push_back(tmpPair);
      colList.push_back(ppDialog.mColors[i]);
      }
    }
  
  if (imageIds.size() > 0)
    {
    if (ppDialog.mClusterNumberCombo->currentText() == mNullMaskString)
      {	  // No mask is selected.  Just sample from the images.
	  this->plotImageScatterplots(imageIds,colList,numsamples); 
      }
    else
      {  // Plot data sampled only under the selected mask
      this->plotImageScatterplots(imageIds,colList,numsamples,ppDialog.mClusterNumberCombo->currentIndex());
      }
    }
  else
    {
    QMessageBox::warning(this, tr("CMP"),
                         tr("Please select one or more images to plot."),
                         QMessageBox::Ok);
    }
  
  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage(); 
}


// void CMP::plotAdvancedScatterplot()
// {
//   if ( uiAdvancedPlotDialog.visImageListWidget->count() <=0 )
//     {  
//       QMessageBox::warning(this, tr("CMP"),
// 			   tr("Scatterplot failed because no image data is available to plot."),
// 			   QMessageBox::Ok);
//       return; 
//     }
  
//   // Adjust the plot dialog layout
//   mAdvancedPlotDialog->setWindowTitle("Scatterplot");
//   uiAdvancedPlotDialog.yAxisImageCombo->show();
//   uiAdvancedPlotDialog.yAxisImageComboLabel->show();
//   uiAdvancedPlotDialog.xAxisImageCombo->hide();
//   uiAdvancedPlotDialog.xAxisImageComboLabel->hide();
//   uiAdvancedPlotDialog.visImageListWidget->show();
//   uiAdvancedPlotDialog.visImageCheckButton->show();
//   uiAdvancedPlotDialog.visImageUncheckButton->show();
//   uiAdvancedPlotDialog.imageDataLabel1->hide();
//   uiAdvancedPlotDialog.imageDataLabel2->show();
//   uiAdvancedPlotDialog.clusterNumberCombo->show();
//   uiAdvancedPlotDialog.clusterNumberComboLabel->show();
//   uiAdvancedPlotDialog.numBins->hide();
//   uiAdvancedPlotDialog.numBinsLabel->hide();
//   uiAdvancedPlotDialog.numSamples->show();
//   uiAdvancedPlotDialog.numSamplesLabel->show();
//   mAdvancedPlotDialog->adjustSize();

//   // Get user input from dialog
//   if (mAdvancedPlotDialog->exec() == 0) return; // if dialog is rejected, do nothing
  
//   // Display the status bar and wait cursor
//   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
//   ui.statusBar->showMessage(tr("Computing scatterplots ..."));
//   mProgressBar->show();

//   CvRNG rng_l(time(NULL));

//   std::vector<QColor> colList;
//   int numsamples = uiAdvancedPlotDialog.numSamples->value();

//   // Compile list of images and clusters  
//   std::vector<int> imageIds;
//   // - first add the y-axis image from the combobox
//   imageIds.push_back(uiAdvancedPlotDialog.yAxisImageCombo->currentIndex());
//   // - now add all of the x-axis images from the list widget, along with colors
//   for (int i = 0; i < uiAdvancedPlotDialog.visImageListWidget->count(); i++)
//     {
//     if (uiAdvancedPlotDialog.visImageListWidget->item(i)->checkState()==Qt::Checked)
//       { imageIds.push_back(i);  }
//     if (i < g_settings->numberOfColors() )
//       { colList.push_back(g_settings->color(i)); }
//     else colList.push_back(QColor(cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255));
//     }
  
//   if (imageIds.size() > 0)
//     {
//     if (uiAdvancedPlotDialog.clusterNumberCombo->currentText() == mNullMaskString)
//       {	  // No mask is selected.  Just sample from the images.
// 	  this->plotImageScatterplots(imageIds,colList,numsamples); 
//       }
//     else
//       {  // Plot data sampled only under the selected mask
//       this->plotImageScatterplots(imageIds,colList,numsamples,uiAdvancedPlotDialog.clusterNumberCombo->currentIndex());
//       }
//     }
//   else
//     {
//     QMessageBox::warning(this, tr("CMP"),
//                          tr("Please select one or more images to plot."),
//                          QMessageBox::Ok);
//     }
  
//   mProgressBar->hide();
//   QApplication::restoreOverrideCursor();
//   ui.statusBar->clearMessage(); 
// }

// void CMP::plotImageScatterplots(const std::vector<int> &imgList,
//                                 const std::vector<QColor> &colList, int numsamples, int clusterId)
// {  
//   // NOTE: this function assumes 8-bit, single-channel data.
//   if (imgList.size() < 2)
//     {
//     QMessageBox::warning(this, tr("CMP"),
//                          tr("Not enough images selected for a scatterplot"),
//                          QMessageBox::Ok);        
//     return;
//     }

//   // Retrieve y-axis image.  This is the data that all other images will plot against
//   const Image *imgY = mImageStack->cmpImage(imgList[0]);
  
//   // Sample random values from the image
//   // NOTE: Converts image to an 8-bit unsigned char and single channel before sampling
//   std::vector<CvPoint> coords;

//   if (clusterId >=0)
//     {  // mask is present
//     coords= imgY->randomCoordinates(numsamples, mImageStack->cluster(clusterId));
//     }
//   else
//     { // no mask
//     coords= imgY->randomCoordinates(numsamples);
//     }
      
   
//   double *y = imgY->sampleList8u(coords);  
  
//   // Find the min/max values for the y axis
//   double y_img_min, y_img_max;
//   imgY->minMax(y_img_min,y_img_max);

//   // Initialize the plot object
//   QwtScatterDataPlot *plot = new QwtScatterDataPlot();
//   plot->setTitle("CMP Scatter Plot");
//   QwtLegend *legend = new QwtLegend();
//   legend->setItemMode(QwtLegend::CheckableItem);
//   plot->insertLegend(legend, QwtPlot::RightLegend);
  
//   // Retrieve x-axis images and scatterplot each against y-axis image
//   double x_img_min = DBL_MAX;
//   double x_img_max = DBL_MIN;
//   for (unsigned int i = 1; i < imgList.size(); i++)
//     {
//     // Retrieve next image
//     const Image *img = mImageStack->cmpImage(imgList[i]);
    
//     // Get min & max counts
//     double tmp_img_min, tmp_img_max;
//     img->minMax(tmp_img_min,tmp_img_max);
//     if (tmp_img_min < x_img_min) x_img_min = tmp_img_min;
//     if (tmp_img_max > x_img_max) x_img_max = tmp_img_max;
    
//     // Sample the set of coordinates from these images
//     double *x = img->sampleList8u(coords);    
    
//     // Add points to this plot
//     QwtPlotCurve *points = new QwtPlotCurve(ui.imageListWidget->item(imgList[i])->text());
//     QwtSymbol *symb = new QwtSymbol(QwtSymbol::Ellipse,QBrush(colList[i-1]),QPen(colList[i-1]),QSize(4,4));
//     points->setSymbol(symb);
//     points->setStyle(QwtPlotCurve::NoCurve);
//     points->setRenderHint(QwtPlotItem::RenderAntialiased);
//     points->setLegendAttribute(QwtPlotCurve::LegendShowLine, false);
//     points->setPen(QPen(colList[i-1]));
//     points->attach(plot);
//     points->setSamples(x,y,numsamples);
    
//     // Make a copy of the data in the plot metadata.
//     plot->addData(ui.imageListWidget->item(imgList[i])->text().toStdString(),
//                   ui.imageListWidget->item(imgList[0])->text().toStdString(),
//                   x,y,numsamples);
    
//     delete[] x;
//     }
  
//   // Set axes
//   plot->setAxisTitle(QwtPlot::yLeft, ui.imageListWidget->item(imgList[0])->text());
//   plot->setAxisScale(QwtPlot::yLeft, y_img_min, y_img_max);
//   plot->setAxisScale(QwtPlot::xBottom, x_img_min, x_img_max);
  
//   // Make a new subwindow for the plot object. Memory is cleaned up
//   // when the user closes this plot subwindow.
//   QMdiSubWindow *sub = new QMdiSubWindow();
//   sub->setAttribute(Qt::WA_DeleteOnClose);
//   sub->setWidget(plot);
//   ui.ImageWindowArea->addSubWindow(sub);
//   sub->resize(QSize(ui.ImageWindowArea->width() / 2,ui.ImageWindowArea->height() /2));
//   sub->show();
  
//   delete[] y;
  
//   mProgressBar->hide();
//   QApplication::restoreOverrideCursor();
//   ui.statusBar->clearMessage();
// }


void CMP::plotImageScatterplots(const std::vector<std::pair<int,int> > &imgList,
                                const std::vector<QColor> &colList, int numsamples, int clusterId)
{  
  // NOTE: this function assumes 8-bit, single-channel data.
  if (imgList.size() < 1)
    {
    QMessageBox::warning(this, tr("CMP"),
                         tr("Not enough images selected for a scatterplot"),
                         QMessageBox::Ok);        
    return;
    }

    // Sample random values from the image
  // NOTE: Converts image to an 8-bit unsigned char and single channel before sampling
  std::vector<CvPoint> coords;
  if (clusterId >=0)
    {  // mask is present
    coords= mImageStack->cmpImage(imgList[0].first)->randomCoordinates(numsamples, mImageStack->cluster(clusterId));    
    }
  else
    { // no mask
    coords= mImageStack->cmpImage(imgList[0].first)->randomCoordinates(numsamples);
    }

   // Initialize the plot object
  QwtScatterDataPlot *plot = new QwtScatterDataPlot();
  plot->setTitle("CMP Scatter Plot");
  QwtLegend *legend = new QwtLegend();
  //  legend->setDefaultItemMode(QwtLegend::CheckableItem);
  plot->insertLegend(legend, QwtPlot::RightLegend);
  
  // Retrieve x-axis images and scatterplot each against y-axis image
  double y_img_min = DBL_MAX;
  double y_img_max = DBL_MIN;
  double x_img_min = DBL_MAX;
  double x_img_max = DBL_MIN;
  
  for (unsigned int i = 0; i < imgList.size(); i++)
    {
    // Retrieve next image pair
    const Image *imgX = mImageStack->cmpImage(imgList[i].first);
    const Image *imgY = mImageStack->cmpImage(imgList[i].second);
  
    // Get min & max counts
    double tmp_img_Xmin, tmp_img_Xmax;
    double tmp_img_Ymin, tmp_img_Ymax;
    imgX->minMax(tmp_img_Xmin,tmp_img_Xmax);
    imgY->minMax(tmp_img_Ymin,tmp_img_Ymax);
    if (tmp_img_Xmin < x_img_min) x_img_min = tmp_img_Xmin;
    if (tmp_img_Xmax > x_img_max) x_img_max = tmp_img_Xmax;
    if (tmp_img_Ymin < y_img_min) y_img_min = tmp_img_Ymin;
    if (tmp_img_Ymax > y_img_max) y_img_max = tmp_img_Ymax;
    
    // Sample the set of coordinates from these images
    double *x = imgX->sampleList8u(coords);
    double *y = imgY->sampleList8u(coords);
    
    // Add points to this plot
    QString qs = QString(ui.imageListWidget->item(imgList[i].first)->text()) + QString("/")
                         +QString(ui.imageListWidget->item(imgList[i].second)->text());
    QwtPlotCurve *points = new QwtPlotCurve(qs);
    QwtSymbol *symb = new QwtSymbol(QwtSymbol::Ellipse,QBrush(colList[i]),QPen(colList[i]),QSize(4,4));
    points->setSymbol(symb);
    points->setStyle(QwtPlotCurve::NoCurve);
    points->setRenderHint(QwtPlotItem::RenderAntialiased);
    points->setLegendAttribute(QwtPlotCurve::LegendShowLine, false);
    points->setPen(QPen(colList[i]));
    points->attach(plot);
    points->setSamples(x,y,numsamples);
    
    // Make a copy of the data in the plot metadata.
    plot->addData(ui.imageListWidget->item(imgList[i].first)->text().toStdString(),
                  ui.imageListWidget->item(imgList[i].second)->text().toStdString(),
                  x,y,numsamples);
    
    delete[] x;
    delete[] y;
    }
  
  // Set axes
  //  plot->setAxisTitle(QwtPlot::yLeft, ui.imageListWidget->item(imgList[0])->text());
  plot->setAxisScale(QwtPlot::yLeft, y_img_min, y_img_max);
  plot->setAxisScale(QwtPlot::xBottom, x_img_min, x_img_max);
  
  // Make a new subwindow for the plot object. Memory is cleaned up
  // when the user closes this plot subwindow.
  QMdiSubWindow *sub = new QMdiSubWindow();
  sub->setAttribute(Qt::WA_DeleteOnClose);
  sub->setWidget(plot);
  ui.ImageWindowArea->addSubWindow(sub);
  sub->resize(QSize(ui.ImageWindowArea->width() / 2,ui.ImageWindowArea->height() /2));
  sub->show();
}

void CMP::plotQuickPawprint()
{
    // NOTE: this function assumes 8-bit, single-channel data.  
  int numsamples = uiPrefs.plotPrefsNumSamples->value();
  //g_settings->value("plots/quick_scatterplot/number_of_samples").toInt();
  
  std::vector<int> imgList;
  QList<QListWidgetItem *> items = ui.imageListWidget->selectedItems();
  if (items.size() != 2)
    {
       QMessageBox::warning(this, tr("CMP"),
                            tr("Please select 2 images in the file manager window."),
                            QMessageBox::Ok);        
       return;
    }
  
  // Get a color list
  std::vector<QColor> colList;
  CvRNG rng_l(time(NULL));
  for (int i = 0; i < items.size(); i++)
    {
    //  QListWidgetItem *item = items.at(i);
    //  imgList.push_back(ui.imageListWidget->row(item));
    if (i < g_settings->numberOfColors() )
      {
	  colList.push_back(g_settings->color(i));
      }
    else colList.push_back(QColor(cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255));
    }
  
  // Retrieve images
  int img1idx = ui.imageListWidget->row(items.at(0));
  int img2idx = ui.imageListWidget->row(items.at(1));
  const Image *img1 = mImageStack->cmpImage(img1idx);
  const Image *img2 = mImageStack->cmpImage(img2idx);
  
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing pawprint plot ..."));
  mProgressBar->show();

  double img_min = DBL_MAX;
  double img_max = DBL_MIN;

  // Compute min & max of image values
  double tmp_img_min, tmp_img_max;
  img1->minMax(tmp_img_min,tmp_img_max);
  img2->minMax(img_min,img_max);
  if (tmp_img_min < img_min) img_min = tmp_img_min;
  if (tmp_img_max > img_max) img_max = tmp_img_max;

  // Sample random values from the images
  std::vector<CvPoint> coords = img1->randomCoordinates(numsamples);

  // NOTE: Converts image to an 8-bit unsigned char and single channel before sampling
  double *x = img1->sampleList8u(coords);
  double *y = img2->sampleList8u(coords);

  // Construct CMP pca object
  PCA pca;
  pca(x,y,numsamples);
 
  // Save the pawprint data
  //  double meanX = pca.mean().at<double>(0,0);
  //  double meanY = pca.mean().at<double>(0,1);
  //  double std1 = sqrt(pca.eigenvalues().at<double>(0,0));
  // double std2 = sqrt(pca.eigenvalues().at<double>(0,1));
  // qreal angle = (qreal)atan(pca.eigenvectors().at<double>(0,1) / (pca.eigenvectors().at<double>(0,0) + 1.0e-10));
  // angle = angle * (180.0 / CMP_PI);

  // Construct the plot object
  QwtPawprintDataPlot *plot = new QwtPawprintDataPlot();    
  plot->setTitle("CMP Pawprint");
  //  plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
  plot->setAxisTitle(QwtPlot::xBottom, items.at(0)->text());
  plot->setAxisTitle(QwtPlot::yLeft,   items.at(1)->text());
  
  QColor tmpcol = colList[0];
  tmpcol.setAlpha(.60 * 255);
  QwtEllipsePlotItem *item1 = new QwtEllipsePlotItem(pca,1,1);
  item1->setRenderHint(QwtPlotItem::RenderAntialiased, true);
  item1->setPen(QPen(tmpcol));
  item1->setBrush(QBrush(tmpcol));
  item1->attach(plot);

  tmpcol.setAlpha(.40 * 255);
  QwtEllipsePlotItem *item2 = new QwtEllipsePlotItem(pca,2,2);
  item2->setRenderHint(QwtPlotItem::RenderAntialiased, true);
  item2->setPen(QPen(tmpcol));
  item2->setBrush(QBrush(tmpcol));
  item2->attach(plot);

  tmpcol.setAlpha(.20 * 255);
  QwtEllipsePlotItem *item3 = new QwtEllipsePlotItem(pca,3,3);
  item3->setRenderHint(QwtPlotItem::RenderAntialiased, true);
  item3->setPen(QPen(tmpcol));
  item3->setBrush(QBrush(tmpcol));
  item3->attach(plot);

  QwtPlotCurve *points = new QwtPlotCurve(items.at(1)->text());
  QwtSymbol *symb = new QwtSymbol(QwtSymbol::Ellipse,QBrush(Qt::SolidPattern),QPen(Qt::SolidLine),QSize(5,5));
  points->setSymbol(symb);
  points->setStyle(QwtPlotCurve::NoCurve);
  points->setRenderHint(QwtPlotItem::RenderAntialiased);
  points->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
  points->setPen(QPen(colList[0]));
  points->attach(plot);
  
  // Reset the sample list to the mean value
  // x[0] = pca.mean().at<double>(0,0);
  // y[0] = pca.mean().at<double>(0,1);
  // points->setSamples(x,y,1);
  points->setSamples(x,y,numsamples);
  
  plot->addData(items.at(0)->text().toStdString(),
                items.at(1)->text().toStdString(),
                pca.eigenvectors(),
                pca.eigenvalues(),
                pca.mean());


  FixedARQMdiSubWindow *sub = new FixedARQMdiSubWindow();
  sub->setAttribute(Qt::WA_DeleteOnClose);
  sub->setWidget(plot);
  ui.ImageWindowArea->addSubWindow(sub);

  sub->show();
  delete[] x;
  delete[] y;
  
  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage(); 
}

void CMP::plotAdvancedPawprint()
{
  QtPairPlotDialog ppDialog(g_settings->numberOfPlotPairs());
  ppDialog.setWindowTitle(QApplication::translate("this", "CMP Pawprint Plot", 0));
  this->setupPairPlotOptions(&ppDialog);
  if (ppDialog.exec() == 0) return; // if dialog is rejected, do nothing

  // Display the status bar and wait cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing pawprints ..."));
  mProgressBar->show();

  // Read the number of random samples.
  int numsamples = ppDialog.mNumSamples->value();
  
  // Compile a list of image pairs and colors.
  std::vector<QColor> colList;
  std::vector<std::pair<int,int> > imageIds;
  for (unsigned int i = 0; i < ppDialog.mNumberOfPairs; i++)
    {
    if (ppDialog.mXAxisCombos[i]->currentText() != mNullImageString &&
        ppDialog.mYAxisCombos[i]->currentText() != mNullImageString)
      {
      std::pair<int,int> tmpPair;
      tmpPair.first  = ppDialog.mXAxisCombos[i]->currentIndex();
      tmpPair.second = ppDialog.mYAxisCombos[i]->currentIndex();
      imageIds.push_back(tmpPair);
      colList.push_back(ppDialog.mColors[i]);
      }
    }
  
  if (imageIds.size() > 0)
    {
    if (ppDialog.mClusterNumberCombo->currentText() == mNullMaskString)
      {	  // No mask is selected.  Just sample from the images.
	  this->plotImagePawprints(imageIds,colList,numsamples); 
      }
    else
      {  // Plot data sampled only under the selected mask
      this->plotImagePawprints(imageIds,colList,numsamples,ppDialog.mClusterNumberCombo->currentIndex());
      }
    }
  else
    {
    QMessageBox::warning(this, tr("CMP"),
                         tr("Please select one or more images to plot."),
                         QMessageBox::Ok);
    }
  
  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage(); 
  
  
}

// void CMP::plotAdvancedPawprint()
// {
//   if ( uiAdvancedPlotDialog.visImageListWidget->count() <=0 )
//     {  
//       QMessageBox::warning(this, tr("CMP"),
// 			   tr("Pawprint plot failed because no image data is available to plot."),
// 			   QMessageBox::Ok);
//       return; 
//     }
  
//   // Adjust the plot dialog layout
//   mAdvancedPlotDialog->setWindowTitle("Pawprints Plot");
//   uiAdvancedPlotDialog.yAxisImageCombo->show();
//   uiAdvancedPlotDialog.yAxisImageComboLabel->show();
//   uiAdvancedPlotDialog.xAxisImageCombo->hide();
//   uiAdvancedPlotDialog.xAxisImageComboLabel->hide();
//   uiAdvancedPlotDialog.visImageListWidget->show();
//   uiAdvancedPlotDialog.visImageCheckButton->show();
//   uiAdvancedPlotDialog.visImageUncheckButton->show();
//   uiAdvancedPlotDialog.imageDataLabel1->hide();
//   uiAdvancedPlotDialog.imageDataLabel2->show();
//   uiAdvancedPlotDialog.clusterNumberCombo->show();
//   uiAdvancedPlotDialog.clusterNumberComboLabel->show();
//   uiAdvancedPlotDialog.numBins->hide();
//   uiAdvancedPlotDialog.numBinsLabel->hide();
//   uiAdvancedPlotDialog.numSamples->show();
//   uiAdvancedPlotDialog.numSamplesLabel->show();
//   mAdvancedPlotDialog->adjustSize();

//   // Get user input from dialog
//   if (mAdvancedPlotDialog->exec() == 0) return; // if dialog is rejected, do nothing
  
//   // Display the status bar and wait cursor
//   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
//   ui.statusBar->showMessage(tr("Computing pawprints ..."));
//   mProgressBar->show();

//   CvRNG rng_l(time(NULL));

//   std::vector<QColor> colList;
//   int numsamples = uiAdvancedPlotDialog.numSamples->value();

//   // Compile list of images and clusters  
//   std::vector<int> imageIds;
//   // - first add the y-axis image from the combobox
//   imageIds.push_back(uiAdvancedPlotDialog.yAxisImageCombo->currentIndex());
//   // - now add all of the x-axis images from the list widget, along with colors
//   for (int i = 0; i < uiAdvancedPlotDialog.visImageListWidget->count(); i++)
//     {
//     if (uiAdvancedPlotDialog.visImageListWidget->item(i)->checkState()==Qt::Checked)
//       { imageIds.push_back(i);  }
//     if (i < g_settings->numberOfColors() )
//       { colList.push_back(g_settings->color(i)); }
//     else colList.push_back(QColor(cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255));
//     }
  
//   if (imageIds.size() > 0)
//     {
//     if (uiAdvancedPlotDialog.clusterNumberCombo->currentText() == mNullMaskString)
//       {	  // No mask is selected.  Just sample from the images.
// 	  this->plotImagePawprints(imageIds,colList,numsamples); 
//       }
//     else
//       {  // Plot data sampled only under the selected mask
//       this->plotImagePawprints(imageIds,colList,numsamples,uiAdvancedPlotDialog.clusterNumberCombo->currentIndex());
//       }
//     }
//   else
//     {
//     QMessageBox::warning(this, tr("CMP"),
//                          tr("Please select one or more images to plot."),
//                          QMessageBox::Ok);
//     }
  
//   mProgressBar->hide();
//   QApplication::restoreOverrideCursor();
//   ui.statusBar->clearMessage(); 
// }


void CMP::plotImagePawprints(const std::vector<std::pair<int,int> > &imgList,
                              const std::vector<QColor> &colList, int numsamples, int clusterId)
{
   // NOTE: this function assumes 8-bit, single-channel data.
  if (imgList.size() < 1)
    {
    QMessageBox::warning(this, tr("CMP"),
                         tr("Not enough images selected for a scatterplot"),
                         QMessageBox::Ok);        
    return;
    }

    // Sample random values from the image
  // NOTE: Converts image to an 8-bit unsigned char and single channel before sampling
  std::vector<CvPoint> coords;
  if (clusterId >=0)
    {  // mask is present
    coords= mImageStack->cmpImage(imgList[0].first)->randomCoordinates(numsamples, mImageStack->cluster(clusterId));    
    }
  else
    { // no mask
    coords= mImageStack->cmpImage(imgList[0].first)->randomCoordinates(numsamples);
    }
  

 // Construct the plot object
  QwtPawprintDataPlot *plot = new QwtPawprintDataPlot();    
  plot->setTitle("CMP Pawprint");
  QwtLegend *legend = new QwtLegend();
  //  legend->setItemMode(QwtLegend::CheckableItem);
  plot->insertLegend(legend, QwtPlot::RightLegend);
  
  // Retrieve x-axis images and scatterplot each against y-axis image
  double y_img_min = DBL_MAX;
  double y_img_max = DBL_MIN;
  double x_img_min = DBL_MAX;
  double x_img_max = DBL_MIN;
  
  for (unsigned int i = 0; i < imgList.size(); i++)
    {
    // Retrieve next image pair
    const Image *imgX = mImageStack->cmpImage(imgList[i].first);
    const Image *imgY = mImageStack->cmpImage(imgList[i].second);
  
    // Get min & max counts
    double tmp_img_Xmin, tmp_img_Xmax;
    double tmp_img_Ymin, tmp_img_Ymax;
    imgX->minMax(tmp_img_Xmin,tmp_img_Xmax);
    imgY->minMax(tmp_img_Ymin,tmp_img_Ymax);
    if (tmp_img_Xmin < x_img_min) x_img_min = tmp_img_Xmin;
    if (tmp_img_Xmax > x_img_max) x_img_max = tmp_img_Xmax;
    if (tmp_img_Ymin < y_img_min) y_img_min = tmp_img_Ymin;
    if (tmp_img_Ymax > y_img_max) y_img_max = tmp_img_Ymax;
    
    // Sample the set of coordinates from these images
    double *x = imgX->sampleList8u(coords);
    double *y = imgY->sampleList8u(coords);


    // Construct CMP pca object
    PCA pca;
    pca(x,y,numsamples);
    
    QColor tmpcol = colList[i];
    tmpcol.setAlpha(.60 * 255);
    QwtEllipsePlotItem *item1 = new QwtEllipsePlotItem(pca,1,1);
    item1->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    item1->setPen(QPen(tmpcol));
    item1->setBrush(QBrush(tmpcol));
    item1->attach(plot);
    
    tmpcol.setAlpha(.40 * 255);
    QwtEllipsePlotItem *item2 = new QwtEllipsePlotItem(pca,2,2);
    item2->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    item2->setPen(QPen(tmpcol));
    item2->setBrush(QBrush(tmpcol));
    item2->attach(plot);
    
    tmpcol.setAlpha(.20 * 255);
    QwtEllipsePlotItem *item3 = new QwtEllipsePlotItem(pca,3,3);
    item3->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    item3->setPen(QPen(tmpcol));
    item3->setBrush(QBrush(tmpcol));
    item3->attach(plot);
    
    
    // Add points to this plot
    QString qs = QString(ui.imageListWidget->item(imgList[i].first)->text()) + QString("/")
                         +QString(ui.imageListWidget->item(imgList[i].second)->text());
    QwtPlotCurve *points = new QwtPlotCurve(qs);
    QwtSymbol *symb = new QwtSymbol(QwtSymbol::Ellipse,QBrush(colList[i]),QPen(colList[i]),QSize(4,4));
    points->setSymbol(symb);
    points->setStyle(QwtPlotCurve::NoCurve);
    points->setRenderHint(QwtPlotItem::RenderAntialiased);
    points->setLegendAttribute(QwtPlotCurve::LegendShowLine, false);
    points->setPen(QPen(colList[i]));
    points->attach(plot);
    points->setSamples(x,y,numsamples);
    
    // Make a copy of the data in the plot metadata.
    plot->addData(ui.imageListWidget->item(imgList[i].first)->text().toStdString(),
                  ui.imageListWidget->item(imgList[i].second)->text().toStdString(),
                  pca.eigenvectors(),
                  pca.eigenvalues(),
                  pca.mean());
    
    delete[] x;
    delete[] y;
    }
  
  // Set axes
  //  plot->setAxisTitle(QwtPlot::yLeft, ui.imageListWidget->item(imgList[0])->text());
  plot->setAxisScale(QwtPlot::yLeft, y_img_min, y_img_max);
  plot->setAxisScale(QwtPlot::xBottom, x_img_min, x_img_max);
  
  // Make a new subwindow for the plot object. Memory is cleaned up
  // when the user closes this plot subwindow.
  QMdiSubWindow *sub = new QMdiSubWindow();
  sub->setAttribute(Qt::WA_DeleteOnClose);
  sub->setWidget(plot);
  ui.ImageWindowArea->addSubWindow(sub);
  sub->resize(QSize(ui.ImageWindowArea->width() / 2,ui.ImageWindowArea->height() /2));
  sub->show();  
}

// void CMP::plotImagePawprints(const std::vector<int> &imgList,
//                                 const std::vector<QColor> &colList, int numsamples, int clusterId)
// {  
//   // NOTE: this function assumes 8-bit, single-channel data.
//   if (imgList.size() < 2)
//     {
//     QMessageBox::warning(this, tr("CMP"),
//                          tr("Not enough images selected for a pawprint"),
//                          QMessageBox::Ok);        
//     return;
//     }

//   // Retrieve y-axis image.  This is the data that all other images will plot against
//   const Image *imgY = mImageStack->cmpImage(imgList[0]);
  
//   // Sample random values from the image
//   // NOTE: Converts image to an 8-bit unsigned char and single channel before sampling
//   std::vector<CvPoint> coords;

//   if (clusterId >=0)
//     {  // mask is present
//     coords= imgY->randomCoordinates(numsamples, mImageStack->cluster(clusterId));
//     }
//   else
//     { // no mask
//     coords= imgY->randomCoordinates(numsamples);
//     }
   
//   double *y = imgY->sampleList8u(coords);  
  
//   // Find the min/max values for the y axis
//   double y_img_min, y_img_max;
//   imgY->minMax(y_img_min,y_img_max);

//   // Initialize the plot object
//   // Construct the plot object
//   QwtPawprintDataPlot *plot = new QwtPawprintDataPlot();    
//   plot->setTitle("CMP Pawprint");
//   QwtLegend *legend = new QwtLegend();
//   legend->setItemMode(QwtLegend::CheckableItem);
//   plot->insertLegend(legend, QwtPlot::RightLegend);
  
//   // Retrieve x-axis images and scatterplot each against y-axis image
//   double x_img_min = DBL_MAX;
//   double x_img_max = DBL_MIN;
//   for (unsigned int i = 1; i < imgList.size(); i++)
//     {
//       // Retrieve next image
//       const Image *img = mImageStack->cmpImage(imgList[i]);
      
//       // Get min & max counts
//       double tmp_img_min, tmp_img_max;
//       img->minMax(tmp_img_min,tmp_img_max);
//       if (tmp_img_min < x_img_min) x_img_min = tmp_img_min;
//       if (tmp_img_max > x_img_max) x_img_max = tmp_img_max;
      
//       // Sample the set of coordinates from these images
//       double *x = img->sampleList8u(coords);    
      
//       // Construct CMP pca object
//       PCA pca;
//       pca(x,y,numsamples);
      
//       QColor tmpcol = colList[i-1];
//       tmpcol.setAlpha(.60 * 255);
//       QwtEllipsePlotItem *item1 = new QwtEllipsePlotItem(pca,1,1);
//       item1->setRenderHint(QwtPlotItem::RenderAntialiased, true);
//       item1->setPen(QPen(tmpcol));
//       item1->setBrush(QBrush(tmpcol));
//       item1->attach(plot);
      
//       tmpcol.setAlpha(.40 * 255);
//       QwtEllipsePlotItem *item2 = new QwtEllipsePlotItem(pca,2,2);
//       item2->setRenderHint(QwtPlotItem::RenderAntialiased, true);
//       item2->setPen(QPen(tmpcol));
//       item2->setBrush(QBrush(tmpcol));
//       item2->attach(plot);
      
//       tmpcol.setAlpha(.20 * 255);
//       QwtEllipsePlotItem *item3 = new QwtEllipsePlotItem(pca,3,3);
//       item3->setRenderHint(QwtPlotItem::RenderAntialiased, true);
//       item3->setPen(QPen(tmpcol));
//       item3->setBrush(QBrush(tmpcol));
//       item3->attach(plot);
      
//       // Add points to this plot
//       QwtPlotCurve *points = new QwtPlotCurve(ui.imageListWidget->item(imgList[i])->text());
//       QwtSymbol *symb = new QwtSymbol(QwtSymbol::Ellipse,QBrush(colList[i-1]),QPen(colList[i-1]),QSize(4,4));
//       points->setSymbol(symb);
//       points->setStyle(QwtPlotCurve::NoCurve);
//       points->setRenderHint(QwtPlotItem::RenderAntialiased);
//       points->setLegendAttribute(QwtPlotCurve::LegendShowLine, false);
//       points->setPen(QPen(colList[i-1]));
//       points->attach(plot);
//       points->setSamples(x,y,numsamples);
      
//       // QwtPlotCurve *points = new QwtPlotCurve(items.at(1)->text());
//       // QwtSymbol *symb = new QwtSymbol(QwtSymbol::Ellipse,QBrush(Qt::SolidPattern),QPen(Qt::SolidLine),QSize(5,5));
//       // points->setSymbol(symb);
//       // points->setStyle(QwtPlotCurve::NoCurve);
//       // points->setRenderHint(QwtPlotItem::RenderAntialiased);
//       // points->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
//       // points->setPen(QPen(colList[0]));
//       // points->attach(plot);
      
//       // Reset the sample list to the mean value
//       // x[0] = pca.mean().at<double>(0,0);
//       // y[0] = pca.mean().at<double>(0,1);
//       // points->setSamples(x,y,1);
//       //    points->setSamples(x,y,numsamples);
      
//       plot->addData(ui.imageListWidget->item(imgList[i])->text().toStdString(),
// 		    ui.imageListWidget->item(imgList[0])->text().toStdString(),
// 		    pca.eigenvectors(),
// 		    pca.eigenvalues(),
// 		    pca.mean());
      
//       delete[] x;
//     }

//   delete[] y;
  
//   // Set axes
//   plot->setAxisTitle(QwtPlot::yLeft, ui.imageListWidget->item(imgList[0])->text());
//   //  plot->setAxisScale(QwtPlot::yLeft, y_img_min, y_img_max);
//   // plot->setAxisScale(QwtPlot::xBottom, x_img_min, x_img_max);
  
//   // Make a new subwindow for the plot object. Memory is cleaned up
//   // when the user closes this plot subwindow.
//   FixedARQMdiSubWindow *sub = new FixedARQMdiSubWindow();
//   sub->setAttribute(Qt::WA_DeleteOnClose);
//   sub->setWidget(plot);
//   ui.ImageWindowArea->addSubWindow(sub);
//   sub->show();
 
//   mProgressBar->hide();
//   QApplication::restoreOverrideCursor();
//   ui.statusBar->clearMessage();
// }

void CMP::plotImageHistograms(const std::vector<int> &imgIdx, 
			      const std::vector<QColor> &colors, int numbins)
{
  // Construct the plot object using the special cmp subclass of QwtPlot
  QwtHistogramDataPlot *plot = new QwtHistogramDataPlot();
  plot->setTitle("CMP Raw Image Histogram Plot");
  plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
  
  float min_value = FLT_MAX; 
  float max_value = FLT_MIN;
  double img_min = DBL_MAX;
  double img_max = DBL_MIN;
  for (unsigned int i = 0; i < imgIdx.size(); i++)
    {
      // Compute histogram and get min & max counts
      float tmp_min_value, tmp_max_value;
      const Image *img = mImageStack->cmpImage(imgIdx[i]);

      CvHistogram *hist = img->computeHistogram(true,numbins);
      
      cvGetMinMaxHistValue( hist, &tmp_min_value, &tmp_max_value);
      
      if (tmp_min_value < min_value) min_value = tmp_min_value;
      if (tmp_max_value > max_value) max_value = tmp_max_value;
      
      // Compute min & max of image values
      double tmp_img_min, tmp_img_max;
      img->minMax(tmp_img_min,tmp_img_max);
      
      if (tmp_img_min < img_min) img_min = tmp_img_min;
      if (tmp_img_max > img_max) img_max = tmp_img_max;
      
      // Fill in bins and edges data
      double *counts = new double[numbins];
      double *edges  = new double[numbins];
      double e  = tmp_img_min;
      double de = (tmp_img_max - tmp_img_min) / (double)numbins;
      for (int j = 0; j < numbins; j++)
        {
        edges[j] = e;//(double) j;
	counts[j] = (double) cvGetReal1D(hist->bins, j);
	  //        counts[j] = (double) cvQueryHistValue_1D(hist,j);
        e += de;
        }
      cvReleaseHist(&hist);
      
      // Insert new curves
      QListWidgetItem *item = ui.imageListWidget->item(imgIdx[i]);

      QwtPlotCurve *histoCounts = new QwtPlotCurve(item->text());

      // QwtSymbol *symb = new QwtSymbol(QwtSymbol::Cross,QBrush(Qt::SolidPattern),QPen(Qt::SolidLine),QSize(5,5));
      // histoCounts->setSymbol(symb);
      histoCounts->setStyle(QwtPlotCurve::Lines);
      histoCounts->setRenderHint(QwtPlotItem::RenderAntialiased);
      histoCounts->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
      histoCounts->setPen(QPen(colors[i]));
      histoCounts->attach(plot);
      histoCounts->setSamples(edges,counts,numbins);

      // Make a deep copy of the data in the plot metadata.  Somewhat redundant
      // since the data is already in the plot curves, but Qwt doesn't provide
      // easy access to the attached plot curves??
      plot->addData(item->text().toStdString(),edges,counts,numbins); 
      
      delete[] counts;
      delete[] edges; 
    }

  // Set axes 
  plot->setAxisTitle(QwtPlot::xBottom, "Pixel Grayscale Intensity");
  plot->setAxisScale(QwtPlot::xBottom, img_min, img_max);
  
  // Set the extent of the axes. Give some headroom on the y-axis.
  plot->setAxisTitle(QwtPlot::yLeft, "Histogram Count");
  plot->setAxisScale(QwtPlot::yLeft, 0, max_value + (0.01*max_value));
 
   // long curveId = plot->insertCurve("Counts");
   // plot->setCurveData(curveId,edges,counts,numbins);
   // plot->setCurvePen(curveId,QPen(blue));
  
   // Make a new subwindow for the plot object. Memory is cleaned up
   // when the user closes this plot subwindow.
   QMdiSubWindow *sub = new QMdiSubWindow();
   sub->setAttribute(Qt::WA_DeleteOnClose);
   sub->setWidget(plot);
   ui.ImageWindowArea->addSubWindow(sub);
   sub->resize(QSize(ui.ImageWindowArea->width() / 2,ui.ImageWindowArea->height() /2));
   sub->show();
   // sub->widget()->setSizeGripEnabled(true);
}

void CMP::plotClusterHistograms(const std::vector<int> &imgIdx,
				int clusterIdx, 
				const std::vector<QColor> &colors, int numbins)
{
  // Construct the plot objecta
   QwtHistogramDataPlot *plot = new QwtHistogramDataPlot();
   //   plot->setTitle("Cluster Histogram Plot");

   plot->setTitle(QString("CMP Histogram Masked by ") + ui.clusterListWidget->item(clusterIdx)->text());
   plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
   
   float min_value = FLT_MAX; 
   float max_value = FLT_MIN;
   double img_min = DBL_MAX;
   double img_max = DBL_MIN;
   for (unsigned int i = 0; i < imgIdx.size(); i++)
     {
       // Compute histogram and get min & max counts
       float tmp_min_value, tmp_max_value;
       const Image *img = mImageStack->cmpImage(imgIdx[i]);
       
       // TODO: histogram of masked image
       CvHistogram *hist = img->computeHistogram(true,numbins, mImageStack->cluster(clusterIdx));
       cvGetMinMaxHistValue( hist, &tmp_min_value, &tmp_max_value);
      
       if (tmp_min_value < min_value) min_value = tmp_min_value;
       if (tmp_max_value > max_value) max_value = tmp_max_value;
      
       // Compute min & max of image values
       // TODO: Min max of masked image
       double tmp_img_min, tmp_img_max;
       img->minMax(tmp_img_min,tmp_img_max);
      
       if (tmp_img_min < img_min) img_min = tmp_img_min;
       if (tmp_img_max > img_max) img_max = tmp_img_max;
      
       // Fill in bins and edges data
       double *counts = new double[numbins];
       double *edges  = new double[numbins];
       double e  = tmp_img_min;
       double de = (tmp_img_max - tmp_img_min) / (double)numbins;
       for (int j = 0; j < numbins; j++)
	 {
   	  edges[j] = e;//(double) j;	  
	  counts[j] = (double) cvGetReal1D(hist->bins, j);
	  // 	  counts[j] = (double) cvQueryHistValue_1D(hist,j);
  	  e += de;
	 }
       cvReleaseHist(&hist);
      
       // Insert new curves
       QListWidgetItem *item = ui.imageListWidget->item(imgIdx[i]);

       QwtPlotCurve *histoCounts = new QwtPlotCurve(item->text());
       histoCounts->setRenderHint(QwtPlotItem::RenderAntialiased);
       histoCounts->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
       histoCounts->setPen(QPen(colors[i]));
       histoCounts->attach(plot);
       histoCounts->setSamples(edges,counts,numbins);
       plot->addData(item->text().toStdString() + std::string(" (")
                     + ui.clusterListWidget->item(clusterIdx)->text().toStdString()
                     + std::string(")"), edges,counts,numbins);
      
       delete[] counts;
       delete[] edges;      
     }

   // Set axes 
   plot->setAxisTitle(QwtPlot::xBottom, "Pixel Grayscale Intensity");
   plot->setAxisScale(QwtPlot::xBottom, img_min, img_max);
   
   // Set the extent of the axes. Give some headroom on the y-axis.
   plot->setAxisTitle(QwtPlot::yLeft, "Histogram Count");
   plot->setAxisScale(QwtPlot::yLeft, 0, max_value + (0.01*max_value));
   
    //  long curveId = plot->insertCurve("Counts");
    // plot->setCurveData(curveId,edges,counts,numbins);
    // plot->setCurvePen(curveId,QPen(blue));
  
    // Make a new subwindow for the plot object. Memory is cleaned up
    // when the user closes this plot subwindow.
    QMdiSubWindow *sub = new QMdiSubWindow();
    sub->setAttribute(Qt::WA_DeleteOnClose);
    sub->setWidget(plot);
    ui.ImageWindowArea->addSubWindow(sub);
    sub->resize(QSize(ui.ImageWindowArea->width() / 2,ui.ImageWindowArea->height() /2));
    sub->show();   
}

void CMP::plotAdvanced2DHistogram()
{
  if ( uiAdvancedPlotDialog.visImageListWidget->count() <=0 )
    {  
      QMessageBox::warning(this, tr("CMP"),
			   tr("Histogram failed because no image data is available to plot."),
			   QMessageBox::Ok);
      return; 
    }
  
  // Adjust the plot dialog layout
  mAdvancedPlotDialog->setWindowTitle("2D Histogram");
  uiAdvancedPlotDialog.yAxisImageCombo->show();
  uiAdvancedPlotDialog.yAxisImageComboLabel->show();
  uiAdvancedPlotDialog.xAxisImageCombo->show();
  uiAdvancedPlotDialog.xAxisImageComboLabel->show();
  uiAdvancedPlotDialog.visImageListWidget->hide();
  uiAdvancedPlotDialog.visImageCheckButton->hide();
  uiAdvancedPlotDialog.visImageUncheckButton->hide();
  uiAdvancedPlotDialog.imageDataLabel1->hide();
  uiAdvancedPlotDialog.imageDataLabel2->hide();
  uiAdvancedPlotDialog.clusterNumberCombo->show();
  uiAdvancedPlotDialog.clusterNumberComboLabel->show();
  uiAdvancedPlotDialog.numBins->hide();
  uiAdvancedPlotDialog.numBinsLabel->hide();
  uiAdvancedPlotDialog.numSamples->show();
  uiAdvancedPlotDialog.numSamplesLabel->show();
  mAdvancedPlotDialog->adjustSize();

  // Get user input from dialog
  if (mAdvancedPlotDialog->exec() == 0) return; // if dialog is rejected, do nothing
  
  // Display the status bar and wait cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing histogram ..."));
  mProgressBar->show();
  
  // Retrieve images
  int img1idx = uiAdvancedPlotDialog.xAxisImageCombo->currentIndex();
  int img2idx = uiAdvancedPlotDialog.yAxisImageCombo->currentIndex();
  const Image *img1 = mImageStack->cmpImage(img1idx);
  const Image *img2 = mImageStack->cmpImage(img2idx);
 
  int numbins = uiAdvancedPlotDialog.numBins->value();

  float min_value = FLT_MAX; 
  float max_value = FLT_MIN;
  double img_min = DBL_MAX;
  double img_max = DBL_MIN;

  // Compute 2D histogram
  const Cluster *mask = (uiAdvancedPlotDialog.clusterNumberCombo->currentText() == mNullMaskString) ? NULL :
    mImageStack->cluster(uiAdvancedPlotDialog.clusterNumberCombo->currentIndex());
  CvHistogram   *hist = img1->compute2DHistogram(true,numbins,img2,mask);
  cvGetMinMaxHistValue( hist, &min_value, &max_value );

 // Compute min & max of image values
  double tmp_img_min, tmp_img_max;
  img1->minMax(tmp_img_min,tmp_img_max);
  img2->minMax(img_min,img_max);
  if (tmp_img_min < img_min) img_min = tmp_img_min;
  if (tmp_img_max > img_max) img_max = tmp_img_max;

  QwtHistogram2DDataPlot *plot = new QwtHistogram2DDataPlot();
  plot->setHistogramData(uiAdvancedPlotDialog.xAxisImageCombo->currentText().toStdString(),
                         uiAdvancedPlotDialog.yAxisImageCombo->currentText().toStdString(),
                         hist,numbins,img_min,img_max);
  plot->setAxisTitle(QwtPlot::xBottom,uiAdvancedPlotDialog.xAxisImageCombo->currentText());
  plot->setAxisTitle(QwtPlot::yLeft,  uiAdvancedPlotDialog.yAxisImageCombo->currentText());
  plot->setTitle("CMP 2D Histogram Plot");

  QMdiSubWindow *sub = new QMdiSubWindow();
  sub->setAttribute(Qt::WA_DeleteOnClose);
  sub->setWidget(plot);
  ui.ImageWindowArea->addSubWindow(sub);
  sub->resize(QSize(ui.ImageWindowArea->width() / 2,ui.ImageWindowArea->height() /2));
  sub->show();
  
  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage();
}

void CMP::plotQuick2DHistogram()
{
  std::vector<int> imgList;
  
  QList<QListWidgetItem *> items = ui.imageListWidget->selectedItems();
  if (items.size() != 2)
    {
    QMessageBox::warning(this, tr("CMP"),
                         tr("Please select 2 images in the file manager window."),
                         QMessageBox::Ok);        
    return;
    }
  
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing 2D histograms ..."));
  mProgressBar->show();
 
  int numbins = uiPrefs.plotPrefsNumBins->value();

  // Retrieve images
  int img1idx = ui.imageListWidget->row(items.at(0));
  int img2idx = ui.imageListWidget->row(items.at(1));
  const Image *img1 = mImageStack->cmpImage(img1idx);
  const Image *img2 = mImageStack->cmpImage(img2idx);

  float min_value = FLT_MAX; 
  float max_value = FLT_MIN;
  double img_min = DBL_MAX;
  double img_max = DBL_MIN;

  // Compute 2D histogram -- no mask (NULL)
  CvHistogram *hist = img1->compute2DHistogram(true,numbins,img2,NULL);
  cvGetMinMaxHistValue( hist, &min_value, &max_value);

 // Compute min & max of image values
  double tmp_img_min, tmp_img_max;
  img1->minMax(tmp_img_min,tmp_img_max);
  img2->minMax(img_min,img_max);
  if (tmp_img_min < img_min) img_min = tmp_img_min;
  if (tmp_img_max > img_max) img_max = tmp_img_max;

  std::cout << "Image min = " << img_min << std::endl;
  std::cout << "Image max = " << img_max << std::endl;

  std::cout << "Hist min = " << min_value << std::endl;
  std::cout << "Hist max = " << max_value << std::endl;

  QwtHistogram2DDataPlot *plot = new QwtHistogram2DDataPlot();
  plot->setHistogramData(items.at(0)->text().toStdString(),
                         items.at(1)->text().toStdString(),
                         hist,numbins,img_min,img_max);
  plot->setAxisTitle(QwtPlot::xBottom, items.at(0)->text());
  plot->setAxisTitle(QwtPlot::yLeft, items.at(1)->text());
  plot->setTitle("CMP 2D Histogram Plot");

  QMdiSubWindow *sub = new QMdiSubWindow();
  sub->setAttribute(Qt::WA_DeleteOnClose);
  sub->setWidget(plot);
  ui.ImageWindowArea->addSubWindow(sub);
  sub->resize(QSize(ui.ImageWindowArea->width() / 2,ui.ImageWindowArea->height() /2));
  sub->show();
  
  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage();
}

void CMP::plotQuickHistogram()
{
  std::vector<int> imgList;
  std::vector<QColor> colList;
  QList<QListWidgetItem *> items = ui.imageListWidget->selectedItems();

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing visualizations..."));
  mProgressBar->show();
  
  CvRNG rng_l(time(NULL));

  ///  int numbins = g_settings->value("plots/quick_histogram/number_of_bins").toInt();
  int numbins = uiPrefs.plotPrefsNumBins->value();

  for (int i = items.size()-1; i >=0; i--)
    {
    QListWidgetItem *item = items.at(i);
    imgList.push_back(ui.imageListWidget->row(item));
    if (i < g_settings->numberOfColors() )
      {
	  colList.push_back(g_settings->color(i));
      }
      else colList.push_back(QColor(cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255));
    }
  
  this->plotImageHistograms(imgList,colList,numbins);
  
  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage();  
}

void CMP::plotRGBHistogram()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing visualizations..."));
  mProgressBar->show();
  
  std::vector<int> imgList;
  std::vector<QColor> colList;
  int numbins = uiPrefs.plotPrefsNumBins->value();//g_settings->value("plots/quick_histogram/number_of_bins").toInt();

  // Get image numbers
  bool found = false;
  if (mRGBIndices[0] >=0) // Red
    {
      colList.push_back(QColor("red"));
      imgList.push_back(mRGBIndices[0]);
      found = true;
    }
  if (mRGBIndices[1] >=0) // Green
    {
      colList.push_back(QColor("green"));
      imgList.push_back(mRGBIndices[1]);
      found = true;
    }
  if (mRGBIndices[2] >=0) // Green
    {
      colList.push_back(QColor("blue"));
      imgList.push_back(mRGBIndices[2]);
      found = true;
    }
  if (!found) return; // no RGB
  else // plot RGB histos
    {
      this->plotImageHistograms(imgList,colList,numbins);
    }

  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage();  
}

void CMP::plotAdvancedHistogram()
{
  if ( (uiAdvancedPlotDialog.clusterNumberCombo->count() <=0) || 
       (uiAdvancedPlotDialog.visImageListWidget->count() <=0) )
    {  
      QMessageBox::warning(this, tr("CMP"),
			   tr("Histogram plot failed because no image clusters are available to plot."),
			   QMessageBox::Ok);
      return; 
    }
  
  // Adjust the plot dialog layout
  mAdvancedPlotDialog->setWindowTitle("Histogram Plot");
  uiAdvancedPlotDialog.yAxisImageCombo->hide();
  uiAdvancedPlotDialog.yAxisImageComboLabel->hide();
  uiAdvancedPlotDialog.xAxisImageCombo->hide();
  uiAdvancedPlotDialog.xAxisImageComboLabel->hide();
  uiAdvancedPlotDialog.visImageListWidget->show();
  uiAdvancedPlotDialog.visImageCheckButton->show();
  uiAdvancedPlotDialog.visImageUncheckButton->show();
  uiAdvancedPlotDialog.imageDataLabel1->show();
  uiAdvancedPlotDialog.imageDataLabel2->hide();
  uiAdvancedPlotDialog.clusterNumberCombo->show();
  uiAdvancedPlotDialog.clusterNumberComboLabel->show();
  uiAdvancedPlotDialog.numBins->show();
  uiAdvancedPlotDialog.numBinsLabel->show();
  uiAdvancedPlotDialog.numSamples->hide();
  uiAdvancedPlotDialog.numSamplesLabel->hide();
  mAdvancedPlotDialog->adjustSize();

  // Get user input from dialog
  if (mAdvancedPlotDialog->exec() == 0) return; // if dialog is rejected, do nothing
  
  // Display the status bar and wait cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui.statusBar->showMessage(tr("Computing visualizations..."));
  mProgressBar->show();

  CvRNG rng_l(time(NULL));

  std::vector<QColor> colList;
  int numbins = uiAdvancedPlotDialog.numBins->value();

  // Compile list of images and clusters  
  std::vector<int> imageIds;
  
  for (int i = 0; i < uiAdvancedPlotDialog.visImageListWidget->count(); i++)
    {
    if (uiAdvancedPlotDialog.visImageListWidget->item(i)->checkState()==Qt::Checked)
      {
	imageIds.push_back(i);
      }
    if (i < g_settings->numberOfColors() )
      {
	colList.push_back(g_settings->color(i));
      }
    else colList.push_back(QColor(cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255,cvRandInt(&rng_l)%255));
    }

  if (imageIds.size() > 0)
    {
      if (uiAdvancedPlotDialog.clusterNumberCombo->currentText() == mNullMaskString)
	{
	  // No mask is selected 
	  this->plotImageHistograms(imageIds,colList,numbins);       
	}
      else
	{	  
	  this->plotClusterHistograms(imageIds,uiAdvancedPlotDialog.clusterNumberCombo->currentIndex(),colList,numbins);
	}
    }
  else
    {
      QMessageBox::warning(this, tr("CMP"),
			   tr("Please select one or more images to plot."),
			   QMessageBox::Ok);
    }

  mProgressBar->hide();
  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage();  
}

} //end namespace cmp
