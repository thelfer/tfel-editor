/*!
 * \file  LanguageToolSupport.hxx
 * \brief the LanguageTool side of the proofreaders a buffer can be read with:
 *        finding the program, what it is asked, and what its answer says.
 * \author Thomas Helfer
 * \date   13 nov. 2026
 */

#ifndef LIB_TFEL_GUI_LANGUAGETOOLSUPPORT_HXX
#define LIB_TFEL_GUI_LANGUAGETOOLSUPPORT_HXX

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/LspProtocol.hxx"

class QJsonArray;

namespace tfel::gui {

  /*!
   * \brief where LanguageTool is and what it is asked to read
   *
   * \note LanguageTool is not a language server: the jar ships an HTTP server,
   * \c org.languagetool.server.HTTPServer, which answers a form request with a
   * JSON body, and no version of it speaks the Language Server Protocol. What
   * this class holds is therefore the same information the client of a server
   * needs — where the program is, what a request is, what an answer says — read
   * and written without any state of its own, so that all of it is testable
   * with no process running
   *
   * \note the settings live in `QSettings`, as the rest of the editor:
   * `language-tool/java-runtime` (empty: `java` of the session, else the one of
   * `JAVA_HOME`), `language-tool/installation` (empty: the directories of the
   * session are searched), `language-tool/port` (0: a free port is taken),
   * `language-tool/language` ("en-US"), `language-tool/java-options`, and the
   * two lists of rules, `language-tool/enabled-categories` and
   * `language-tool/disabled-categories`, both empty by default, which leaves the
   * editor no opinion of its own about what LanguageTool considers an error
   *
   * \note the request is answered with the *default* activation of the rules,
   * which is neither the picky nor the basic set: LanguageTool knows a
   * `level` of activation (`default`, `picky`, `academic`, `clarity`,
   * `professional`, `creative`) and the editor does not name one, since what
   * the person who installed the program wants of it is that program's own
   * answer
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LanguageToolSupport {
    /*!
     * \return the address of the setting which names the Java runtime the
     * server is started with, empty meaning the one of the session
     */
    static QString javaRuntimeAddress();
    /*!
     * \return the Java runtime to start the server with: the setting
     * `language-tool/java-runtime`, else `java` in the directories of the
     * session, else the `bin/java` of `JAVA_HOME`; empty when there is none,
     * which is what keeps the entries of a menu from offering a checker that
     * can not be started
     *
     * \note a bare name of the setting is looked for in the directories of the
     * session, as the default one is, so that a relative answer is not read
     * against the directory the editor happens to run in
     */
    static QString javaRuntime();
    /*!
     * \return the address of the setting which names where LanguageTool is
     * installed
     */
    static QString installationAddress();
    /*!
     * \return the jar to put on the classpath of the server: the setting
     * `language-tool/installation`, which is either the jar itself or the
     * directory holding it, else `languagetool-server.jar` of the directories
     * the program is usually installed in, of `LANGUAGETOOL_HOME` and of
     * `LANGUAGETOOLHOME`; empty when there is none
     *
     * \note a directory is searched for the jar of the distribution —
     * `languagetool-server.jar`, which is a small jar whose manifest names the
     * libraries of a `libs` sub-directory — and, failing that, for the jar of a
     * build tree, whose manifest names nothing and has to be completed by the
     * libraries themselves
     *
     * \note this is the one entry of the configuration a person has to fill by
     * hand for the checker to work at all: the program is a download, not a
     * package, and nothing in the session finds it by itself
     */
    static QString serverJar();
    /*!
     * \return the arguments the Java runtime is started with to serve the
     * checks on the given port: the class path, the options of the setting, the
     * class of the server and its `--port`
     *
     * \note the server is started without `--public`, which is what keeps it
     * reachable from the machine it was started on only, and without a
     * `--config` file: the limits of the server are none of the editor's
     * business and its defaults are the ones the program ships with
     */
    static QStringList serverArguments(const QString &jar, int port);
    /*!
     * \return the setting `language-tool/port`, which is 0 when nothing was
     * chosen: the answer says what the person asked for, not what the server
     * ends up listening on. The free port a 0 stands for is picked by whoever
     * starts the server, see `LanguageToolClient::startServer`, since a port is
     * none's business until a server is asked to stand on it
     */
    static int port();
    /*!
     * \return true when both a Java runtime and a LanguageTool jar are to be
     * found: the entries which start the checker are worth showing only then
     */
    static bool isAvailable();
    /*!
     * \return the language the text is read as, from the setting
     * `language-tool/language`: "en-US" when unset, "auto" for what the server
     * detects on its own
     */
    static QString language();
    /*!
     * \brief store the setting `language-tool/language`
     * \param[in] code : a code of \c languages
     *
     * \note "en-US", and a code which is none of the known ones, remove the
     * address
     */
    static void setLanguage(const QString &code);
    /*!
     * \return the codes the menus offer a language to be read as, in the order
     * they are offered: "auto", then the six varieties of English, then the
     * other languages the server reads
     *
     * \note those are the codes the HTTP API reads in its `language` parameter,
     * spelled as the server spells them. The list is the editor's own: a
     * running server answers the full list of what it knows, which is a longer
     * list than anyone wants to choose in a menu, and a list read from the
     * server would be an empty menu until the server is up
     */
    static QStringList languages();
    /*!
     * \return the name a code is offered to a person under: "American English"
     * for "en-US", "French" for "fr", "Auto-detected language" for "auto"
     * \param[in] code : one of the codes of \c languages
     */
    static QString languageLabel(const QString &code);
    /*!
     * \return the extra options the Java runtime is given, from the setting
     * `language-tool/java-options`, such as `-Xmx2g`
     */
    static QStringList javaOptions();
    /*!
     * \return the categories of rules the setting `language-tool/enabled-categories`
     * names, empty by default
     *
     * \note an enabled category adds a group of rules to what the server
     * activates by itself; it is not a filter, and the server reads the two
     * settings of the editor for what they are: see \c disabledCategories
     */
    static QStringList enabledCategories();
    /*!
     * \return the categories of rules the setting `language-tool/disabled-categories`
     * names, empty by default: the style of a text, the typographical
     * conventions and the fancy rules of a premium install stay in the answer
     * unless a person takes them out
     */
    static QStringList disabledCategories();
    /*!
     * \return the body of a `POST` of the check request, form encoded as the
     * API reads it: the text, the language, and the two lists of categories
     * when they name anything
     *
     * \note the text is encoded as it stands, newlines included: the offsets of
     * the answer are places in the text which was sent, so the text is sent for
     * what it is and nothing else — no trimming, no rewrapping, no markup
     * removed — is what keeps an answer standing where the buffer holds the
     * words it is about
     */
    static QByteArray requestBody(const QString &text,
                                  const QString &language,
                                  const QStringList &enabled_categories,
                                  const QStringList &disabled_categories);
    /*!
     * \brief the longest text a request is worth sending, in characters
     *
     * \note the server answers a long text as well as a short one, but it
     * answers it *after* having read the whole of it: a request which takes ten
     * seconds draws the underlines of a buffer ten seconds after the person
     * stopped typing, and takes the thread of the server with it. The text of a
     * buffer is cut on that length, on a blank of a paragraph if there is one
     * near it, and the answers of the pieces are put back where the whole text
     * has them
     */
    static int maximumTextLength();
    /*!
     * \return the pieces the text is cut into for the requests: each of them is
     * at most \c maximumTextLength characters long and none of them starts in
     * the middle of a line of the text, so that the offsets of their answers are
     * offsets of the whole text once the place of the piece is added to them
     *
     * \note the cut looks for the last empty line before the limit — a paragraph
     * is a whole thought to a checker — then for the last end of a line, and
     * cuts in the middle of a line only for a text which holds no line break
     * over the whole of its first piece
     */
    static QVector<int> sliceStarts(const QString &text);
    /*!
     * \brief the errors of an answer, in the coordinates of the protocol
     * \param[in] matches : the `matches` array of a check answer
     * \param[in] text : the whole text the answer is about, the text of the
     * document, to turn the character offsets of the answer into lines and
     * columns
     * \param[in] offset : where the piece which was sent stands in \c text,
     * which is 0 for a request which was sent the whole text
     *
     * \note the ranges are clamped to \c text by the caller of this function,
     * \c LspProtocol::rangeInText, which is what an answer about a text which
     * moved a little can be trusted with: a range past the end of the text is
     * put at its end, never on the line the buffer happens to have there
     *
     * \note a report whose range holds no letter and no digit is dropped: such a
     * report is about the text the editor wrote, not about the text of the
     * buffer. A mode which hides the markup of a line leaves blanks there, see
     * `MajorMode::grammarCheckText`, and LanguageTool reads a run of blanks as a
     * repeated whitespace
     *
     * \note the `code` of a diagnostic is the identifier of the rule which
     * answered, `EN_A_VS_AN` or `MORFOLOGIK_RULE_EN_US`, which is the name the
     * documentation of LanguageTool spells and the place where a person who
     * thinks the rule wrong looks it up; the `source` is the identifier of the
     * category of that rule, `GRAMMAR` or `TYPOS`
     */
    static QVector<LspDiagnostic> diagnosticsOf(const QJsonArray &matches,
                                                const QString &text,
                                                int offset = 0);
    /*!
     * \return what the body of an answer which is no JSON says of itself, cut to
     * a length which fits the minibuffer
     *
     * \note the server answers a bad request as plain text, not as JSON: a wrong
     * language code is answered `Error: 'xx-XX' is not a language code known to
     * LanguageTool. Supported language codes are: ...`, and that sentence is
     * what the person who typed the code needs to read
     */
    static QString errorOf(const QByteArray &body);
  };  // end of struct LanguageToolSupport

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LANGUAGETOOLSUPPORT_HXX */
