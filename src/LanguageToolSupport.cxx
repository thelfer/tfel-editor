/*!
 * \file   LanguageToolSupport.cxx
 * \brief  finding LanguageTool, the request its server reads, the answer it
 *         sends back.
 * \author Thomas Helfer
 * \date   13 nov. 2026
 */

#include <algorithm>
#include <cstdlib>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QStandardPaths>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include "TFEL/GUI/LanguageToolSupport.hxx"
#include "TFEL/GUI/SettingsKey.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"

namespace tfel::gui {

  namespace {

    // the class the Java runtime is asked to run: the HTTP server of the
    // distribution. It is named here and nowhere else, since it is the one thing
    // about the program which no setting can change: where the jar is and what
    // port it serves on are the two choices a person has, see the theme of the
    // configuration dialog
    const char *const server_class = "org.languagetool.server.HTTPServer";

    // the jar of a distribution; the file `java -jar` would be started with, and
    // whose manifest names the libraries of a `libs` sub-directory
    const char *const server_jar = "languagetool-server.jar";

    // the languages the menus offer, as pairs of a code and of the name it is
    // offered under. The six varieties of English come first — the same six a
    // person chooses of `harper-ls`, spelled with the codes this server reads —
    // and the rest of the list follows the languages the program counts as its
    // own, in the order of its documentation.
    //
    // A code is what the request carries; the label is what a person reads. The
    // full list of a running server is longer than anybody wants to scroll
    // through in a menu, and the setting `language-tool/language` takes any code
    // of it, so this list is a convenience and no boundary
    struct KnownLanguage {
      const char *code;
      const char *label;
    };

    const KnownLanguage known_languages[] = {
        {"auto", QT_TR_NOOP("Auto-detected language")},
        {"en-US", QT_TR_NOOP("American English")},
        {"en-GB", QT_TR_NOOP("British English")},
        {"en-CA", QT_TR_NOOP("Canadian English")},
        {"en-AU", QT_TR_NOOP("Australian English")},
        {"en-NZ", QT_TR_NOOP("New Zealand English")},
        {"en-ZA", QT_TR_NOOP("South African English")},
        {"de-DE", QT_TR_NOOP("German")},
        {"de-AT", QT_TR_NOOP("German (Austria)")},
        {"de-CH", QT_TR_NOOP("German (Switzerland)")},
        {"de-DE-x-simple-language", QT_TR_NOOP("Simple German")},
        {"fr", QT_TR_NOOP("French")},
        {"fr-CA", QT_TR_NOOP("French (Canada)")},
        {"fr-CH", QT_TR_NOOP("French (Switzerland)")},
        {"es", QT_TR_NOOP("Spanish")},
        {"ca", QT_TR_NOOP("Catalan")},
        {"it-IT", QT_TR_NOOP("Italian")},
        {"pt-PT", QT_TR_NOOP("Portuguese")},
        {"pt-BR", QT_TR_NOOP("Portuguese (Brazil)")},
        {"nl", QT_TR_NOOP("Dutch")},
        {"pl-PL", QT_TR_NOOP("Polish")},
        {"ru-RU", QT_TR_NOOP("Russian")},
        {"uk-UA", QT_TR_NOOP("Ukrainian")},
        {"sk-SK", QT_TR_NOOP("Slovak")},
        {"sv-SE", QT_TR_NOOP("Swedish")},
        {"el-GR", QT_TR_NOOP("Greek")},
        {"eo", QT_TR_NOOP("Esperanto")},
        {"fa-IR", QT_TR_NOOP("Persian")},
        {"ja-JP", QT_TR_NOOP("Japanese")},
        {"zh-CN", QT_TR_NOOP("Chinese")},
    };

