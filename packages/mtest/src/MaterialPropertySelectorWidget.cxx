/*!
 * \file   MaterialPropertySelectorWidget.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   01/03/2018
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include "QEmacs/ImportMFMMaterialProperty.hxx"
#include "QEmacs/MaterialPropertySelectorWidget.hxx"

namespace qemacs {

  MaterialPropertySelectorWidget::MaterialPropertySelectorWidget(
      const QString& n, QWidget* const p)
      : QGroupBox(n,p) {
    auto* const ml = new QVBoxLayout();
    auto* const mt = new QComboBox();
    mt->addItems(QStringList() << "constant"
                               << "castem");
    ml->addWidget(mt);
    auto * mpd = new QStackedWidget;
    // a line edit to set the constant value
    auto* const cv = new QLineEdit;
    cv->setFixedHeight(QFontMetrics(cv->font()).lineSpacing());
    mpd->addWidget(cv);
    // a widget to set a castem material property
    auto* const cw = new QGroupBox;
    auto* const gl = new QGridLayout;
    // library
    gl->addWidget(new QLabel(QObject::tr("Library")),0,0);
    auto* const cle = new QLineEdit;
    cle->setFixedHeight(QFontMetrics(cle->font()).lineSpacing());
    gl->addWidget(cle,0,1);
    // function
    gl->addWidget(new QLabel(QObject::tr("Function")),1,0);
    auto* const cfe = new QLineEdit;
    cfe->setFixedHeight(QFontMetrics(cfe->font()).lineSpacing());
    gl->addWidget(cfe,1,1);
    // import buttons
    auto* const co = new QPushButton("Browse");
    co->setIcon(QIcon::fromTheme("document-open"));
    QObject::connect(co, &QPushButton::clicked, this,
                     &MaterialPropertySelectorWidget::import);
    auto* const cmfm = new QPushButton("Import");
    QObject::connect(cmfm, &QPushButton::clicked, this,
                     &MaterialPropertySelectorWidget::importFromMFM);
    //
    auto* const chl = new QHBoxLayout;
    chl->addLayout(gl);
    auto* const bl = new QVBoxLayout;
    bl->addWidget(co);
    bl->addWidget(cmfm);
    chl->addLayout(bl);
    cw->setLayout(chl);
    mpd->addWidget(cw);
    //
    ml->addWidget(mpd);
    this->setLayout(ml);

    QObject::connect(mt, static_cast<void (QComboBox::*)(int)>(
                             &QComboBox::activated),
                     mpd, &QStackedWidget::setCurrentIndex);
  }  // end
  // of MaterialPropertySelectorWidget::MaterialPropertySelectorWidget

  void MaterialPropertySelectorWidget::import() {
  }  // end of MaterialPropertySelectorWidget::import

  void MaterialPropertySelectorWidget::importFromMFM() {
    ImportMFMMaterialProperty::Options o;
    o.minterface = "Castem";
    ImportMFMMaterialProperty w(o);
    if (w.exec() != QDialog::Accepted) {
      return;
   }
   

  }  // end of MaterialPropertySelectorWidget::importFromMFM

  MaterialPropertySelectorWidget::~MaterialPropertySelectorWidget() =
      default;

}  // end of namespace qemacs
