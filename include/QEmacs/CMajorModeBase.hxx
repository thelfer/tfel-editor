/*! 
 * \file  CMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 05 sept. 2012
 */

#ifndef LIB_QEMACS_CMAJORMODEBASE_HXX
#define LIB_QEMACS_CMAJORMODEBASE_HXX 

#include"QEmacs/Config.hxx"
#include"QEmacs/CompiledLanguageMajorModeBase.hxx"

namespace qemacs
{

  /*!
   * base class for C-based language.
   * This mainly deals with indentation.
   */
  class QEMACS_VISIBILITY_EXPORT CMajorModeBase
    : public CompiledLanguageMajorModeBase
  {

    Q_OBJECT

  public:

    CMajorModeBase(QEmacsWidget&,
		   QEmacsBuffer&,
		   QEmacsTextEditBase&);

    /*!
     * format a paragraph
     */
    void format() override;
    /*!
     * comment/uncomment a region
     */
    void comment() override;

    void indentLine(const QTextCursor&) override;
    
    void indentRegion(const QTextCursor&) override;

    ~CMajorModeBase() override;
    
  protected:

    /*!
     * \brief indent a region
     * \param[in] b: beginning of the region
     * \param[in] e: end of the region
     * \param[in] c: current position
     */
    virtual void
    indent(const QTextCursor&,
	   const QTextCursor&,
	   const QTextCursor&);

  }; // end of struct CMajorModeBase

} // end of namespace qemacs

#endif /* LIB_QEMACS_CMAJORMODEBASE_HXX */