    // the places a jar of the program is looked for when nothing was written:
    // the two variables which name an installation, and the directories a
    // distribution of the libraries puts it in. The program is a download and
    // not a package of this platform, so the honest hope is the variable, and
    // the directories are there for the people who unpacked it in one of them
    QStringList searchDirectories() {
      auto directories = QStringList();
      for (const auto *name : {"LANGUAGETOOL_HOME", "LANGUAGETOOLHOME"}) {
        const auto value = qEnvironmentVariable(name).trimmed();
        if (!value.isEmpty()) {
          directories << value;
        }
      }
      directories << QStringLiteral("/usr/share/languagetool")
                  << QStringLiteral("/usr/local/share/languagetool")
                  << QStringLiteral("/opt/languagetool");
      return directories;
    }  // end of searchDirectories()

    // the jar of an installation: the directory itself when it is a jar, the jar
    // of the distribution in it else, the one of the sub-directory a Debian
    // package unpacks the components to else, empty when the directory holds
    // none
    QString jarInDirectory(const QString &directory) {
      const QFileInfo information(directory);
      if (!information.exists()) {
        return {};
      }
      if (information.isFile() && information.suffix() == QLatin1String("jar")) {
        return information.absoluteFilePath();
      }
      if (!information.isDir()) {
        return {};
      }
      const auto root = information.absoluteFilePath();
      for (const auto *sub : {"", "components", "share/languagetool"}) {
        const auto candidate = QString::fromLatin1(sub).isEmpty()
                                   ? root + QLatin1Char('/') +
                                         QLatin1String(server_jar)
                                   : root + QLatin1Char('/') +
                                         QString::fromLatin1(sub) +
                                         QLatin1Char('/') +
                                         QLatin1String(server_jar);
        if (QFileInfo::exists(candidate)) {
          return QFileInfo(candidate).absoluteFilePath();
        }
      }
      return {};
    }  // end of jarInDirectory()

  }  // end of anonymous namespace

  QString LanguageToolSupport::javaRuntimeAddress() {
    return QStringLiteral("language-tool/java-runtime");
  }  // end of LanguageToolSupport::javaRuntimeAddress

  QString LanguageToolSupport::javaRuntime() {
    const auto configured = QSettings()
                                .value(javaRuntimeAddress())
                                .toString()
                                .trimmed();
    if (!configured.isEmpty()) {
      // an absolute path is taken as it stands; a bare name, `java-17` for
      // instance, is looked for as the default one is
      return QFileInfo::exists(configured)
                 ? configured
                 : QStandardPaths::findExecutable(configured);
    }
    const auto runtime = QStandardPaths::findExecutable(QStringLiteral("java"));
    if (!runtime.isEmpty()) {
      return runtime;
    }
    // no `java` in the directories of the session: a JDK which is not in the
    // path but is installed is named by JAVA_HOME, which every installer of a
    // JDK sets, and whose bin directory holds the runtime of the version that
    // was installed
    const auto home = qEnvironmentVariable("JAVA_HOME").trimmed();
    if (!home.isEmpty()) {
      const auto candidate = home + QLatin1String("/bin/java");
      if (QFileInfo::exists(candidate)) {
        return QFileInfo(candidate).absoluteFilePath();
      }
    }
    return {};
  }  // end of LanguageToolSupport::javaRuntime

  QString LanguageToolSupport::installationAddress() {
    return QStringLiteral("language-tool/installation");
  }  // end of LanguageToolSupport::installationAddress

  QString LanguageToolSupport::serverJar() {
    const auto configured = QSettings()
                                .value(installationAddress())
                                .toString()
                                .trimmed();
    if (!configured.isEmpty()) {
      // an absolute answer is taken as it stands; a relative one is read
      // against the directories the editor was started in only as a last
      // resort, which is what QFileInfo::exists does to it below
      const auto jar = jarInDirectory(configured);
      if (!jar.isEmpty()) {
        return jar;
      }
    }
    for (const auto &directory : searchDirectories()) {
      const auto jar = jarInDirectory(directory);
      if (!jar.isEmpty()) {
        return jar;
      }
    }
    return {};
  }  // end of LanguageToolSupport::serverJar

