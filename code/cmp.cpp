//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#include "cmp.h"

namespace cmp {

Preferences *CMP::g_settings;

void CMPApplication::loadStyleSheet(const char *fn)
{
  QString sheetName(fn);
  QFile file(fn);
  file.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(file.readAll());

  qApp->setStyleSheet(styleSheet);
}


CMP::CMP(QWidget *parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags),
    mImageStack(NULL),
    mClusterDirty(true),
    mVisualizeDirty(true),
    mExportDirty(true)
{
  mNullMaskString = "NO CLUSTER MASK";
  mNullImageString = "";
  mRGBIndices[0] = -1;
  mRGBIndices[1] = -1;
  mRGBIndices[2] = -1;
  mFontBoostSize =  1;
  
  init();
}

CMP::~CMP()
{
  if (mImageStack)
    { delete mImageStack; }
}

void CMP::setupToolbar()
{
// Toolbar
  mImportAction = new QAction(QIcon(":/CMP/Resources/import.png"), tr("&File Manager"), this);
  mImportAction->setToolTip(tr("File Manager"));
  mImportAction->setStatusTip(tr("Import registered images"));
  mImportAction->setCheckable(true);
  connect(mImportAction, SIGNAL(toggled(bool)), this, SLOT(importPage(bool)));
  
  mPrefilterAction = new QAction(QIcon(":/CMP/Resources/prefilter.png"), tr("&Image Filters"), this);
  mPrefilterAction->setToolTip(tr("Image Filters"));
  mPrefilterAction->setStatusTip(tr("PreFilter image stack"));
  mPrefilterAction->setCheckable(true);
  //  mPrefilterAction->setToggleAction(true);
  connect(mPrefilterAction, SIGNAL(toggled(bool)), this, SLOT(prefilterPage(bool)));
    
  mClusterAction = new QAction(QIcon(":/CMP/Resources/cluster.png"), tr("&Cluster Data"), this);
  mClusterAction->setToolTip(tr("Data Clustering"));
  mClusterAction->setStatusTip(tr("Cluster image stack"));
  mClusterAction->setCheckable(true);
  //  mClusterAction->setToggleAction(true);
  connect(mClusterAction, SIGNAL(toggled(bool)), this, SLOT(clusterPage(bool)));
  
  mImportAction->setEnabled(true);
  mPrefilterAction->setEnabled(true);
  mClusterAction->setEnabled(true);

  mImportAction->setChecked(true);
  mPrefilterAction->setChecked(true);
  mClusterAction->setChecked(true);
  
  this->importPage(true);
  this->prefilterPage(true);
  this->clusterPage(true);
  
  // ui.mainToolBar->addAction(mImportAction);
  // ui.mainToolBar->addAction(mPrefilterAction);
  // ui.mainToolBar->addAction(mClusterAction);
  // ui.mainToolBar->show();
}

void CMP::setupMenu()
{
  // File Menu
  QMenu *fileMenu = ui.menuBar->addMenu(tr("&File"));
  
  mImportImageAction = new QAction(tr("Load Images"), this);
  mImportImageAction->setStatusTip(tr("Load images from file"));
  mExportColormapAction = new QAction(tr("Export Colormap"), this);
  mExportColormapAction->setStatusTip(tr("Export selected colormaps"));
  mExportColormapAction->setEnabled(false);

  mSaveWindowImageAction = new QAction(tr("Save Image"), this);  
  mSaveWindowImageAction->setStatusTip(tr("Save the image in the currently-selected window to disk."));
  connect(mSaveWindowImageAction, SIGNAL(triggered()), this, SLOT(saveWindowToDisk()));

  mSaveThemeMapAction = new QAction(tr("Save Theme Map"), this);  
  mSaveThemeMapAction->setStatusTip(tr("Save a theme map to disk."));
  //connect(mSaveThemeMapAction, SIGNAL(triggered()), this,  SLOT(saveThemeMapToDisk()));
  connect(mSaveThemeMapAction, SIGNAL(triggered()), this, SLOT(exportThemeMap()));

  mSavePlotDataAction = new QAction(tr("Save Plot Data"), this);
  mSavePlotDataAction->setStatusTip(tr("Save plot data to disk."));
  connect(mSavePlotDataAction, SIGNAL(triggered()), this, SLOT(savePlotDataToDisk()));

  QAction *prefAction = new QAction(tr("&Preferences..."),this);
  prefAction->setStatusTip(tr("Set program preferences"));
  connect(prefAction,SIGNAL(triggered()), mPreferencesWindow,SLOT(exec()));
  
  QAction *exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+Q"));
  exitAction->setStatusTip(tr("Exit the application"));
  connect(mImportImageAction, SIGNAL(triggered()), this, SLOT(importAdd()));
  connect(mExportColormapAction, SIGNAL(triggered()), this, SLOT(exportThemeMap()));
  connect(exitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

  fileMenu->addAction(mImportImageAction);
  fileMenu->addAction(mSaveWindowImageAction);
  fileMenu->addAction(mSaveThemeMapAction);
  fileMenu->addAction(mSavePlotDataAction);
  fileMenu->addAction(prefAction);
  fileMenu->addAction(exitAction);

// Data Plotting Menu
  QMenu *plotMenu = ui.menuBar->addMenu(tr("&Plot"));
  
  mPlotQuickHistogramAction = new QAction(tr("Quick Histogram"),this);
  mPlotQuickHistogramAction->setStatusTip(tr("Plot histograms of the selected images"));
  connect(mPlotQuickHistogramAction, SIGNAL(triggered()),this,SLOT(plotQuickHistogram()));

  mPlotQuick2DHistogramAction = new QAction(tr("Quick 2D Histogram"),this);
  mPlotQuick2DHistogramAction->setStatusTip(tr("Plot 2D histogram of two selected images"));
  connect(mPlotQuick2DHistogramAction, SIGNAL(triggered()),this,SLOT(plotQuick2DHistogram()));

  mPlotQuickScatterplotAction = new QAction(tr("Quick Scatter Plot"),this);
  mPlotQuickScatterplotAction->setStatusTip(tr("Scatter plot of two selected images"));
  connect(mPlotQuickScatterplotAction, SIGNAL(triggered()),this,SLOT(plotQuickScatterplot()));
  
  mPlotQuickPawprintAction = new QAction(tr("Quick Pawprint Plot"),this);
  mPlotQuickPawprintAction->setStatusTip(tr("Quick pawprint plot of two selected images"));
  connect(mPlotQuickPawprintAction, SIGNAL(triggered()),this,SLOT(plotQuickPawprint()));

  mPlotRGBHistogramAction = new QAction(tr("Quick RGB Histogram"),this);
  mPlotRGBHistogramAction->setStatusTip(tr("Plot histograms of RGB Images"));
  connect(mPlotRGBHistogramAction, SIGNAL(triggered()),this,SLOT(plotRGBHistogram()));

  mPlotAdvancedHistogramAction = new QAction(tr("&Histogram Plot ..."),this);
  mPlotAdvancedHistogramAction->setStatusTip(tr("Advanced plot of the histograms of intensity values, optionally masked by clusters"));
  connect(mPlotAdvancedHistogramAction, SIGNAL(triggered()),this,SLOT(plotAdvancedHistogram()));

  mPlotAdvanced2DHistogramAction = new QAction(tr("&2D Histogram Plot ..."),this);
  mPlotAdvanced2DHistogramAction->setStatusTip(tr("Advanced plot of the 2D histogram of intensity values, optionally masked by a cluster"));
  connect(mPlotAdvanced2DHistogramAction, SIGNAL(triggered()),this,SLOT(plotAdvanced2DHistogram()));
  
  mPlotAdvancedScatterplotAction = new QAction(tr("&Scatter Plot ..."),this);
  mPlotAdvancedScatterplotAction->setStatusTip(tr("Scatter plot multiple images"));
  connect(mPlotAdvancedScatterplotAction,SIGNAL(triggered()), this, SLOT(plotAdvancedScatterplot()));

  mPlotAdvancedPawprintAction = new QAction(tr("&Pawprint Plot ..."),this);
  mPlotAdvancedPawprintAction->setStatusTip(tr("Pawprint plot of multiple images"));
  connect(mPlotAdvancedPawprintAction,SIGNAL(triggered()), this, SLOT(plotAdvancedPawprint()));

  // Layout menu and separators
  plotMenu->setTearOffEnabled(true);
  plotMenu->setSeparatorsCollapsible(true);
  
  plotMenu->addAction(mPlotRGBHistogramAction);
  plotMenu->addAction(mPlotQuickHistogramAction);
  plotMenu->addAction(mPlotQuick2DHistogramAction);
  plotMenu->addAction(mPlotQuickScatterplotAction);
  plotMenu->addAction(mPlotQuickPawprintAction);
  plotMenu->addSeparator();
  plotMenu->addAction(mPlotAdvancedHistogramAction);
  plotMenu->addAction(mPlotAdvanced2DHistogramAction);
  plotMenu->addAction(mPlotAdvancedScatterplotAction);
  plotMenu->addAction(mPlotAdvancedPawprintAction);

  // View Menu
  QMenu *viewMenu = ui.menuBar->addMenu(tr("&View"));
  
  mTileWindowsAction = new QAction(tr("Tile Windows"),this);
  mTileWindowsAction->setStatusTip(tr("Tile windows in viewer."));
  connect(mTileWindowsAction, SIGNAL(triggered()),ui.ImageWindowArea,SLOT(tileSubWindows()));

  // mHideWindowAction = new QAction(tr("Hide Window"),this);
  // mHideWindowAction->setStatusTip(tr("Hide selected image window."));
  // connect(mHideWindowAction, SIGNAL(triggered()),this,SLOT(hideSelectedWindow()));

  mCascadeWindowsAction = new QAction(tr("Cascade Windows"),this);
  mCascadeWindowsAction->setStatusTip(tr("Cascade windows in viewer."));
  connect(mCascadeWindowsAction, SIGNAL(triggered()),ui.ImageWindowArea,SLOT(cascadeSubWindows()));
  
  mResetWindowSizeAction = new QAction(tr("Actual Size"),this);
  mResetWindowSizeAction->setStatusTip(tr("Resets the window to the actual size of the image."));
  connect(mResetWindowSizeAction, SIGNAL(triggered()),this,SLOT(resetSelectedWindow()));
  
  viewMenu->addAction(mTileWindowsAction);
  viewMenu->addAction(mCascadeWindowsAction);
  //  viewMenu->addAction(mHideWindowAction);
  viewMenu->addAction(mResetWindowSizeAction);
  
  // Add tools to main menu 
  //  QMenu *toolsMenu = ui.menuBar->addMenu(tr("&Tool Windows"));
  viewMenu->addSeparator();
  viewMenu->addAction(mImportAction);
  viewMenu->addAction(mPrefilterAction);
  viewMenu->addAction(mClusterAction);
  //  viewMenu->addAction(mVisualizeAction);
  //  viewMenu->addAction(mExportAction); 
}

void CMP::setupFileWindow()
{
  ui.deleteButton->setEnabled(false);
  ui.imageListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui.imageListWidget->setDragEnabled(true);
  ui.imageListWidget->setAcceptDrops(true);
  ui.imageListWidget->setDropIndicatorShown(true);
  ui.imageListWidget->setDragDropMode(QAbstractItemView::DropOnly);
  ui.imageListWidget->setStatusTip(tr("Select the '+' icon or drag and drop images"));
  
  connect(ui.imageListWidget, SIGNAL(itemSelectionChanged()),
          this, SLOT(importSelectionChanged()));
  connect(ui.addButton, SIGNAL(released()),
          this, SLOT(importAdd()));
  connect(ui.deleteButton, SIGNAL(released()),
          this, SLOT(importDelete()));
  connect(ui.redButton, SIGNAL(released()),
          this, SLOT(setRedImage()));
  connect(ui.greenButton, SIGNAL(released()),
          this, SLOT(setGreenImage()));
  connect(ui.blueButton, SIGNAL(released()),
          this, SLOT(setBlueImage()));
  
  connect(ui.imageDisplay->scene(), SIGNAL(imageDropped(QString)),
          this, SLOT(imageDropped(QString)));
  connect(ui.imageListWidget, SIGNAL(imageDropped(QString)),
          this, SLOT(imageDropped(QString))); 
}


void CMP::setupPreferencesWindow()
{
  // Setup the preferences widget
  mPreferencesWindow = new QDialog;
  uiPrefs.setupUi(mPreferencesWindow);

  // NOTE: Color palette size is hard-coded to match cmpPreferences number of colors
  mColorPaletteWidget = new ColorPaletteWidget(0,10,2);
  uiPrefs.colorsFrame->setWidget(mColorPaletteWidget);
  mColorPaletteWidget->setMaximumSize(mColorPaletteWidget->sizeHint());
  mColorPaletteWidget->resize(mColorPaletteWidget->sizeHint());
  uiPrefs.colorsFrame->setMaximumSize(mColorPaletteWidget->sizeHint());
  uiPrefs.colorsFrame->resize(mColorPaletteWidget->sizeHint());

  mClusterColorPaletteWidget = new ColorPaletteWidget(0,10,2);
  uiPrefs.clusterColorsFrame->setWidget(mClusterColorPaletteWidget);
  mClusterColorPaletteWidget->setMaximumSize(mClusterColorPaletteWidget->sizeHint());
  mClusterColorPaletteWidget->resize(mClusterColorPaletteWidget->sizeHint());
  uiPrefs.clusterColorsFrame->setMaximumSize(mClusterColorPaletteWidget->sizeHint());
  uiPrefs.clusterColorsFrame->resize(mClusterColorPaletteWidget->sizeHint());

  // Button to set the background color
  uiPrefs.backgroundColorButton->resize(50,50);
  uiPrefs.clusterBackgroundColorButton->resize(50,50);

  mPreferencesWindow->resize(uiPrefs.preferencesTab->sizeHint());
  mPreferencesWindow->setModal(true);
  mPreferencesWindow->layout()->setSizeConstraint(QLayout::SetFixedSize);
  mPreferencesWindow->setSizeGripEnabled(false);

  mPreferencesWindow->adjustSize();
}

void CMP::setupMdiArea()
{
  // Close all windows
  QList<QMdiSubWindow *> l = ui.ImageWindowArea->subWindowList();
  for ( int i = 0; i < l.size(); i++)
    {
    //    MdiSubWindowMap[l[i], l[i]
    l[i]->setWindowIcon(QIcon(":/CMP/Resources/import.png"));
    l[i]->setAttribute(Qt::WA_DeleteOnClose,false); // prevent widget from
                                                    // deleting when closed
    
    //    l[i]->setWindowFlags(Qt::Window|Qt::CustomizeWindowHint);

    l[i]->hide();
    }
  
  // Setup the background
  //  ui.ImageWindowArea->setBackground(QBrush(QColor(50,50,50)));
}

void CMP::setupAdvancedPlotDialog()
{
 // Set up the various persistent dialogs
  mAdvancedPlotDialog = new QDialog;
  uiAdvancedPlotDialog.setupUi(mAdvancedPlotDialog);
  mAdvancedPlotDialog->resize(uiPrefs.preferencesTab->sizeHint());
  //  mAdvancedPlotDialog->clusterLayo->hide();
  mAdvancedPlotDialog->setModal(true);
  mAdvancedPlotDialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
  mAdvancedPlotDialog->setSizeGripEnabled(false);
  uiAdvancedPlotDialog.visImageListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  
  // Provide some default plot settings if they are not present
  if (g_settings->value("plots/advanced_histogram/number_of_bins").isNull())
    {
      g_settings->setValue("plots/advanced_histogram/number_of_bins", 100);
    }
  if (g_settings->value("plots/advanced_scatterplot/number_of_samples").isNull())
    {
      g_settings->setValue("plots/advanced_scatterplot/number_of_samples", 200);
    }
  uiAdvancedPlotDialog.numBins->setValue( g_settings->numberOfBins());
  uiAdvancedPlotDialog.numSamples->setValue( g_settings->numberOfSamples());

  connect(uiAdvancedPlotDialog.visImageCheckButton, SIGNAL(released()),
          this, SLOT(visImageCheckAll()));
  connect(uiAdvancedPlotDialog.visImageUncheckButton, SIGNAL(released()),
	  this, SLOT(visImageUncheckAll()));
}

void CMP::setupSaveThemeMapDialog()
{
  // Set up the theme map export dialog
  mExportThemeMapDialog = new QDialog;
  ui.exportDialog->setParent(mExportThemeMapDialog);
  mExportThemeMapDialog->setModal(true);
  mExportThemeMapDialog->setWindowTitle("Save Theme Map");
  connect(ui.exportCancelButton,SIGNAL(released()),mExportThemeMapDialog,SLOT(close()));

  ui.exportClassListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  
  connect(ui.exportCheckButton, SIGNAL(released()),
          this, SLOT(exportCheckAll()));
  connect(ui.exportUncheckButton, SIGNAL(released()),
          this, SLOT(exportUncheckAll()));
  connect(ui.exportClassListWidget, SIGNAL(itemChanged(QListWidgetItem *)),
          this, SLOT(exportListItemChanged(QListWidgetItem *)));
  connect(ui.exportButton, SIGNAL(released()),this, SLOT(exportThemeMapImage()));
}

void CMP::init()
{
  ui.setupUi(this);

  // Create the main stack of QImages
  mImageStack = new ImageStack;

  // Preferences window
  this->setupPreferencesWindow();

  // Set up and read the current settings for all preferences and
  // connect proper GUI elements to these settings
  g_settings = new Preferences("MarcLab", "CMP");
  this->connectPreferences();

  // Set up the GUI and connections
  this->setupToolbar();
  this->setupMenu();
  this->setupFileWindow();
  this->setupMdiArea();
  this->setupAdvancedPlotDialog();
  this->setupSaveThemeMapDialog();

  ui.clusterDisplay->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
  
  // PreFiltering connections
  connect(ui.smoothButton, SIGNAL(released()),
          this, SLOT(smoothImages()));
  connect(ui.resetSmoothButton, SIGNAL(released()),
          this, SLOT(resetSmoothImages()));

  // Clustering selections
  ui.clusterListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(ui.clusterCheckButton, SIGNAL(released()),
          this, SLOT(clusterCheckAll()));
  connect(ui.clusterUncheckButton, SIGNAL(released()),
          this, SLOT(clusterUncheckAll()));
  connect(ui.clusterButton, SIGNAL(released()),
          this, SLOT(computeClusters()));
  connect(ui.clusterListWidget, SIGNAL(itemChanged(QListWidgetItem *)),
          this, SLOT(clusterListItemChanged(QListWidgetItem *)));
  connect(ui.clusterDisplay->scene(), SIGNAL(imagePicked(int,int)),
          this, SLOT(clusterPicked(int,int)));
  connect(mImageStack, SIGNAL(progressChanged(int)),
          this, SLOT(updateProgress(int)));

  // Status Bar
  mProgressBar = new QProgressBar(this);
  mProgressBar->setRange(0,100);
  ui.statusBar->addPermanentWidget(mProgressBar);
  mProgressBar->hide();

  ui.statusBar->show();

  readSettings();
  update();
}


void CMP::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

void CMP::writeSettings()
{
  // Preference window settings save when modified.  This method includes things that are saved on exiting the program.
  g_settings->setValue("window/geometry", saveGeometry());
  g_settings->setValue("plots/advanced_histogram/number_of_bins", uiAdvancedPlotDialog.numBins->value());
  g_settings->setValue("plots/advanced_scatterplot/number_of_samples", uiAdvancedPlotDialog.numSamples->value());
}

void CMP::readSettings()
{
  // Reads only selected settings.   Other settings are queried when their respective UI elements are set up.
  restoreGeometry(g_settings->value("window/geometry").toByteArray());
  if (g_settings->value("file/load_path").isNull())
    g_settings->setValue("file/load_path", tr("."));
  if (g_settings->value("file/save_path").isNull())
    g_settings->setValue("file/save_path", tr("."));
}

void CMP::connectPreferences()
{
  // Read settings.

  // Cluster background color
  if (g_settings->value("main/cluster_background_color").isNull())
    {
      g_settings->setClusterBackgroundColor(QColor("gray"));
    }
  QColor color1 = g_settings->clusterBackgroundColor();
  mImageStack->setClusterBackgroundColor(color1);
  QString s1 = "background-color: ";
  uiPrefs.clusterBackgroundColorButton->setStyleSheet(s1 + color1.name());

  // Main window background color
  if (g_settings->value("main/background_color").isNull())
    {
      g_settings->setBackgroundColor(QColor("gray"));
    }
  QColor color = g_settings->backgroundColor();
  ui.ImageWindowArea->setBackground(QBrush(color));
  QString s = "background-color: ";
  uiPrefs.backgroundColorButton->setStyleSheet(s + color.name());

  // Number of bins for histogramming.
  if (g_settings->value("plots/quick_histogram/number_of_bins").isNull())
    {
      // Initialize number of bins
      uiPrefs.plotPrefsNumBins->setValue(256);
      g_settings->setNumberOfBins(256);
    }
  else 
    {
      uiPrefs.plotPrefsNumBins->setValue(
			 g_settings->value("plots/quick_histogram/number_of_bins").toInt());
   }

  // Number of samples for scatter plots
  if (g_settings->value("plots/quick_scatterplot/number_of_samples").isNull())
    {
      // Initialize number of bins
      uiPrefs.plotPrefsNumSamples->setValue(200);
      g_settings->setNumberOfSamples(200);
    }
  else 
    {
      uiPrefs.plotPrefsNumSamples->setValue(
			 g_settings->value("plots/quick_scatterplot/number_of_samples").toInt());
   }

   // Number of scatterplot and pawprint x-y pairs
  if (g_settings->value("plots/number_of_plot_pairs").isNull())
    {
      // Initialize number of bins
      uiPrefs.plotPrefsNumPlotPairs->setValue(5);
      g_settings->setNumberOfPlotPairs(5);
    }
  else 
    {
      uiPrefs.plotPrefsNumPlotPairs->setValue(
			 g_settings->value("plots/number_of_plot_pairs").toInt());
   }



  // Read the color palettes
  for (int i = 0; i < g_settings->numberOfColors(); i++)
    {
    if (i < (int)mColorPaletteWidget->numberOfColors())
      {
      // Favorite colors-- used for plots, etc
	  if (g_settings->value( QString("colors/favorite%1").arg(i) ).isNull())
	    {
	      g_settings->setColor(i,mColorPaletteWidget->color(i));
	    }
	  else
	    {
	      mColorPaletteWidget->setColor(i,g_settings->color(i));
	    }
      
      // Cluster colors -- used to color and identify clusters
      if (g_settings->value( QString("colors/cluster%1").arg(i) ).isNull())
	    {
	      g_settings->setClusterColor(i,mClusterColorPaletteWidget->color(i));
	    }
	  else
	    {
	      mClusterColorPaletteWidget->setColor(i,g_settings->clusterColor(i));
	    }
      }
    }
  
  //Set up the proper links between signals and slots 
  connect(uiPrefs.plotPrefsNumBins, SIGNAL(valueChanged(int)), g_settings, SLOT(setNumberOfBins(int)));
  connect(uiPrefs.plotPrefsNumPlotPairs, SIGNAL(valueChanged(int)), g_settings, SLOT(setNumberOfPlotPairs(int)));
  connect(uiPrefs.plotPrefsNumSamples, SIGNAL(valueChanged(int)), g_settings, SLOT(setNumberOfSamples(int)));
  connect(mColorPaletteWidget, SIGNAL(colorSelected(int,const QColor &)),g_settings,SLOT(setColor(int,const QColor &)));
  connect(mClusterColorPaletteWidget, SIGNAL(colorSelected(int,const QColor &)),g_settings,SLOT(setClusterColor(int,const QColor &)));
  connect(uiPrefs.backgroundColorButton,SIGNAL(released()),this,SLOT(setBackgroundColor()));
  connect(uiPrefs.clusterBackgroundColorButton,SIGNAL(released()),this,SLOT(setClusterBackgroundColor()));
}

void CMP::setClusterBackgroundColor()
{
  QColor color = QColorDialog::getColor(Qt::green, this);
  if (color.isValid()) 
    {
      QString s = "background-color: ";
      uiPrefs.clusterBackgroundColorButton->setStyleSheet(s + color.name());
      mImageStack->setClusterBackgroundColor(color);
      g_settings->setClusterBackgroundColor(color);
    }
}

void CMP::setBackgroundColor()
{
  QColor color = QColorDialog::getColor(Qt::green, this);
  if (color.isValid()) 
    {
      QString s = "background-color: ";
      uiPrefs.backgroundColorButton->setStyleSheet(s + color.name());
      ui.ImageWindowArea->setBackground(QBrush(color));
      g_settings->setBackgroundColor(color);
    }
}

void CMP::importPage(bool toggleOn)
{
  if (toggleOn) ui.fileManagerWindow->show();
  else ui.fileManagerWindow->hide();
}

void CMP::prefilterPage(bool toggleOn)
{
  if (toggleOn) ui.filteringWindow->show();
  else ui.filteringWindow->hide();
}

void CMP::clusterPage(bool toggleOn)
{
  if (toggleOn) ui.clusterWindow->show();
  else ui.clusterWindow->hide();
}

void CMP::updateVisualizeOptions()
{
  if (mVisualizeDirty)
  {
    // Update cluster list widget
    uiAdvancedPlotDialog.clusterNumberCombo->clear();
    for (int i = 0; i < mImageStack->numClusters(); i++)
    {
      QPixmap pix(20,20);
      pix.fill(mImageStack->clusterColor(i));
      uiAdvancedPlotDialog.clusterNumberCombo->insertItem(uiAdvancedPlotDialog.clusterNumberCombo->count(),
							  QIcon(pix), mImageStack->clusterName(i));
    }
    uiAdvancedPlotDialog.clusterNumberCombo->insertItem(uiAdvancedPlotDialog.clusterNumberCombo->count(), mNullMaskString);
							

    // Update image list widget
    uiAdvancedPlotDialog.visImageListWidget->clear();
    uiAdvancedPlotDialog.yAxisImageCombo->clear();
    uiAdvancedPlotDialog.xAxisImageCombo->clear();
    for (int i = 0; i < mImageStack->numImages(); i++)
    {
      QListWidgetItem *item = new QListWidgetItem;
      item->setText(mImageStack->name(i));
      item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      item->setCheckState(Qt::Unchecked);
      QPixmap pix(20,20);
      pix.fill(mImageStack->imageColor(i));
      item->setIcon(QIcon(pix));
      uiAdvancedPlotDialog.visImageListWidget->insertItem(uiAdvancedPlotDialog.visImageListWidget->count(), item);
      uiAdvancedPlotDialog.yAxisImageCombo->insertItem(uiAdvancedPlotDialog.yAxisImageCombo->count(), QIcon(pix), mImageStack->name(i));
      uiAdvancedPlotDialog.xAxisImageCombo->insertItem(uiAdvancedPlotDialog.xAxisImageCombo->count(), QIcon(pix), mImageStack->name(i));
    }
    uiAdvancedPlotDialog.visImageListWidget->adjustSize();

    mVisualizeDirty = false;
  }
}

void CMP::exportThemeMap()
{
  // If there are no clusters, display message and return
  if (mImageStack->numClusters() ==0)
    {
      QMessageBox::warning(this, tr("CMP"),
			   tr("Could not save theme map because no clusters have been computed."),
			   QMessageBox::Ok);
      return;
    }
  
  if (mExportDirty)
    {
      
      ui.exportClassListWidget->clear();
      for (int i = 0; i < mImageStack->numClusters(); i++)
	{
	  QListWidgetItem *item = new QListWidgetItem;
	  item->setText(mImageStack->clusterName(i));
	  item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	  item->setCheckState(Qt::Unchecked);
	  QPixmap pix(20,20);
	  pix.fill(mImageStack->clusterColor(i));
	  item->setIcon(QIcon(pix));
	  ui.exportClassListWidget->insertItem(ui.exportClassListWidget->count(), item); 
	}

      ui.exportButton->setEnabled(false);
      mExportColormapAction->setEnabled(false);
      mExportDirty = false;
    }
  mExportThemeMapDialog->show();
  mExportThemeMapDialog->adjustSize(); 
}

void CMP::updateImageDisplay()
{
  if (mImageStack->selectedImage(true).isNull())
    {
      //    ui.imageDisplay->setBackground(QImage());
      ui.imageDisplay->setQImageToDisplay(QImage());      
      reinterpret_cast<QMdiSubWindow *>(ui.ImageDisplaySubwindow->parent())->hide();
    }
  else
    {
      //    ui.imageDisplay->setBackground(mImageStack->selectedImage(true));
      ui.imageDisplay->setQImageToDisplay(mImageStack->selectedImage(true));
      //    ui.imageDisplay->getScene()->addPixmap((QPixmap::fromImage(mImageStack->selectedImage(true))));

    reinterpret_cast<QMdiSubWindow *>(ui.ImageDisplaySubwindow->parent())->show();
    ui.ImageDisplaySubwindow->show();
    }
}

void CMP::updateRGBImage(unsigned char COL)
{
  // Grab the pointer to the current list item
  int listIdx = mImageStack->selectedIndex();
    
  // Clear COL from all relevant Icon identifiers
  mImageStack->iconType(mRGBIndices[0]) &= ~COL;
  mImageStack->iconType(mRGBIndices[1]) &= ~COL;
  mImageStack->iconType(mRGBIndices[2]) &= ~COL;
    
  // Map COL to the appropriate mRGBIndices index
  int mapIdx;
  if ((COL & cmpRED) == cmpRED)          mapIdx = 0;
  else if ((COL & cmpGREEN) == cmpGREEN) mapIdx = 1;
  else if ((COL & cmpBLUE) == cmpBLUE)   mapIdx = 2;
  else return; // bad input
  
  // If index already matches this image index, set this color index to no image
  if (mRGBIndices[mapIdx] == listIdx)
    {
    mRGBIndices[mapIdx] = -1;
    }
  else  // Assign this color to this index and turn on COL in Icon
    {
    mRGBIndices[mapIdx] = listIdx;
    mImageStack->iconType(mRGBIndices[mapIdx]) |= COL;    
    }

  // Update the image list icons
  this->updateImageListIcons();
  
  // Construct and display the RGB image
  this->updateRGBImageDisplay();
}

void CMP::updateImageListIcons()
{
  for (int i = 0; i < ui.imageListWidget->count(); i++)
    {
    unsigned char COL = mImageStack->iconType(i);
    
    QListWidgetItem *item =  ui.imageListWidget->item(i);
    if ((COL & (cmpRED | cmpGREEN | cmpBLUE) ) == (cmpRED | cmpGREEN | cmpBLUE)) // RED-GREEN-BLUE
      {
      QIcon dataIcon (QString::fromUtf8(":/CMP/Resources/RGB.png"));
      item->setIcon(dataIcon);
      }
    else if ((COL & (cmpRED | cmpGREEN) ) == (cmpRED | cmpGREEN)) // RED-GREEN
      {
      QIcon dataIcon (QString::fromUtf8(":/CMP/Resources/RG.png"));
      item->setIcon(dataIcon);
      }
    else if ((COL & (cmpRED | cmpBLUE) ) == (cmpRED | cmpBLUE)) // RED-BLUE
      {
      QIcon dataIcon (QString::fromUtf8(":/CMP/Resources/RB.png"));
      item->setIcon(dataIcon);
      }
    else if ((COL & (cmpGREEN | cmpBLUE) ) == (cmpGREEN | cmpBLUE)) // GREEN-BLUE
      {
      QIcon dataIcon (QString::fromUtf8(":/CMP/Resources/GB.png"));
      item->setIcon(dataIcon);
      }
    else if ((COL & cmpRED) == cmpRED) // only RED
      {
      QIcon dataIcon (QString::fromUtf8(":/CMP/Resources/R.png"));
      item->setIcon(dataIcon);
      // QFont myfont = item->font();
      // myfont.setPointSize(myfont.pointSize() + mFontBoostSize);
      // item->setFont(myfont);
      }
    else if ((COL & cmpGREEN) == cmpGREEN) // only GREEN
      {
      QIcon dataIcon (QString::fromUtf8(":/CMP/Resources/G.png"));
      item->setIcon(dataIcon);
      // QFont myfont = item->font();
      // myfont.setPointSize(myfont.pointSize() + mFontBoostSize);
      //item->setFont(myfont);
      }
    else if ((COL & cmpBLUE) == cmpBLUE) // only BLUE
      {
      QIcon dataIcon (QString::fromUtf8(":/CMP/Resources/B.png"));
      item->setIcon(dataIcon);
      }
    else // no ICON
      {
      ui.imageListWidget->item(i)->setIcon(QIcon());
      }    
    }  
}

void CMP::updateRGBImageDisplay()
{
  if (mImageStack->selectedImage(true).isNull())
    {
    ui.rgbImageDisplay->setQImageToDisplay(QImage());
    reinterpret_cast<QMdiSubWindow *>(ui.rgbImageDisplaySubwindow->parent())->hide();
    }
  else
    {
    // Construct new RGB image to display
    const QImage &R = (mRGBIndices[0] >= 0) ? mImageStack->image(mRGBIndices[0],true) : QImage();
    const QImage &G = (mRGBIndices[1] >= 0) ? mImageStack->image(mRGBIndices[1],true) : QImage();
    const QImage &B = (mRGBIndices[2] >= 0) ? mImageStack->image(mRGBIndices[2],true) : QImage();
    
    unsigned int w, h;
    if (! R.isNull())
      {
      w = R.width();
      h = R.height();
      }
    else if (! G.isNull())
      {    
      w = G.width();
      h = G.height();
      }
    else if (! B.isNull())
      {    
      w = B.width();
      h = B.height();
      }
    else
      {
      ui.rgbImageDisplay->setQImageToDisplay(QImage());
      reinterpret_cast<QMdiSubWindow *>(ui.rgbImageDisplaySubwindow->parent())->hide();
      
      return; // None of RGB component images are defined   
      }
    
    QImage rgb(w,h, QImage::Format_RGB32);
    
    int numcomponents = sizeof(QRgb);
    //    int numcomponents = R.hasAlphaChannel() ? 4 : 3;
        
    for(unsigned int i=0; i < h; i++)
      {
      unsigned char* row;
      //      row = static_cast<unsigned char*>(vtkimage->GetScalarPointer(0, height-i-1, 0));
      row = rgb.scanLine(i);
      const QRgb* linePixelsR = R.isNull() ? NULL : reinterpret_cast<const QRgb*>(R.scanLine(i));
      const QRgb* linePixelsG = G.isNull() ? NULL : reinterpret_cast<const QRgb*>(G.scanLine(i));
      const QRgb* linePixelsB = B.isNull() ? NULL : reinterpret_cast<const QRgb*>(B.scanLine(i));
      for(unsigned int j=0; j < w; j++)
        {
        if (! R.isNull())
          {
          const QRgb& col = linePixelsR[j];
          row[j*numcomponents+2] = qRed(col);          
          }
        else { row[j*numcomponents+2] = 0; }

        if (! G.isNull())
          {
          const QRgb& col = linePixelsG[j];
          row[j*numcomponents+1] = qGreen(col);          
          }
        else { row[j*numcomponents+1] = 0; }
        
        if (! B.isNull())
          {
          const QRgb& col = linePixelsB[j];
          row[j*numcomponents+0] = qBlue(col);          
          }
        else { row[j*numcomponents+0] = 0; }
        //        row[j*numcomponents+3] = qAlpha(col);
                
        }
      }

    ui.rgbImageDisplay->setQImageToDisplay(rgb);
    reinterpret_cast<QMdiSubWindow *>(ui.rgbImageDisplaySubwindow->parent())->show();
    ui.rgbImageDisplaySubwindow->show();
  }
}

void CMP::updateImageFilterDisplay()
{
  if (mImageStack->selectedImage(false).isNull() 
      || (mImageStack->selectedImage(false) == mImageStack->selectedImage(true)))
    {
    ui.filterDisplay->setQImageToDisplay(mImageStack->image(0,false));
    reinterpret_cast<QMdiSubWindow *>(ui.FilterDisplaySubwindow->parent())->hide();
    }
  else
    {
    ui.filterDisplay->setQImageToDisplay(mImageStack->selectedImage(false));   
    reinterpret_cast<QMdiSubWindow *>(ui.FilterDisplaySubwindow->parent())->show();
    ui.FilterDisplaySubwindow->show();
    }
}

void CMP::importSelectionChanged()
{
  QList<QListWidgetItem *> items = ui.imageListWidget->selectedItems();
  if (items.size())
  {
    ui.deleteButton->setEnabled(true);
    mImageStack->setSelectedByName((items.at(0))->text());
    reinterpret_cast<QMdiSubWindow *>(ui.ImageDisplaySubwindow->parent())->setWindowTitle(items.at(0)->text());
    reinterpret_cast<QMdiSubWindow *>(ui.FilterDisplaySubwindow->parent())->setWindowTitle(items.at(0)->text());
  }
  else
  {
    ui.deleteButton->setEnabled(false);
    mImageStack->clearSelection();
  }
  updateImageDisplay();
  updateImageFilterDisplay();
}


void CMP::importAdd()
{
  QStringList files = QFileDialog::getOpenFileNames(this, tr("Import Images"),
                                                    g_settings->value("file/load_path").toString(), 
                                                    tr("ImageFiles (*.tif *.png *.jpg)"));
  for (int i = 0; i < files.size(); i++)
  {
    QString imagePath = files.at(i);
    if (!addImageFromFile(imagePath))
      break;
  }
}

void CMP::importDelete()
{
  QList<QListWidgetItem *> items = ui.imageListWidget->selectedItems();
  for (int i = items.size()-1; i >=0; i--)
  {
    QListWidgetItem *item = items.at(i);
    int row = ui.imageListWidget->row(item);
    QString name = item->text();
    if (mImageStack->removeImage(name))
    {
      ui.imageListWidget->takeItem(row);
      delete item;
    }
  }
  // update the visualization widget options
  this->updateVisualizeOptions();
  
  mVisualizeDirty = true;
  mExportDirty = true;
}

void CMP::imageDropped(QString fname)
{
  addImageFromFile(fname);
}

bool CMP::addImageFromFile(QString fname)
{
  if (mImageStack->addImage(fname))
  {
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(mImageStack->selectedName());
    ui.imageListWidget->insertItem(ui.imageListWidget->count(), item);
    ui.imageListWidget->setCurrentRow(ui.imageListWidget->count()-1);
    
    // Resize the image view windows to match this image
    reinterpret_cast<QMdiSubWindow *>(ui.ImageDisplaySubwindow->parent())->resize(mImageStack->selectedImage(true).size());
    reinterpret_cast<QMdiSubWindow *>(ui.rgbImageDisplaySubwindow->parent())->resize(mImageStack->selectedImage(true).size());
    reinterpret_cast<QMdiSubWindow *>(ui.FilterDisplaySubwindow->parent())->resize(mImageStack->selectedImage(true).size());
    reinterpret_cast<QMdiSubWindow *>(ui.ClusterDisplaySubwindow->parent())->resize(mImageStack->selectedImage(true).size());

    // Update the visualization menu options
    this->updateVisualizeOptions();    
  }
  else
    {
    // int ret =
    QMessageBox::warning(this, tr("CMP"),
                         tr("There was an error loading image at %1").arg(fname),
                         QMessageBox::Ok);
    
    }
  QFileInfo fi(fname);
  QString path = fi.canonicalPath();
  if (!path.isNull())
    g_settings->setValue("file/load_path", path);
  
  mVisualizeDirty = true;
  mExportDirty = true;
  
  return true;
}

void CMP::smoothImages()
{
  ui.statusBar->showMessage(tr("Smoothing images..."));
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  mProgressBar->show();
  
  mImageStack->bilateralFilter(ui.kernelSizeCB->value(),
                               ui.colorSigmaCB->value(),
                               ui.spaceSigmaCB->value());
  updateImageFilterDisplay();

  mProgressBar->hide();
  ui.statusBar->clearMessage();
  QApplication::restoreOverrideCursor();
  qApp->flush(); // ignore any user events that have accumulated
  
  mVisualizeDirty = true;
  mExportDirty = true;
}


void CMP::resetSmoothImages()
{
  mImageStack->resetFilters();
  updateImageFilterDisplay();
  //  QApplication::restoreOverrideCursor();
  ui.statusBar->clearMessage();
}


void CMP::computeClusters()
{
  // Do some error checking
  if (mImageStack->numImages() == 0)
    {
    // int ret =
    QMessageBox::warning(this, tr("CMP"),
                         tr("Could not compute clusters because no images have been loaded."),
                         QMessageBox::Ok);
    return;
    }
  
  // Update the status bar
  ui.statusBar->showMessage(tr("Computing clusters..."));
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  mProgressBar->show();
  
  for (int i = 0; i < ui.clusterListWidget->count(); i++)
    {
    mImageStack->setClusterMasked(i, ui.clusterListWidget->item(i)->checkState() == Qt::Checked);
    }
  
  try 
    {
      time_t tmpTime;
      time(&tmpTime);
      QString qs = QString(ctime(&tmpTime)) 
	+ QString("K-Means: computing\n clusters\t%1\n max iters.\t%2\n accur.\t%3\n trials\t%4\n")
	.arg(ui.clusterNumSB->value())
	.arg(ui.itersSB->value())
	.arg(ui.accuracySB->value())
	.arg(ui.numTrials->value());

      ui.clusterOutputWindow->append(qs);


    std::vector<std::vector<double> > centers; // output for the centers
    double compactness; // output for compactness
    
    int result = mImageStack->kMeansCluster(ui.clusterNumSB->value(), 
					    ui.itersSB->value(), 
					    ui.accuracySB->value(),
					    ui.numTrials->value(),
					    mClusterColorPaletteWidget->colorList(),
					    centers,
					    compactness);
    if (result > 0)
      {
      updateClusterDisplay();

      time(&tmpTime);
      ui.clusterOutputWindow->append( QString(ctime(&tmpTime))+QString("K-Means: converged"));
      ui.clusterOutputWindow->append(QString(" compactness: %1").arg(compactness,0,'e',5));

      // Print the centers
      ui.clusterOutputWindow->append(QString(" centers"));
      for (unsigned int i = 0; i < centers.size(); i++)
        {
	  QString s = QString(" %1: ").arg(i+1);

	  for (unsigned int k = 0; k < centers[i].size(); k++)
	    {
	      s.append(QString("%1 ").arg(centers[i][k],0,'d',2));
	    }
	  ui.clusterOutputWindow->append(s);
        }
      
      ui.clusterOutputWindow->append("\n");
      // update the visualization menu options
      this->updateVisualizeOptions();
      }
    
    // Change the message on the clustering button
    //    ui.clusterButton->resize(ui.clusterButton->width() * 3, ui.clusterButton->height());
    ui.clusterButton->setText("Cluster Again");
    }
  catch(Exception &ex)
    {
    QMessageBox::warning(this, tr("CMP"), tr(ex.info.c_str()), QMessageBox::Ok);
    ui.clusterOutputWindow->append(QString("ERROR! K-means failed."));
    }
  mProgressBar->hide();
  ui.statusBar->clearMessage();
  QApplication::restoreOverrideCursor();
  
  mVisualizeDirty = true;
  mExportDirty = true;
}


void CMP::clusterListItemChanged(QListWidgetItem *item)
{
  int currentRow = ui.clusterListWidget->currentRow();
  // Name changed
  if (item->text().compare(mImageStack->clusterName(currentRow)) != 0)
  {
    mImageStack->setClusterName(currentRow, item->text());
  }
  // Check state Changed
  bool checkState = item->checkState()==Qt::Checked;
  if (checkState != mImageStack->clusterMasked(currentRow))
  {
    if (checkState) 
      {
	//	ui.clusterButton->resize(ui.clusterButton->width() * 3,ui.clusterButton->height());
	ui.clusterButton->setText("Cluster Unchecked Classes");
      }
    mImageStack->setClusterMasked(currentRow, checkState);
    ui.clusterDisplay->setQImageToDisplay(mImageStack->maskedClusterImage());
  }

}

void CMP::updateClusterDisplay()
{
  // Set the image and show the necessary widgets.
  ui.clusterDisplay->setQImageToDisplay(mImageStack->maskedClusterImage());
  reinterpret_cast<QMdiSubWindow *>(ui.ClusterDisplaySubwindow->parent())->show();
  ui.ClusterDisplaySubwindow->show();
  ui.clusterDisplay->show();
  
  ui.clusterListWidget->clear();
  for (int i = 0; i < mImageStack->numClusters(); i++)
  {
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(mImageStack->clusterName(i));
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    item->setCheckState((mImageStack->clusterMasked(i)) ? Qt::Checked : Qt::Unchecked);
    QPixmap pix(20,20);
    pix.fill(mImageStack->clusterColor(i));
    item->setIcon(QIcon(pix));
    ui.clusterListWidget->insertItem(ui.clusterListWidget->count(), item);
  }
  
  ui.clusterListWidget->setCurrentRow(ui.clusterListWidget->count()-1);
}

void CMP::clusterPicked(int x, int y)
{
  int cluster = mImageStack->pixelCluster(x, y);
  if (cluster >= 0)
  {
    ui.clusterListWidget->setCurrentRow(cluster);
  }
}



void CMP::exportListItemChanged(QListWidgetItem *item)
{
  bool checked = false;
  int numEntries = ui.exportClassListWidget->count();
  std::vector<bool> selectedClusters;
  for (int i = 0; i < numEntries; i++)
  {
    bool state = ui.exportClassListWidget->item(i)->checkState()==Qt::Checked;
    selectedClusters.push_back(state);
    if (state)
      checked = true;
  }
  //  ui.exportDisplay->setBackground(mImageStack->selectedClusterImage(selectedClusters));
  ui.exportButton->setEnabled(checked);
  mExportColormapAction->setEnabled(checked);
}

void CMP::exportThemeMapImage()
{
  QString file = QFileDialog::getSaveFileName(this, tr("Export Image"),
					      g_settings->value("file/save_path").toString(), 
					      tr("ImageFiles (*.tif *.png *.jpg)"));

  int numEntries = ui.exportClassListWidget->count();
  std::vector<bool> selectedClusters;
  for (int i = 0; i < numEntries; i++)
    {
      selectedClusters.push_back(ui.exportClassListWidget->item(i)->checkState()==Qt::Checked);
    }
  QImage img = mImageStack->selectedClusterImage(selectedClusters);
  bool success  =  img.save(file);
  
  // Save operation went wrong for reasons other than already reported.
  if (! success)
    {
      if (file.isNull() || file.isEmpty())
	{
 	  //        QMessageBox::warning(this, tr("CMP"),
 	  //                     tr("There was an error saving the image."),
 	  //                     QMessageBox::Ok);        
	}
      else
	{
 	  QMessageBox::warning(this, tr("CMP"),
 			       tr("There was an error saving theme map %1").arg(file),
 			       QMessageBox::Ok);
	}
    }
  else
    {
      // Save the path info in preferences
      QFileInfo fi(file);
      QString path = fi.canonicalPath();
      if (!path.isNull())
	g_settings->setValue("file/save_path", path);    
    }
  mExportThemeMapDialog->hide();
}

void CMP::clusterCheckAll()
{
  int numEntries = ui.clusterListWidget->count();
  for (int i = 0; i < numEntries; i++)
  {
    // Set the checked state of the interface widget
    ui.clusterListWidget->item(i)->setCheckState(Qt::Checked);

    // Register this as masked in the image stack structure
    mImageStack->setClusterMasked(i, true);
  }

  // Update the cluster display image
  ui.clusterDisplay->setQImageToDisplay(mImageStack->maskedClusterImage());

}


void CMP::clusterUncheckAll()
{
  int numEntries = ui.clusterListWidget->count();
  for (int i = 0; i < numEntries; i++)
  {
    // Set the checked state of the interface widget
    ui.clusterListWidget->item(i)->setCheckState(Qt::Unchecked);

    // Register this as masked in the image stack structure
    mImageStack->setClusterMasked(i, false);
  }

  // Update the cluster display image
  ui.clusterDisplay->setQImageToDisplay(mImageStack->maskedClusterImage());
}

void CMP::visClassCheckAll()
{
  //  int numEntries = ui.visClassListWidget->count();
  // for (int i = 0; i < numEntries; i++)
  // {
  //   ui.visClassListWidget->item(i)->setCheckState(Qt::Checked);
  // }
}


void CMP::visClassUncheckAll()
{
  // int numEntries = ui.visClassListWidget->count();
  // for (int i = 0; i < numEntries; i++)
  // {
  //   ui.visClassListWidget->item(i)->setCheckState(Qt::Unchecked);
  // }
}

void CMP::visImageCheckAll()
{
   int numEntries = uiAdvancedPlotDialog.visImageListWidget->count();
   for (int i = 0; i < numEntries; i++)
    {
     uiAdvancedPlotDialog.visImageListWidget->item(i)->setCheckState(Qt::Checked);
    }
}


void CMP::visImageUncheckAll()
{
  int numEntries = uiAdvancedPlotDialog.visImageListWidget->count();
  for (int i = 0; i < numEntries; i++)
    {
      uiAdvancedPlotDialog.visImageListWidget->item(i)->setCheckState(Qt::Unchecked);
    }
}

void CMP::exportCheckAll()
{
  int numEntries = ui.exportClassListWidget->count();
  for (int i = 0; i < numEntries; i++)
  {
    ui.exportClassListWidget->item(i)->setCheckState(Qt::Checked);
  }
}


void CMP::exportUncheckAll()
{
  int numEntries = ui.exportClassListWidget->count();
  for (int i = 0; i < numEntries; i++)
  {
    ui.exportClassListWidget->item(i)->setCheckState(Qt::Unchecked);
  }
}

void CMP::updateProgress(int p)
{
  mProgressBar->setValue(p);
  // mProgressBar->show();
  // mProgressBar->repaint();
  
  //  if (qApp->overrideCursor()->shape() == Qt::WaitCursor)
  //   {
      //    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor),TRUE);
      //      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  //  }
  
  // TODO: The following is a hack to get the progress bar and status bar to
  // actually repaint.  Probably there is a much better way to do this?  Maybe
  // using a QTimer? - jc
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents,10);
}


void CMP::savePlotDataToDisk()
{   
  if ( ui.ImageWindowArea->activeSubWindow() != 0 )
    {
    bool success = true;
    
    QString filename;
    
    // Switch on subwindow.  Each type may need different handling.    
    if (ui.ImageWindowArea->currentSubWindow()->widget() == ui.ImageDisplaySubwindow)
      {
	QMessageBox::warning(this, tr("CMP"),
			     tr("Save data operation not supported for selected window."),
			     QMessageBox::Ok);      
	
	//  QImage img(ui.imageDisplay->getScene()->getBackground()->pixmap());
	// success = img.save(filename);
      }
    else if (ui.ImageWindowArea->currentSubWindow()->widget() == ui.rgbImageDisplaySubwindow)
      {
	QMessageBox::warning(this, tr("CMP"),
			     tr("Save data operation not supported for selected window."),
			     QMessageBox::Ok);      
	//  QImage img(ui.rgbImageDisplay->getScene()->getBackground()->pixmap());
	// success = img.save(filename);
      }
    else if (ui.ImageWindowArea->activeSubWindow()->widget() == ui.ClusterDisplaySubwindow)
      {
	QMessageBox::warning(this, tr("CMP"),
			     tr("Save data operation not supported for selected window."),
			     QMessageBox::Ok);      
	// QImage img(ui.clusterDisplay->getScene()->getBackground()->pixmap());
	// success = img.save(filename);
      }
    else if (ui.ImageWindowArea->activeSubWindow()->widget() == ui.FilterDisplaySubwindow)
      {
	QMessageBox::warning(this, tr("CMP"),
			     tr("Save data operation not supported for selected window."),
			     QMessageBox::Ok);      
	
      //  QImage img(ui.filterDisplay->getScene()->getBackground()->pixmap());
      //  success = img.save(filename);
      }
    else //if (typeid(ui.ImageWindowArea->activeSubWindow()->widget()) == typeid(QwtDataPlot *))
      {
	// Ok this is bad form, but safe until adding new window types.
	//
	// Grab a filename from the user
	filename = QFileDialog::getSaveFileName(this, tr("Image Name"),
						g_settings->value("file/save_path").toString()); 
	success = dynamic_cast<QwtDataPlot *>(ui.ImageWindowArea->activeSubWindow()->widget())->save(filename.toStdString());
	
      //success = pix.save(filename);
      }
    // else
    //   {  
    //   QMessageBox::warning(this, tr("CMP"),
    //                        tr("Save data operation not supported for selected window."),
    //                        QMessageBox::Ok);      
    //  return;
    //  }
    
    // Save operation went wrong for reasons other than already reported.
    if (! success)
      {
      if (filename.isNull() || filename.isEmpty())
        {
        // User probably hit "cancel", so do nothing.
        }
      else
        {
        QMessageBox::warning(this, tr("CMP"),
                             tr("There was an error saving image %1").arg(filename),
                             QMessageBox::Ok);
        }
      }
    else
      {
      // Save the path info in preferences
      QFileInfo fi(filename);
      QString path = fi.canonicalPath();
      if (!path.isNull())
        g_settings->setValue("file/save_path", path);    
      }    
    
    }
  else
    {
    QMessageBox::warning(this, tr("CMP"),
                         tr("No window is selected.  Please select a window before choosing save."),
                         QMessageBox::Ok);
    return;
    } 
  
  

}

void CMP::saveWindowToDisk()
{
  // Switch on active window and do the appropriate thing.  If it is
  // an image window, save the underlying image data.  Otherwise, grab
  // the widget display and save what the user sees.
  if (ui.ImageWindowArea->activeSubWindow() != 0)
    {
    bool success = true;  

    // Grab a filename from the user
    QString filename = QFileDialog::getSaveFileName(this, tr("Image Name"),
                                                    g_settings->value("file/save_path").toString(), 
                                                    tr("ImageFiles (*.tif *.png *.jpg)")); 

    // Switch on subwindow.  Each type may need different handling.    
    if (ui.ImageWindowArea->currentSubWindow()->widget() == ui.ImageDisplaySubwindow)
      {
	QImage img(ui.imageDisplay->getScene()->getBackground()->pixmap().toImage());
	success = img.save(filename);
      }
    else if (ui.ImageWindowArea->currentSubWindow()->widget() == ui.rgbImageDisplaySubwindow)
      {
	QImage img(ui.rgbImageDisplay->getScene()->getBackground()->pixmap().toImage());
	success = img.save(filename);
      }
    else if (ui.ImageWindowArea->activeSubWindow()->widget() == ui.ClusterDisplaySubwindow)
      {
	QImage img(ui.clusterDisplay->getScene()->getBackground()->pixmap().toImage());
	success = img.save(filename);
      }
    else if (ui.ImageWindowArea->activeSubWindow()->widget() == ui.FilterDisplaySubwindow)
      {
	QImage img(ui.filterDisplay->getScene()->getBackground()->pixmap().toImage());
	success = img.save(filename);
      }
    else // Just grab the widget contents with a pixmap and save
      {
	QPixmap pix = QPixmap::grabWidget(ui.ImageWindowArea->activeSubWindow()->widget());
	
	if(pix.isNull())
	  {  
	    QMessageBox::warning(this, tr("CMP"),
				 tr("Save operation not supported for selected window."),
				 QMessageBox::Ok);      
	    return;
        }
      else
        {
	    success = pix.save(filename);
        }
      }
    
    // Save operation went wrong for reasons other than already reported.
    if (! success)
      {
      if (filename.isNull() || filename.isEmpty())
        {
	    // User probably hit "cancel", so do nothing.
        }
      else
        {
	    QMessageBox::warning(this, tr("CMP"),
                             tr("There was an error saving image %1").arg(filename),
                             QMessageBox::Ok);
        }
      }
    else
      {
      // Save the path info in preferences
      QFileInfo fi(filename);
      QString path = fi.canonicalPath();
      if (!path.isNull())
        g_settings->setValue("file/save_path", path);    
      }
    }
  else
    {
    QMessageBox::warning(this, tr("CMP"),
			   tr("No window is selected.  Please select a window before choosing save."),
                         QMessageBox::Ok);
    }
}

} // end namespace cmp
