/*!
 * \file  FileDownloader.hxx
 * \brief asynchronous download of the content of a url with Qt.
 * \author Thomas Helfer
 * \date   05/09/2012
 */

#ifndef LIB_TFEL_GUI_FILEDOWNLOADER_HXX
#define LIB_TFEL_GUI_FILEDOWNLOADER_HXX

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <TFEL/GUI/Config.hxx>

namespace tfel::gui {

  /*!
   * \brief downloads the content of an url with the Qt network access
   * manager
   *
   * \note the download is asynchronous: the content is only available when
   * the `downloaded` signal is emitted
   *
   * \note no code of the editor uses this class at present: the `Castem` mode,
   * which includes this header, opens its web help with
   * `QDesktopServices::openUrl` instead
   */
  struct TFEL_GUI_VISIBILITY_EXPORT FileDownloader : public QObject {
    /*!
     * \brief constructor, which issues the request of the given url
     * \param[in] url : url of the file to download
     * \param[in] p : parent object, as defined by `QObject`
     *
     * \note a downloader handles a single request: a new file requires a
     * new instance
     *
     * \note the data is not available when the constructor returns
     */
    explicit FileDownloader(QUrl, QObject* = nullptr);

    //! destructor, the network access manager is destroyed with it
    virtual ~FileDownloader();

    /*!
     * \return the downloaded content, which is an empty byte array as long
     * as the reply has not been received and for a failed request, for
     * which no error is reported
     */
    QByteArray downloadedData() const;

   signals:
    /*!
     * \brief emitted once the content of the url has been stored
     * \param[in] f : downloader holding the downloaded data, which is the
     * sender of the signal
     */
    void downloaded(FileDownloader*);

   private slots:
    /*!
     * \brief store the content of a completed reply and emit `downloaded`
     * \param[in] pReply : reply to read, whose lifetime ends here as it is
     * scheduled for deletion
     *
     * \note this slot is connected to the `finished` signal of the network
     * access manager, so it is called once the request is over, whatever
     * the outcome is
     */
    void fileDownloaded(QNetworkReply* pReply);

   private:
    //! network access manager in charge of issuing the request
    QNetworkAccessManager m_WebCtrl;

    //! content of the last reply that has been received
    QByteArray m_DownloadedData;

    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_FILEDOWNLOADER_*/