  QStringList LanguageToolSupport::serverArguments(const QString &jar,
                                                   const int port) {
    auto arguments = javaOptions();
    // the class path: the jar, and the libraries beside it when they are there.
    //
    // The manifest of the jar of a distribution names its libraries, and the
    // Java runtime reads those names against the directory of the jar, so the
    // jar alone is enough to start the server from any directory. The wildcard
    // is added for two cases: a jar of a build tree, whose manifest names
    // nothing, and a distribution whose `libs` directory was renamed — in both
    // cases the runtime needs the jars named on its own command line. A
    // duplicate entry of the class path is not an error to the runtime, and the
    // wildcard of a directory the manifest already names costs nothing
    auto class_path = QStringList{jar};
    const auto libraries = QFileInfo(jar).absolutePath() + QLatin1String("/libs");
    if (QFileInfo(libraries).isDir()) {
      class_path << libraries + QLatin1String("/*");
    }
    arguments << QStringLiteral("-cp") << class_path.join(QLatin1Char(':'))
              << QLatin1String(server_class)
              << QStringLiteral("--port") << QString::number(port);
    return arguments;
  }  // end of LanguageToolSupport::serverArguments

  int LanguageToolSupport::port() {
    // 0 is the answer of the setting which says 'take any free port': what the
    // server answers on is then what the client reads, see
    // LanguageToolClient::startServer
    return QSettings().value(QStringLiteral("language-tool/port"), 0).toInt();
  }  // end of LanguageToolSupport::port

  bool LanguageToolSupport::isAvailable() {
    return (!javaRuntime().isEmpty()) && (!serverJar().isEmpty());
  }  // end of LanguageToolSupport::isAvailable

  QString LanguageToolSupport::language() {
    const auto code =
        QSettings().value(QStringLiteral("language-tool/language")).toString();
    return languages().contains(code) ? code : QStringLiteral("en-US");
  }  // end of LanguageToolSupport::language

  void LanguageToolSupport::setLanguage(const QString &code) {
    auto settings = QSettings();
    if ((code == QStringLiteral("en-US")) || (!languages().contains(code))) {
      // "en-US" is what the editor asks for by default and what a stored value
      // of it says no more than its absence; a code which is none of the known
      // ones is no choice either, and keeping it would have the next request
      // answered with an error nobody can select again
      settings.remove(QStringLiteral("language-tool/language"));
      return;
    }
    settings.setValue(QStringLiteral("language-tool/language"), code);
  }  // end of LanguageToolSupport::setLanguage

  QStringList LanguageToolSupport::languages() {
    auto codes = QStringList();
    for (const auto &known : known_languages) {
      codes << QLatin1String(known.code);
    }
    return codes;
  }  // end of LanguageToolSupport::languages

  QString LanguageToolSupport::languageLabel(const QString &code) {
    for (const auto &known : known_languages) {
      if (code == QLatin1String(known.code)) {
        return QObject::tr(known.label);
      }
    }
    // a code of the setting which is none of the known ones is shown as what it
    // is: whoever wrote it knows what it means, and inventing a name for it in
    // the editor would be a claim about a language nobody here read
    return code;
  }  // end of LanguageToolSupport::languageLabel

  QStringList LanguageToolSupport::javaOptions() {
    return QSettings()
        .value(QStringLiteral("language-tool/java-options"))
        .toStringList();
  }  // end of LanguageToolSupport::javaOptions

  QStringList LanguageToolSupport::enabledCategories() {
    return QSettings()
        .value(QStringLiteral("language-tool/enabled-categories"))
        .toStringList();
  }  // end of LanguageToolSupport::enabledCategories

  QStringList LanguageToolSupport::disabledCategories() {
    return QSettings()
        .value(QStringLiteral("language-tool/disabled-categories"))
        .toStringList();
  }  // end of LanguageToolSupport::disabledCategories

