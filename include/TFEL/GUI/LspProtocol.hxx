/*!
 * \file  LspProtocol.hxx
 * \brief pure helpers of the Language Server Protocol client: message
 *        framing, URIs, positions, incremental text synchronization.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPPROTOCOL_HXX
#define LIB_TFEL_GUI_LSPPROTOCOL_HXX

#include <QtCore/QByteArray>
#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <functional>
#include <optional>
#include "TFEL/GUI/Config.hxx"

// forward declaration
class QTextDocument;

namespace tfel::gui {

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

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPPROTOCOL_HXX */
