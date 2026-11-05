/*!
 * \file  LspProtocol.hxx
 * \brief pure helpers of the Language Server Protocol client: message
 *        framing, URIs, positions, incremental text synchronization, and the
 *        plain data the answers are turned into: diagnostics, locations,
 *        edits of a workspace edit, signatures, code actions, the symbols of
 *        a file, the legend of the semantic tokens and their answer.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPPROTOCOL_HXX
#define LIB_TFEL_GUI_LSPPROTOCOL_HXX

#include <QtCore/QByteArray>
#include <QtCore/QJsonObject>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <functional>
#include <optional>
#include "TFEL/GUI/Config.hxx"

// forward declaration
class QTextDocument;

namespace tfel::gui {

  // forward declarations: the data of the protocol, defined below, which the
  // helpers of `LspProtocol` turn in and out of json
  struct LspDiagnostic;
  struct LspTextEdit;

  /*!
   * \brief helpers of the Language Server Protocol, without any state nor any
   * signal: what the pure part of the protocol is, the tests can exercise
   * without a server
   *
   * \note positions are UTF-16 code units, as the Language Server Protocol
   * specifies them and as a `QTextDocument` stores its text: a column of a
   * line is a number of `QChars`, so the mapping is one to one, surrogate
   * pairs included. clangd, of the versions this project supports, only
   * speaks the UTF-16 encoding anyway
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspProtocol {
    /*!
     * \brief a message of the protocol, framed as the standard input of a
     * server expects it: the compact json, preceded by its length in bytes
     */
    static QByteArray encode(const QJsonObject &);
    /*!
     * \brief extract the complete messages held by an incoming byte stream
     * \param[in,out] buffer : bytes read from the server; the complete
     * messages are removed from it, a partial one is left for the next read
     * \return the messages, in the order they were framed. A message which is
     * framed but is not a json object is dropped, not returned
     */
    static QVector<QJsonObject> decode(QByteArray &buffer);
    //! \return the `file:` uri of a file name
    static QString uriOf(const QString &file);
    //! \return the file name a `file:` uri designates
    static QString fileOfUri(const QString &uri);
    /*!
     * \return the position of the protocol, `{"line":..,"character":..}`,
     * for a position of a document; the character count is in UTF-16 code
     * units of the line the position belongs to
     */
    static QJsonObject positionOf(const QTextDocument &, int position);
    /*!
     * \return the position of a document for a position of the protocol; a
     * line beyond the last one, or a column beyond the end of the line, is
     * clamped, as a server which has not seen the latest change may very well
     * answer with a position the document has grown past
     */
    static int positionOf(const QTextDocument &, int line, int character);
    /*!
     * \return the whole text of a document, the blocks joined by `'\n'`
     * \note `toRawText` can not be used: it turns the line separators a block
     * may hold into line feeds too, which would move the offsets of the rest
     * of the line
     */
    static QString textOf(const QTextDocument &);
    /*!
     * \brief a change of the text: the range of the old text it replaces,
     * given in the coordinates of the protocol, and the new text
     */
    struct Change {
      QJsonObject range;
      QString text;
    };
    /*!
     * \return the smallest change turning `old_text` into `new_text`, as a
     * common prefix and a common suffix stripped off, or nothing when the two
     * texts are equal
     *
     * \note the range is that of the old text, which is what
     * `textDocument/didChange` of an incrementally synchronized document
     * asks for: the server applies the change to the text it holds, which is
     * the snapshot, not the one the editor holds
     */
    static std::optional<Change> diffAgainst(const QString &old_text,
                                             const QString &new_text);
    /*!
     * \return the range of a plain, `'\n'` separated text, as the protocol
     * describes it
     */
    static QJsonObject rangeInText(const QString &text, int start, int end);
    /*!
     * \brief run a nested event loop until the stop function the dispatcher
     * is handed is called, or until the timeout expires
     * \param[in] dispatch : called with the stop function; it shall send the
     * request whose answer is waited for
     * \param[in] timeout_ms : longest the wait can last
     * \return true if the stop function was called before the timeout
     *
     * \note this is how the editor turns the asynchronous protocol into the
     * synchronous answer a key press wants: completion, hover and definition
     * block the command, not the application, for at most the timeout
     *
     * \note the stop function may be copied into a callback which outlives
     * the wait, when the answer arrives after the timeout expired; calling it
     * then is harmless
     */
    static bool boundedWait(
        const std::function<void(const std::function<void()> &)> &,
        int timeout_ms);
    /*!
     * \brief the edits of a `WorkspaceEdit`, gathered under the files they
     * apply to, whichever of the two shapes the server answered with
     * \param[in] workspace_edit: the `edit` of a code action, or the whole
     * answer of a rename
     *
     * \note the `documentChanges` of the richer shape are reduced to their
     * plain text edits: a whole document-syncing operation, the creation of a
     * file for instance, is none of this editor's business and is skipped
     */
    static QMap<QString, QVector<LspTextEdit>> editsOf(const QJsonValue &);
    /*!
     * \return the diagnostic as the protocol writes it
     *
     * \note a `codeAction` question is answered with the fixes of the
     * diagnostics it is handed, so the ones the client holds are sent back as
     * they came, ranges and severity included
     */
    static QJsonObject jsonOf(const LspDiagnostic &);
    /*!
     * \brief the range of the protocol, from the corners in its coordinates
     */
    static QJsonObject rangeOf(int start_line,
                               int start_character,
                               int end_line,
                               int end_character);
  };  // end of struct LspProtocol

  /*!
   * \brief a diagnostic of the protocol, as the server published it, with the
   * range in the coordinates of the protocol
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspDiagnostic {
    //! severity, as the protocol defines it: 1 is an error, 2 a warning
    int severity = 1;
    //! message of the server
    QString message;
    //! name of the checker, `clang` or `clangd`, empty if none was given
    QString source;
    int startLine = 0;
    int startCharacter = 0;
    int endLine = 0;
    int endCharacter = 0;
  };  // end of struct LspDiagnostic

  /*!
   * \brief a place a definition or a reference lives: a file and a position
   * of the protocol
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspLocation {
    QString file;
    int line = 0;
    int character = 0;
  };  // end of struct LspLocation

  /*!
   * \brief an edit of a `WorkspaceEdit`: the range it replaces, in the
   * coordinates of the protocol, and the text it replaces it with
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspTextEdit {
    int startLine = 0;
    int startCharacter = 0;
    int endLine = 0;
    int endCharacter = 0;
    QString text;
  };  // end of struct LspTextEdit

  /*!
   * \brief a parameter of a signature of the protocol
   *
   * The protocol names a parameter either with its own text or with the range
   * of the label of its signature which spells it out; both are kept, the
   * offsets being what lets the parameter be picked out of the label
   *
   * \note the offsets are those of the protocol: UTF-16 code units, counted
   * from the start of the label of the signature, which the offsets of
   * \c QString agree with
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspParameterInfo {
    //! the text of the parameter, as the server spelled it, if it spelled one
    QString text;
    //! first offset in the label of the signature, \c -1 when none was sent
    int start = -1;
    //! past the last offset, \c -1 when none was sent
    int end = -1;
  };  // end of struct LspParameterInfo

  /*!
   * \brief one candidate of the signature help of the protocol: the call as
   * the server writes it, and its parameters
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspSignature {
    //! the whole call, e.g. `store(int a, double b) -> void`
    QString label;
    //! the parameters, in the order they are called
    QVector<LspParameterInfo> parameters;
  };  // end of struct LspSignature

  /*!\brief one entry of the overview of a file, in the order the file spells
   * the symbols and flattened: the tree a server answers with is walked
   * depth-first and each symbol is qualified by the scopes it lives in
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspDocumentSymbol {
    //! the name, qualified by its enclosing scopes: `Widget::paint`
    QString name;
    /*!
     * \brief what the server says of the symbol besides its name: the
     * signature of a function, the kind of a class, empty when it says nothing
     */
    QString detail;
    //! the `SymbolKind` of the protocol
    int kind = 0;
    //! line of the name itself, in the coordinates of the protocol
    int line = 0;
    //! offset of the name in its line, in the coordinates of the protocol
    int character = 0;
  };  // end of struct LspDocumentSymbol

  /*!\brief the entries of the overview of a file out of either shape of answer
   * a `textDocument/documentSymbol` may take: the tree of `DocumentSymbol`s or
   * the flat list of `SymbolInformation`s
   * \param[in] result: the `result` of the answer
   * \return the symbols in the order the file spells them, each qualified by
   * the scopes it lives in
   */
  TFEL_GUI_VISIBILITY_EXPORT QVector<LspDocumentSymbol> documentSymbolsOf(
      const QJsonValue &result);

  /*!\brief the kind of a symbol, spelled out
   * \param[in] kind: the `kind` the server sent, a `SymbolKind` of the protocol
   * \return the name of the kind: `function`, `class`, `variable`, ...
   */
  TFEL_GUI_VISIBILITY_EXPORT QString symbolKindName(int kind);

  /*!\brief what a server says the semantic tokens it answers are named after
   *
   * \note the legend is announced at the handshake, not carried by the answers:
   * a token is a number into \c types and a bit mask into \c modifiers, so the
   * answers are meaningless without the legend they were written against
   *
   * \note a server may name several of its kinds with the same word: `clangd`
   * answers `variable` for a local, a global and a member, which are three
   * different entries of its legend, and `type` for an alias, a built-in and a
   * class. Whoever maps kinds to faces goes by the word and loses that
   * distinction, which is what a restrained set of faces does on purpose
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspSemanticLegend {
    //! the names of the kinds, in the numbering the answers use
    QStringList types;
    //! the names of the modifiers, in the order their bits are numbered
    QStringList modifiers;
    //! \return true when a legend was announced at all
    bool isValid() const;
  };  // end of struct LspSemanticLegend

  /*!
   * \brief a run of text a server says is one kind of thing: a local, a type,
   * a macro..., with what it says of it: declared, deprecated, static...
   *
   * \note the names, not the numbers, of the legend: the answer carries the
   * numbers, the reader of an answer wants the names, and the legend is what
   * turns one into the other, see \c semanticTokensOf
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspSemanticToken {
    //! first line of the run, in the coordinates of the protocol
    int line = 0;
    //! first character of the run, in the coordinates of the protocol
    int character = 0;
    /*!
     * \brief length of the run, in the coordinates of the protocol, which is
     * where the coordinates of an extra selection are not: the offsets the
     * text edits draw with count the characters of a `QString`, the protocol
     * counts the UTF-16 units it was written with, see
     * \c LspProtocol::positionOf
     */
    int length = 0;
    //! the kind of the run, named by the legend
    QString type;
    //! what the server says of it, named by the legend
    QStringList modifiers;
  };  // end of struct LspSemanticToken

  /*!\brief the legend of the semantic tokens a server announced
   * \param[in] capabilities: the `capabilities` of the answer of `initialize`
   * \return the legend, invalid when the server offers no semantic tokens
   *
   * \note a server which supports the question answers an object; one which
   * does not answer nothing, or an object without a legend, which is as good
   * as nothing: an answer of tokens could not be read without it
   */
  TFEL_GUI_VISIBILITY_EXPORT LspSemanticLegend
  semanticLegendOf(const QJsonValue &capabilities);

  /*!\brief the tokens of the answer of `textDocument/semanticTokens/full`
   * \param[in] result: the `result` of the answer
   * \param[in] legend: the legend the server announced at the handshake
   * \return the tokens, in the order the server answered them, which is the
   * order of the file, named by the legend
   *
   * \note the answer is a flat array of five numbers per token, the line and
   * the place of the token written as what they differ from those of the
   * previous one, which is what makes an answer cheap to send and expensive to
   * read by hand
   *
   * \note a `result` which is `null`, or names no `data`, is a server which had
   * nothing to say about the file, and is an empty list of tokens
   */
  TFEL_GUI_VISIBILITY_EXPORT QVector<LspSemanticToken> semanticTokensOf(
      const QJsonValue &result, const LspSemanticLegend &legend);

  /*!\brief an action the server offers on a range: a quick fix of a
   * diagnostic, a refactor of the code there
   *
   * \note an action whose \c edits are empty is one the editor can not apply:
   * `clangd` answers its AST tweaks — extract a variable, add an override —
   * with a `clangd.applyTweak` command and no text at all, since the change is
   * meant to be computed by a second request the server would answer with
   * `workspace/applyEdit`. Such an action is reported by its title and left
   * alone, which is honest, rather than offered as an entry doing nothing
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspCodeAction {
    //! what the server calls it, e.g. `insert ';'`
    QString title;
    //! `quickfix`, `refactor`, `info`, `source`, or a dotted sub-kind thereof
    QString kind;
    //! true when the server ranks this the fix one would want
    bool is_preferred = false;
    //! the files it changes and the edits of each, empty when not applicable
    QMap<QString, QVector<LspTextEdit>> edits;
    //! the title of the server-side command it came with, empty if none
    QString command;
    /*!\brief the diagnostic this action fixes, empty when it fixes none, which
     * is what a `quickfix` answers to
     */
    QString diagnostic;
  };  // end of struct LspCodeAction

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPPROTOCOL_HXX */