  QByteArray LanguageToolSupport::requestBody(
      const QString &text,
      const QString &language,
      const QStringList &enabled_categories,
      const QStringList &disabled_categories) {
    // the form the API reads, percent encoded in UTF-8: the offsets of the
    // answer are UTF-16 code units of the text which was sent, the same units a
    // QString counts in, so a text of accents, of quotes or of an emoji comes
    // back with its errors where the buffer holds them
    auto query = QUrlQuery();
    query.addQueryItem(QStringLiteral("language"), language);
    query.addQueryItem(QStringLiteral("text"), text);
    if (!enabled_categories.isEmpty()) {
      query.addQueryItem(QStringLiteral("enabledCategories"),
                         enabled_categories.join(QLatin1Char(',')));
    }
    if (!disabled_categories.isEmpty()) {
      query.addQueryItem(QStringLiteral("disabledCategories"),
                         disabled_categories.join(QLatin1Char(',')));
    }
    // `FullyEncoded` is the name Qt 6.2 gives to the encoding a form is read
    // with: every character outside the unreserved set becomes `%XX`, which is
    // what `curl --data-urlencode` writes, byte for byte. The other reading of
    // the content type turns a space into a `+` and leaves the rest alone, and a
    // text full of newlines is not worth betting on
    return query.query(QUrl::FullyEncoded).toUtf8();
  }  // end of LanguageToolSupport::requestBody

  int LanguageToolSupport::maximumTextLength() {
    // measured on a server of the version this editor was developed against:
    // 40 000 characters of English are checked in about a tenth of a second,
    // 60 000 in two seconds, and the answer of a whole document is a single
    // request as long as it fits
    return 40000;
  }  // end of LanguageToolSupport::maximumTextLength

  QVector<int> LanguageToolSupport::sliceStarts(const QString &text) {
    auto starts = QVector<int>{0};
    const auto maximum = maximumTextLength();
    if (text.isEmpty()) {
      return starts;
    }
    auto start = 0;
    while (text.size() - start > maximum) {
      // the cut is looked for in the last half of the piece, so that the pieces
      // stay of a like size: a paragraph which would run over the whole limit is
      // a text with no empty line near it, and the second best is the end of a
      // line
      auto cut = -1;
      const auto limit = start + maximum;
      const auto floor = start + maximum / 2;
      for (auto i = text.lastIndexOf(QStringLiteral("\n\n"), limit); i >= floor;
           i = text.lastIndexOf(QStringLiteral("\n\n"), i - 1)) {
        // the piece ends past the blank line: the empty line itself belongs to
        // the piece it closes, which is what keeps a paragraph whole
        cut = i + 2;
        break;
      }
      if (cut < 0) {
        for (auto i = text.lastIndexOf(QLatin1Char('\n'), limit); i >= floor;
             i = text.lastIndexOf(QLatin1Char('\n'), i - 1)) {
          cut = i + 1;
          break;
        }
      }
      if (cut < 0) {
        // a text of one line longer than a piece: cut where the piece ends, the
        // checker reads the two halves as two sentences and says nothing of the
        // run between them, which is the price of a text with no break in it
        cut = limit;
      }
      starts << cut;
      start = cut;
    }
    return starts;
  }  // end of LanguageToolSupport::sliceStarts

