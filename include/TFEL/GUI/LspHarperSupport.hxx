/*!
 * \file  LspHarperSupport.hxx
 * \brief the English grammar side of the Language Server Protocol: finding
 *        `harper-ls`, and what a session of it is told about the text.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPHARPER_SUPPORT_HXX
#define LIB_TFEL_GUI_LSPHARPER_SUPPORT_HXX

#include <QtCore/QJsonValue>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief where the grammar checker is, and which English it reads the text as
   *
   * \note the settings live in `QSettings`, as the rest of the editor:
   * `harper/executable` (empty by default: the program is looked for in the
   * directories of the session), `harper/launch-by-default` (false: a buffer of
   * English starts no session on its own) and `harper/dialect` ("American": the
   * variety the text is read as)
   *
   * \note what the checker is started with beyond its dialect — the severity of
   * its lints, the size of a file it still reads, how long it waits for the
   * text to settle — is left to its own defaults: the editor displays those
   * defaults as they come, and an entry for each of them would be an entry
   * nobody can check the effect of from the editor
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspHarperSupport {
    /*!
     * \return the program of the grammar checker: the setting
     * `harper/executable`, else `harper-ls` in the directories of the session;
     * an empty string when there is none, which is what keeps the entries of a
     * grammar menu from being offered for a program that is not there
     *
     * \note an absolute path of the setting is taken as it stands, a bare name
     * is looked for as the default one is: a relative answer would otherwise be
     * read against whatever directory the editor was started in
     */
    static QString serverExecutable();
    /*!
     * \return the setting `harper/launch-by-default`, false when unset: whether
     * a buffer takes a grammar session with it as it is opened
     *
     * \note the entry of a menu which launches a session writes this address,
     * whichever mode it is the menu of: the state of the checker is one
     * decision, made once, and a buffer opened afterwards follows it. The
     * setting is what the entry of the `C++` mode says it keeps, and it is what
     * the configuration dialog edits
     */
    static bool launchByDefault();
    /*!
     * \brief store the setting `harper/launch-by-default`
     * \param[in] on : true when a buffer is to take a session with it
     *
     * \note false removes the address: a configuration file holds what was
     * chosen, not what the editor would have answered anyway
     */
    static void setLaunchByDefault(const bool on);
    /*!
     * \return the variety of English a session reads the text as, from the
     * setting `harper/dialect`: "American" when unset or when the stored name
     * names no variety the checker knows
     */
    static QString dialect();
    /*!
     * \brief store the setting `harper/dialect`
     * \param[in] name : one of the names of \c dialects
     *
     * \note "American", and a name which is none of them, remove the address
     */
    static void setDialect(const QString &name);
    /*!
     * \return the names of the varieties a session can read the text as, in the
     * order they are offered: "American", "Canadian", "Australian", "British",
     * "Indian"
     *
     * \note those are the names the server reads in its settings; the first one
     * is the default of the server itself, and the one a text holding no mark
     * of another variety is read as
     *
     * \note the five differ by the spellings they hold to be their own: a
     * document spelled in one of the four varieties which are not the default
     * is full of lints until it is read as the variety it is spelled in, which
     * is the whole use of the choice
     */
    static QStringList dialects();
    /*!
     * \return the name a variety is offered to a person under: "American
     * English" for "American", and the same shape for the others
     * \param[in] name : one of the names of \c dialects
     */
    static QString dialectLabel(const QString &name);
    /*!
     * \return the arguments a grammar session runs with: `--stdio`, which the
     * server needs to speak over the pipes the client talks through; without it
     * it serves a local socket and reads nothing at all
     *
     * \note the dialect and the rest of the settings go in the answers the
     * client gives the server, not on its command line, so these arguments are
     * nobody's choice and no address holds them
     */
    static QStringList serverArguments();
    /*!
     * \return the settings a grammar session is answered with: an object whose
     * one key is the name of the server, `harper-ls`, holding the dialect it is
     * to read the text as
     *
     * \note the server wants its settings under its own name and says so when
     * they are not: `Settings must contain a "harper-ls" key`. And it asks them
     * again before every analysis, so a dialect which changed is the one the
     * next change of a document is read with — see
     * \c LspClient::recheckDocuments, which is what makes a document which
     * changed by itself not at all be read again
     */
    static QJsonValue configuration();
  };  // end of struct LspHarperSupport

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPHARPER_SUPPORT_HXX */
