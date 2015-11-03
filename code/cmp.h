//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#ifndef CMP_H
#define CMP_H

#include <vector>
#include <iostream>
#include <cmath>

// CMP includes
#include "cmpException.h"
#include "cmpImage.h"
#include "cmpUtils.h"
#include "cmpNumerics.h"

// Qt includes
// For higher warning levels, Qt generates many thousands of warnings.  This is
// an attempt to quell some of those.
#ifdef _WIN32
#pragma warning(disable : 4251 4244 4800)
#pragma warning(push,3)
#endif

#include <QMainWindow>
#include <QProgressBar>
#include <QMdiSubWindow>
#include <QSettings>
#include <QDebug>

#include "ui_cmp2.h"
#include "ui_cmpPreferences.h"
#include "ui_cmpAdvancedPlotDialog.h"

// Custom cmp Qt includes
#include "cmpPreferences.h"
#include "cmpColorPaletteWidget.h"
#include "cmpQtPairPlotDialog.h"

// Qwt Includes
#define QWT_DLL

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_legend.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include <qwt_math.h>
#include "qwt_symbol.h"

// cmp Custom Qwt Includes
#include "cmpQwtEllipsePlotItem.h"
#include "cmpQwtDataPlot.h"

#ifdef _WIN32
#pragma warning(pop)
#endif

namespace cmp {

/** \brief QApplication class for CMP. */
class CMPApplication : public QApplication
{
public:
  CMPApplication(int argc, char **argv) : QApplication(argc, argv) {}
  ~CMPApplication() {}
  
  void loadStyleSheet(const char *fn);
  
  bool notify(QObject *receiver, QEvent *e)
  {
    try
      {
      return QApplication::notify(receiver, e);
      }
    catch(Exception &ex)
      {
      qDebug() << "CMP caught an exception from " << receiver->objectName() 
               << " from event type " << e->type()
               << ". Exception reports the following information: "
               << ex;
      qFatal("Exiting due to exception caught");
      return false;
      }
    catch(...)
      {
      qDebug() << "CMP caught an exception from " << receiver->objectName() 
               << " from event type " << e->type();
      qFatal("Exiting due to exception caught");
      return false;
      }
  }
};


/** \brief The main class for the CMP application that sets up the GUI and
    signals and slots. */
class CMP : public QMainWindow
{
  Q_OBJECT
  
public:
  CMP(QWidget *parent = 0, Qt::WindowFlags = 0);
  ~CMP();
  
  void init();
  void closeEvent(QCloseEvent *event);
  
  void updateImageDisplay();
  void updateRGBImage(unsigned char);
  void updateImageListIcons();
  void updateRGBImageDisplay();
  void updateImageFilterDisplay();
  void updateClusterDisplay();

  /** Load an image from a file */
  bool addImageFromFile(QString fname);
  
  /** Instance of a custom QPreference object for storing application
      variables that are persistent across sessions. */
  static Preferences *g_settings;
              
  //  Ui::CMPClass &getUi() { return ui; }
  //  Ui::preferencesWindow &getUiPrefs() { return uiPrefs; }
                
public slots:
  /** Compute and draw a "pawprint" plot for two or more selected
      images.  A pawprint plot consists of a simple scatter plot of
      two images, with the oriented multivariate Gaussian superimposed
      over the distribution.  */
  void plotQuickPawprint();
  void plotAdvancedPawprint();
  
  /** Compute and plot the histograms from all selected images */
  void plotQuickHistogram();

  /** Compute and draw a scatterplot of two selected images */
  void plotQuickScatterplot();

  /** Compute and draw a scatterplot of multiple images.  This method will show
  the advanced plot dialog to acquire the plot parameters.  One image is
  plotted against all other selected images in the same coordinate frame. */
  void plotAdvancedScatterplot();

  /** Compute and plot the 2D histogram (joint histogram) of two selected images.*/
  void plotQuick2DHistogram();

  /** Compute and plot the histograms for all R G B selected images. */
  void plotRGBHistogram();
  
  /** TODO: Document*/
  void plotAdvancedHistogram();