  QVector<LspDiagnostic> LanguageToolSupport::diagnosticsOf(
      const QJsonArray &matches, const QString &text, const int offset) {
    auto diagnostics = QVector<LspDiagnostic>();
    for (const auto &match : matches) {
      const auto object = match.toObject();
      // the two corners of the answer are a place in the text which was sent and
      // a length, both in UTF-16 code units; `offset` is where the piece which
      // was sent stands in the whole text, and the whole text is what the
      // ranges are computed against, so a piece which cut a line in two cannot
      // answer a range on a line the buffer has not
      const auto start = offset + object.value(QStringLiteral("offset")).toInt();
      const auto length = object.value(QStringLiteral("length")).toInt();
      // A report which names no word of the text the checker was handed is a
      // report about what the editor wrote there, and not about the text of the
      // buffer. A mode which hides the markup of a line leaves a run of blanks of
      // the same length in its place, and LanguageTool reads a run of blanks as a
      // repeated whitespace — `WHITESPACE_RULE`, of the category `TYPOGRAPHY` —
      // so the eight spaces standing for `\section` are answered as a typo, on a
      // place of the buffer of which the checker was handed nothing. The reports
      // a reader wants all name the word they are about: a missing space after a
      // period is answered on the word which lacks it, `an tests.It` at an offset
      // of 17 and a length of 2 which is `It` (verified on 6.4). And a
      // typographical detail of a *source* file is nothing in the text it is
      // rendered to, since a renderer collapses the spaces of its own accord.
      const auto named = text.mid(start, length);
      if (!std::any_of(named.cbegin(), named.cend(),
                       [](const QChar &c) { return c.isLetterOrNumber(); })) {
        continue;
      }
      const auto range =
          LspProtocol::rangeInText(text, start, start + length);
      const auto first = range.value(QStringLiteral("start")).toObject();
      const auto last = range.value(QStringLiteral("end")).toObject();
      auto diagnostic = LspDiagnostic();
      // the message, and nothing else: LanguageTool writes a sentence which says
      // what is wrong and what to put in its place, `Use "an" instead of 'a' if
      // the following word starts with a vowel sound`, and that is the tooltip
      // of the underline
      diagnostic.message = object.value(QStringLiteral("message")).toString();
      const auto rule = object.value(QStringLiteral("rule")).toObject();
      diagnostic.code = rule.value(QStringLiteral("id")).toString();
      diagnostic.source = rule.value(QStringLiteral("category"))
                              .toObject()
                              .value(QStringLiteral("id"))
                              .toString();
      // the severity is left to its default: the server classifies its answers
      // by the category of the rule and by no severity at all, and the grammar
      // checker draws one face for all of them, whatever the answer says
      diagnostic.startLine = first.value(QStringLiteral("line")).toInt();
      diagnostic.startCharacter =
          first.value(QStringLiteral("character")).toInt();
      diagnostic.endLine = last.value(QStringLiteral("line")).toInt();
      diagnostic.endCharacter = last.value(QStringLiteral("character")).toInt();
      if (!diagnostic.message.isEmpty()) {
        diagnostics.append(diagnostic);
      }
    }
    return diagnostics;
  }  // end of LanguageToolSupport::diagnosticsOf

  QString LanguageToolSupport::errorOf(const QByteArray &body) {
    // the sentence of the server is plain text and can be long — the answer to
    // a wrong language code lists every code the program knows, over four
    // thousand characters — so it is cut to what the minibuffer holds
    auto message = QString::fromUtf8(body).trimmed();
    if (message.isEmpty()) {
      return {};
    }
    const auto end = message.indexOf(QLatin1Char('\n'));
    if (end >= 0) {
      message = message.left(end);
    }
    return message.length() > 200 ? message.left(200) + QStringLiteral("...")
                                 : message;
  }  // end of LanguageToolSupport::errorOf

