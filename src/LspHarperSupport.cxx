/*!
 * \file   LspHarperSupport.cxx
 * \brief  finding `harper-ls`, and the settings its session is answered with.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "TFEL/GUI/LspHarperSupport.hxx"

namespace tfel::gui {
  QString LspHarperSupport::serverExecutable() {
    auto settings = QSettings();
    const auto configured = settings.value(QStringLiteral("harper/executable"))
                                .toString()
                                .trimmed();
    if (!configured.isEmpty()) {
      // an absolute path is taken as it stands; a bare name is looked for as
      // the default one is, so that a relative answer is not read against the
      // directory the editor happens to run in
      return QFileInfo::exists(configured)
                 ? configured
                 : QStandardPaths::findExecutable(configured);
    }
    return QStandardPaths::findExecutable(QStringLiteral("harper-ls"));
  }  // end of LspHarperSupport::serverExecutable

  bool LspHarperSupport::launchByDefault() {
    return QSettings()
        .value(QStringLiteral("harper/launch-by-default"), false)
        .toBool();
  }  // end of LspHarperSupport::launchByDefault

  void LspHarperSupport::setLaunchByDefault(const bool on) {
    auto settings = QSettings();
    if (on) {
      settings.setValue(QStringLiteral("harper/launch-by-default"), true);
    } else {
      // the default is not stored, as no entry of the editor stores its own
      // default: the file holds what was chosen, not what the editor would
      // have answered anyway
      settings.remove(QStringLiteral("harper/launch-by-default"));
    }
  }  // end of LspHarperSupport::setLaunchByDefault

  QString LspHarperSupport::dialect() {
    const auto name =
        QSettings().value(QStringLiteral("harper/dialect")).toString();
    return dialects().contains(name) ? name : dialects().front();
  }  // end of LspHarperSupport::dialect

  void LspHarperSupport::setDialect(const QString &name) {
    auto settings = QSettings();
    if ((name == dialects().front()) || (!dialects().contains(name))) {
      // "American" is what the server is by default and what a stored value
      // of "American" says no more than its absence; a name which is none of
      // the five is no choice either, and keeping it would have the next
      // session read the text as a dialect nobody can select again
      settings.remove(QStringLiteral("harper/dialect"));
      return;
    }
    settings.setValue(QStringLiteral("harper/dialect"), name);
  }  // end of LspHarperSupport::setDialect

  QStringList LspHarperSupport::dialects() {
    static const auto names = QStringList() << QStringLiteral("American")
                                            << QStringLiteral("Canadian")
                                            << QStringLiteral("Australian")
                                            << QStringLiteral("British")
                                            << QStringLiteral("Indian");
    return names;
  }  // end of LspHarperSupport::dialects

  QString LspHarperSupport::dialectLabel(const QString &name) {
    // the names above are those the server reads; a person chooses a variety
    // of English, and the entry of a menu says so
    return name + QStringLiteral(" English");
  }  // end of LspHarperSupport::dialectLabel

  QStringList LspHarperSupport::serverArguments() {
    return QStringList{QStringLiteral("--stdio")};
  }  // end of LspHarperSupport::serverArguments

  QJsonValue LspHarperSupport::configuration() {
    // the settings of the server live under its own name and nowhere else:
    // an answer which does not hold a "harper-ls" key is refused, and the
    // server says so, and it asks them again before every analysis, so the
    // dialect a document is read with is the one held when the change of it
    // is sent, not the one of the handshake
    return QJsonObject{{QStringLiteral("harper-ls"),
                        QJsonObject{{QStringLiteral("dialect"), dialect()}}}};
  }  // end of LspHarperSupport::configuration

}  // end of namespace tfel::gui