  void plotAdvanced2DHistogram();

  /** TODO: Document*/
  void exportThemeMap();

  /** TODO: Document*/
  void importPage(bool);
  
  /** TODO: Document*/
  void prefilterPage(bool);

  /** TODO: Document*/
  void clusterPage(bool);
  
  /** TODO: Document*/
  void updateVisualizeOptions();

  /** TODO: Document*/
  void importSelectionChanged();

  /** TODO: Document*/
  void importAdd();

  /** TODO: Document*/
  void importDelete();
  
  /** TODO: Document*/
  void smoothImages();

  /** TODO: Document*/
  void resetSmoothImages();
  
  /** TODO: Document*/
  void computeClusters();

  /** TODO: Document*/
  void clusterListItemChanged(QListWidgetItem *item);

  /** TODO: Document*/
  void clusterPicked(int x, int y);

  /** TODO: Document*/
  void imageDropped(QString);

  /** TODO: Document*/
  void exportListItemChanged(QListWidgetItem *item);

  /** TODO: Document*/
  void exportThemeMapImage();

  /** Check all items in the advanced plot dialog cluster list.*/
  void clusterCheckAll();

  /** Uncheck all items in the advanced plot dialog cluster list.*/
  void clusterUncheckAll();
  
  /** Check all items in the advanced plot dialog image list.*/
  void visClassCheckAll();

  /** Check all items in the advanced plot dialog image list.*/
  void visClassUncheckAll();

  /** TODO: Document*/
  void visImageCheckAll();
  
  /** TODO: Document*/
  void visImageUncheckAll();

  /** Check all items in the export dialog class list. */
  void exportCheckAll();

  /** Uncheck all items in the export dialog class list.*/
  void exportUncheckAll();

  /** Set the background color of the MDI window. */
  void setBackgroundColor();

  /** Set the background color for the clustered image */
  void setClusterBackgroundColor();

  void updateProgress(int p);
  void setRedImage()   { this->updateRGBImage(cmpRED);   }
  void setGreenImage() { this->updateRGBImage(cmpGREEN); }
  void setBlueImage()  { this->updateRGBImage(cmpBLUE);  }
  void hideSelectedWindow()  { ui.ImageWindowArea->activeSubWindow()->hide(); }
  void resetSelectedWindow() 
  {
    ui.ImageWindowArea->activeSubWindow()->showNormal();
    ui.ImageWindowArea->activeSubWindow()->resize(mImageStack->selectedImage(true).size()); 
  }
  
  /** Saves the currently-selected image window to disk. */
  void saveWindowToDisk();

  /** Saves the data from the plots as a .csv text file on disk. */
  void savePlotDataToDisk();

  /** Displays a message indicating that this feature is not yet implemented. */
  void unimplementedFeature()
  {  
    QMessageBox::warning(this, tr("CMP"),
                         tr("This feature is coming soon!"),
                         QMessageBox::Ok);
  }
  
private:
  /** Initializes all settings.  Meant to be run once on startup.*/
  void readSettings();

  /** Saves any settings at the end of a session.  This is meant to be
      run once when shutting down the application. */
  void writeSettings();
  
  /** Construct the toolbar. These are the buttons at the top of the main window. */
  void setupToolbar();

  /** Construct the preferences window. */
  void setupPreferencesWindow();
  
  /** Fill in options in a pair plot dialog: possible clusters and image
      combinations. */
  void setupPairPlotOptions(QtPairPlotDialog *) const;
  
  /** Construct the advanced plot dialog, connect events, read default values, etc */
  void setupAdvancedPlotDialog();

  /** Sets up the dialog that is displayed when saving a theme map. */
  void setupSaveThemeMapDialog();

  /** Sets up the connections for saving values from specific CMP
      preference window gui elements. */
  void connectPreferences();

  /** Construct the main menu.  These are the actions accessible through the
      main application menu. */
  void setupMenu();
  
  /** Construct the file menu and connections. */
  void setupFileWindow();
  
  /** Setup the Mdi area.  Generally this should only be done once. */
  void setupMdiArea();

