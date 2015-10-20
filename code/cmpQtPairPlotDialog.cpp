#include "cmpQtPairPlotDialog.h"
#include <iostream>
namespace cmp
{

QtPairPlotDialog::QtPairPlotDialog(unsigned int n)
{
  // Specify number of pairs to plot
  mNumberOfPairs = n;
  
  if (this->objectName().isEmpty()) 
    { this->setObjectName(QString::fromUtf8("PairPlotDialog")); }
  //  this->resize(511, 545);
  mMainVertLayout = new QVBoxLayout(this);
  mMainVertLayout->setObjectName(QString::fromUtf8("mMainVertLayout"));

  QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);

  // Cluster combo box 
  mClusterLayout = new QHBoxLayout();
  mClusterLayout->setObjectName(QString::fromUtf8("mClusterLayout"));
  mClusterNumberComboLabel = new QLabel(this);
  mClusterNumberComboLabel->setObjectName(QString::fromUtf8("mClusterNumberComboLabel"));  
  mClusterLayout->addWidget(mClusterNumberComboLabel);  
  //  mClusterLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
  
  mClusterNumberCombo = new QComboBox(this);
  mClusterNumberCombo->setObjectName(QString::fromUtf8("mClusterNumberCombo"));
  sizePolicy.setHeightForWidth(mClusterNumberCombo->sizePolicy().hasHeightForWidth());
  mClusterNumberCombo->setSizePolicy(sizePolicy);
  mClusterNumberCombo->setMinimumSize(QSize(200, 0));
  
  mClusterLayout->addWidget(mClusterNumberCombo);
  
  mClusterLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
  mMainVertLayout->addLayout(mClusterLayout);

  
  // Construct the list of pairs
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setAlignment(Qt::AlignCenter);
  vl->addWidget(new QLabel(QString("Image Data (x - y pairs)")));
  for (unsigned int i = 0; i < mNumberOfPairs; i++)
    {
    // Allocate combo boxes and store pointers in lists
    QComboBox *tmpY  = new QComboBox();
    QComboBox *tmpX  = new QComboBox();
    QLabel *tmpLabel = new QLabel();
    mComboLabels.push_back(tmpLabel);
    mYAxisCombos.push_back(tmpY);
    mXAxisCombos.push_back(tmpX);    

    // Setup layout
    QHBoxLayout *hl = new QHBoxLayout();
    hl->setAlignment(Qt::AlignLeft);
    QPixmap tmpPix(20,20);
    tmpPix.fill(QColor("black"));    
    tmpLabel->setPixmap(tmpPix);

    QLabel *l = new QLabel();
    l->setText(QString("%1").arg(i+1));

    hl->addWidget(tmpLabel);
    hl->addWidget(l);
    //    hl->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hl->addWidget(tmpX);
    //    hl->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hl->addWidget(tmpY);
    hl->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    vl->addLayout(hl);
    }
  vl->addWidget(new QLabel(QString("( Enable additional pairs in Preferences )")));
  mMainVertLayout->addLayout(vl);
  

  // Number of bins spinner
  QHBoxLayout *mHorizontalLayout_2 = new QHBoxLayout();
  mHorizontalLayout_2->setObjectName(QString::fromUtf8("mHorizontalLayout_2"));
  mNumBinsLabel = new QLabel(this);
  mNumBinsLabel->setObjectName(QString::fromUtf8("mNumBinsLabel"));
  
  mHorizontalLayout_2->addWidget(mNumBinsLabel);  
  //  mHorizontalLayout_2->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
  
  mNumBins = new QSpinBox(this);
  mNumBins->setObjectName(QString::fromUtf8("mNumBins"));
  mNumBins->setMinimum(1);
  mNumBins->setMaximum(1000000);
  
  mHorizontalLayout_2->addWidget(mNumBins);  
  mHorizontalLayout_2->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
  
  mMainVertLayout->addLayout(mHorizontalLayout_2);

  // Number of samples spinner
  QHBoxLayout *mHorizontalLayout_3 = new QHBoxLayout();
  mHorizontalLayout_3->setObjectName(QString::fromUtf8("mHorizontalLayout_3"));
  mNumSamplesLabel = new QLabel(this);
  mNumSamplesLabel->setObjectName(QString::fromUtf8("mNumSamplesLabel"));
  
  mHorizontalLayout_3->addWidget(mNumSamplesLabel);  
  // mHorizontalLayout_3->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
  mNumSamples = new QSpinBox(this);
  mNumSamples->setObjectName(QString::fromUtf8("mNumSamples"));
  mNumSamples->setMinimum(2);
  mNumSamples->setMaximum(65535);
  
  mHorizontalLayout_3->addWidget(mNumSamples);  
  mHorizontalLayout_3->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
  
  mMainVertLayout->addLayout(mHorizontalLayout_3);
  
  buttonBox = new QDialogButtonBox(this);
  buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
  
  mMainVertLayout->addWidget(buttonBox);
  
  this->setWindowTitle(QApplication::translate("this", "Dialog", 0, QApplication::UnicodeUTF8));
  mClusterNumberComboLabel->setText(QApplication::translate("this", "Mask with cluster -> ", 0, QApplication::UnicodeUTF8));
  mNumBinsLabel->setText(QApplication::translate("this", "Number of Bins", 0, QApplication::UnicodeUTF8));
  mNumSamplesLabel->setText(QApplication::translate("this", "Number of Samples", 0, QApplication::UnicodeUTF8));
  
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  
  QMetaObject::connectSlotsByName(this);

  this->adjustSize();
}
 
} // end namespace cmp
