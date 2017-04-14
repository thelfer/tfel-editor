/*! 
 * \file  CMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 05 sept. 2012
 */

#ifndef _LIB_QEMACS_CMAJORMODEBASE_H_
#define _LIB_QEMACS_CMAJORMODEBASE_H_ 

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
    virtual void
    format(void) override;
    /*!
     * comment/uncomment a region
     */
    virtual void
    comment(void) override;

    virtual void
    indentLine(const QTextCursor&) override;
    
    virtual void
    indentRegion(const QTextCursor&) override;

    virtual ~CMajorModeBase();
    
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

#endif /* _LIB_QEMACS_CMAJORMODEBASE_H */