  /** The main user interface layout class. */
  Ui::CMPClass ui;

  /** The preference window layout class. */
  Ui::cmpPreferencesWindow uiPrefs;

  /** The advanced plot dialog layout class. */
  Ui::cmpAdvancedPlotDialog uiAdvancedPlotDialog;

  /** String describing the condition where no mask has been selected. */
  QString mNullMaskString;

  /** String describing the condition where no image has been selected. */  
  QString mNullImageString;
  
  /** Instantiation of the prefWindowUi layout. */
  QDialog *mPreferencesWindow;

  /** Instantiation of the plot dialog for advanced plotting options (scatter, pawprint) */
  QDialog *mAdvancedPlotDialog;

  /** Maps the file list entries to the RGB channels of the RGB display. */
  int mRGBIndices[3];

  /** */
  //  void plotImagePawprints(const std::vector<int> &,
  //                           const std::vector<QColor> &, int numsamples,
  //                           int clusterId = -1);
  void plotImagePawprints(const std::vector<std::pair<int,int> > &,
                             const std::vector<QColor> &, int numsamples,
                             int clusterId = -1);


  /** Plots a random sampling of values from one image against values from
      multiple other images.  First list in the image will be used as the
      y-image value.  Numsamples is the number of samples from each image to
      use when computing scatterplots. The cluster id, if present, indicates
      which cluster to use as a mask for sampling the data. */
  //  void plotImageScatterplots(const std::vector<int> &,
  //                             const std::vector<QColor> &, int numsamples,
  //                            int clusterId = -1);
  void plotImageScatterplots(const std::vector<std::pair<int,int> > &,
                             const std::vector<QColor> &, int numsamples,
                             int clusterId = -1);

  /** Plots histograms of the given list of images (indices of the
      mImage stack), with the colors indicated in the second
      argument. */
  void plotImageHistograms(const std::vector<int> &, 
			   const std::vector<QColor> &, int numbins);

  /** Plots histograms of pixels in the masked areas in the given list
      of images (indices of the mImage stack), with the colors
      indicates in list. */
  void plotClusterHistograms(const std::vector<int> &, int,
			     const std::vector<QColor> &, int numbins);

  /** A modal dialog that is shown for input during exporting theme maps. */
  QDialog *mExportThemeMapDialog;
  
  /** One of the actions in the toolbar and/or main menu */
  QAction *mTileWindowsAction;
  QAction *mCascadeWindowsAction;
  QAction *mImportAction;
  QAction *mPrefilterAction;
  QAction *mClusterAction;
  QAction *mImportImageAction;
  QAction *mExportColormapAction;
  QAction *mSaveWindowImageAction;
  QAction *mSaveThemeMapAction;
  QAction *mSavePlotDataAction;
  QAction *mHideWindowAction;
  QAction *mResetWindowSizeAction;
  
  /** Actions for producing plots*/
  QAction *mPlotQuickHistogramAction;
  QAction *mPlotQuickScatterplotAction;
  QAction *mPlotRGBHistogramAction;
  QAction *mPlotQuick2DHistogramAction;
  QAction *mPlotQuickPawprintAction;
  QAction *mPlotAdvancedHistogramAction;
  QAction *mPlotAdvanced2DHistogramAction;
  QAction *mPlotAdvancedScatterplotAction;
  QAction *mPlotAdvancedPawprintAction;
  
  /** The main progress bar */  
  QProgressBar *mProgressBar;

  /** The stack of images that has been loaded for display and processing. */
  ImageStack *mImageStack;

  /** Color palettes used in the preferences dialog. */
  ColorPaletteWidget *mColorPaletteWidget;
  ColorPaletteWidget *mClusterColorPaletteWidget;
  
  /** Flags to mark images as needing updates */
  bool mClusterDirty;
  bool mVisualizeDirty;
  bool mExportDirty;

  /** Variable currently not used.  Originally planned to be used to boost the
      size of fonts of selected values in lists. */
  int mFontBoostSize;
};

} // end namespace cmp

#endif // CMP_H
