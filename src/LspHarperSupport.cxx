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
#include "TFEL/GUI/SettingsKey.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"

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

  namespace {

    // the five varieties of English, offered under the names a person reads and
    // kept under the names the server reads them
    QVector<SettingsKey::Choice> dialectChoices() {
      auto choices = QVector<SettingsKey::Choice>();
      for (const auto &name : LspHarperSupport::dialects()) {
        choices << SettingsKey::Choice(
            name, QObject::tr("%1 English").arg(name));
      }
      return choices;
    }  // end of dialectChoices()

  }  // end of anonymous namespace

  // the checker is a program of its own and the theme is named after it, the way
  // the theme of `clangd` is: that is the name a person looks for when the
  // underlines of a buffer are not what they expected. The `C++` mode declared
  // this theme while it was the only mode to hold a grammar session; it is a
  // theme of the editor now, because a document of LaTeX, a note of Markdown and
  // the comments of a source file are read by the same program, and what reads a
  // buffer is one choice for the editor, see `SpellingAndGrammar`. Whether a
  // buffer is read at all is an entry of that choice and no entry of this theme;
  // what is left here are the two entries which say which program runs and which
  // English it reads
  //
  // The theme is a submenu of `Spelling and grammar`: which program checks a
  // text is one decision, and which program was chosen then has to say what it
  // is asked with. It is spelled `Harper`, as the program is spelled, whatever
  // the address of its entries is
  static const StandardSettingsGroupProxy harper(
      "Harper",
      QObject::tr(
          "The English grammar checker a buffer can be read with: the text it "
          "is given is underlined where it finds something wrong. The checker "
          "knows no word of French, and it reads the whole text it is handed, "
          "which is to say the comments of a source file"),
      QVector<SettingsKey>()
          << SettingsKey::text(
                 "harper/executable",
                 QObject::tr("Program of the grammar checker"),
                 QObject::tr("The `harper-ls` to start. Left empty, the editor "
                             "looks for `harper-ls` in the directories of the "
                             "session; when there is none, no menu offers a "
                             "checker which names this program at all"))
          << SettingsKey::choice(
                 "harper/dialect", QObject::tr("Variety of English to read as"),
                 QObject::tr("Which English the text is read as: which spellings "
                             "are held to be their own — `colour` and `color` do "
                             "not both fit one variety — and which inflections "
                             "are accepted. The language submenu of the "
                             "`Hunspell and Harper` entry of `Options > "
                             "Orthograph/Grammar` writes this same address, and a "
                             "checker which runs reads its buffers again under "
                             "the new answer"),
                 dialectChoices(), QStringLiteral("American")),
      QStringLiteral("Spelling and grammar"));

}  // end of namespace tfel::gui