  // where the program is and what it is asked, in a theme named after the program
  // it is about, the way the themes of `clangd` and `harper` are. Whether a buffer
  // is read by this proofreader is none of them: that is the answer of
  // `spelling/checker`, see `SpellingAndGrammar`. What is declared here is the two
  // entries a person has to fill for the program to be found at all — it is a
  // download and not a package of this platform —, the port it is started on, the
  // options the Java runtime is given, and the two lists of rules which decide what
  // of what the program knows is said about a text. No category is named by the
  // editor itself: what the program which was installed considers an error is the
  // answer of that program, and the two lists are where a person who wants less of
  // it writes what to leave out
  static const StandardSettingsGroupProxy language_tool(
      "LanguageTool",
      QObject::tr("Where LanguageTool is installed, the Java runtime it is "
                  "started with, and what of its rules reach a text"),
      QVector<SettingsKey>()
          << SettingsKey::text(
                 "language-tool/installation",
                 QObject::tr("Directory LanguageTool is installed in"),
                 QObject::tr("The directory of the installation, or the file "
                             "`languagetool-server.jar` itself. Left empty, the "
                             "editor looks in `LANGUAGETOOL_HOME`, in "
                             "`LANGUAGETOOLHOME` and in "
                             "`/usr/share/languagetool`, which is as much as "
                             "anything finds of a program nobody installs as a "
                             "package. A directory which holds no such file is "
                             "of no use: the server is a Java program, not a "
                             "command, and a choice whose program is not to be "
                             "found is not even offered in a menu"))
          << SettingsKey::text(
                 "language-tool/java-runtime",
                 QObject::tr("Java runtime used to run the server"),
                 QObject::tr("The `java` to start. Left empty, the editor takes "
                             "the `java` of the directories of the session, or "
                             "the one of `JAVA_HOME`, which is what a machine "
                             "where a Java runtime is installed answers. A bare "
                             "name is looked for as the default one is, so "
                             "`java-17` names a runtime beside the default one"))
          << SettingsKey::integer(
                 "language-tool/port",
                 QObject::tr("Port the server is started on"),
                 QObject::tr("The editor starts the server itself and asks it on "
                             "this port of the machine it runs on. Zero, which is "
                             "the default, means a free port is taken: two "
                             "editors opened at once then run two servers, and "
                             "nothing collides. A number written here is the port "
                             "the editor starts its own server on; a server "
                             "already listening there is not adopted, and the "
                             "start fails with a message naming the port"),
                 0, 0, 65535)
          << SettingsKey::stringList(
                 "language-tool/java-options",
                 QObject::tr("Options given to the Java runtime"),
                 QObject::tr("What is put before the class path of the server, "
                             "such as `-Xmx2g`. The rules of every language the "
                             "program knows are loaded into one virtual machine, "
                             "which is what a bigger heap is for. In the "
                             "configuration file the entries of the list are "
                             "separated by a comma"))
          << SettingsKey::text(
                 "language-tool/language",
                 QObject::tr("Language the text is read as"),
                 QObject::tr("A code the program reads, such as `en-GB` or `fr`; "
                             "`auto` lets the program detect it. `en-US` is what "
                             "is asked as long as nothing is chosen here. The "
                             "language submenu of the `LanguageTool` entry of "
                             "`Options > Orthograph/Grammar` offers thirty of the "
                             "codes and writes this same address; any other code "
                             "the program knows can be written here, and a code it "
                             "knows nothing of is answered with the sentence the "
                             "program says of it"))
          << SettingsKey::stringList(
                 "language-tool/enabled-categories",
                 QObject::tr("Categories of rules to add to what is active"),
                 QObject::tr("Identifiers of the categories of rules the program "
                             "groups its rules under, `TYPOGRAPHY` or "
                             "`SEMANTICS` for instance, separated by a comma in "
                             "the configuration file. What is named here is "
                             "*added* to the rules the program activates by "
                             "itself, and a category which is none of its own is "
                             "answered with a sentence on the minibuffer"),
                 QStringList())
          << SettingsKey::stringList(
                 "language-tool/disabled-categories",
                 QObject::tr("Categories of rules to leave out"),
                 QObject::tr("The identifiers of the categories to take out of the "
                             "answer, `STYLE` or `MISC` for instance. Of the two "
                             "lists of the settings this is the one which is a "
                             "filter: what is named here is gone from the answer, "
                             "whatever else the program makes of the text"),
                 QStringList()),
      QStringLiteral("Spelling and grammar"));

}  // end of namespace tfel::gui
