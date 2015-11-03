#ifndef __cmpQtPairPlotDialog__
#define __cmpQtPairPlotDialog__

#include <vector>

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QSpacerItem>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

namespace cmp
{

/** A custom Qt widget for specifying a n x 2 matrix of combo boxes, plus some
    other inputs to support plotting.  Used for scatterplotting and pawprints. */
class QtPairPlotDialog : public QDialog
{
public:
  /** Widgets for specifying cluster information. */
  QComboBox *mClusterNumberCombo;
  QLabel *mClusterNumberComboLabel;

  /** The array of Combo Boxes */
  std::vector<QComboBox *> mYAxisCombos;
  std::vector<QComboBox *> mXAxisCombos;

  /** Row labels for the combo box rows. */
  std::vector<QLabel *>    mComboLabels;

  /** Colors associated with each row */
  std::vector<QColor> mColors;

  QLabel *mNumBinsLabel;
  QSpinBox *mNumBins;
  QLabel *mNumSamplesLabel;
  QSpinBox *mNumSamples;
  QDialogButtonBox *buttonBox;
  
  unsigned int mNumberOfPairs;
    
  QtPairPlotDialog(unsigned int i);
  ~QtPairPlotDialog() {}

private:
  /** Layout widget. */
  QVBoxLayout *mMainVertLayout;
  QHBoxLayout *mClusterLayout;
};
  
  
} // end namespace

#endif //__cmpQtPairPlotDialog__
