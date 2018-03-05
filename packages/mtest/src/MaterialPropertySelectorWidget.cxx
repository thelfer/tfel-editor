/*!
 * \file   MaterialPropertySelectorWidget.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   01/03/2018
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include "QEmacs/MaterialPropertySelectorWidget.hxx"

namespace qemacs {

  MaterialPropertySelectorWidget::MaterialPropertySelectorWidget(
      const QString& n, QWidget* const p)
      : QWidget(p) {
    auto* const gl = new QGridLayout(this);
    auto* const nw = new QLabel(n,this);
    auto* const mt = new QComboBox();
    gl->addWidget(nw, 0, 0);
    gl->addWidget(mt, 0, 1);
    this->setLayout(gl);
}  // end
  // of MaterialPropertySelectorWidget::MaterialPropertySelectorWidget

  MaterialPropertySelectorWidget::~MaterialPropertySelectorWidget() =
      default;

}  // end of namespace qemacs
