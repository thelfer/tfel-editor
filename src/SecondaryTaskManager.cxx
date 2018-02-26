/*!
 * \file   SecondaryTaskManager.cxx
 * \brief
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#include "QEmacs/Debug.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/SecondaryTaskManager.hxx"

namespace qemacs {

  SecondaryTaskManager::SecondaryTaskManager() = default;
  SecondaryTaskManager::SecondaryTaskManager(
      const SecondaryTaskManager&) = default;
  SecondaryTaskManager::SecondaryTaskManager(SecondaryTaskManager&&) =
      default;
  SecondaryTaskManager& SecondaryTaskManager::operator=(
      const SecondaryTaskManager&) = default;
  SecondaryTaskManager& SecondaryTaskManager::operator=(
      SecondaryTaskManager&&) = default;
  SecondaryTaskManager::~SecondaryTaskManager() = default;

  static std::vector<SecondaryTask>::iterator wfind(
      std::vector<SecondaryTask>& v, QWidget* const w) {
    return std::find_if(
        v.begin(), v.end(),
        [w](const SecondaryTask& t) { return t.w == w; });
  } // end of wfind

  static std::vector<SecondaryTask>::const_iterator wfind(
      const std::vector<SecondaryTask>& v, QWidget* const w) {
    return std::find_if(
        v.begin(), v.end(),
        [w](const SecondaryTask& t) { return t.w == w; });
  }  // end of wfind

  void SecondaryTaskManager::removeBuffer(QEmacsBuffer* const b){
    if (b == nullptr) {
      return;
    }
    debug("removing buffer: ", b->getBufferName(), b);
    const auto pb = this->bufferTasks.find(b);
    if(pb==this->bufferTasks.end()){
      return;
    }
    // move the tasks associated with the removed buffer
    const auto tasks = std::move(pb->second);
    this->bufferTasks.erase(pb);
    for(const auto& t : tasks){
      // counting the number of times this task exists
      bool found = false;
      for(const auto& bv : this->bufferTasks){
        const auto pt = wfind(bv.second, t.w);
        if (pt != bv.second.end()) {
          found = true;
          break;
        }
      }
      if (!found) {
        debug("deleting widget associated with task:", t.title);
        t.w->deleteLater();
      }
    }
  }  // end of SecondaryTaskManager::removeBuffer

  void SecondaryTaskManager::setCurrentSecondaryTask(
      QEmacsBuffer* const b, QWidget* const w) {
    const auto pb = this->bufferTasks.find(b);
    if (pb == this->bufferTasks.end()) {
      return;
    }
    if (wfind(pb->second, w) == pb->second.end()) {
      return;
    }
    for (auto& t : pb->second) {
      t.current = (t.w == w);
    }
  }  // end of SecondaryTaskManager::setCurrentSecondaryTask

  void SecondaryTaskManager::setSecondaryTaskTitle(QWidget* const w,
                                                   const QString& t) {
    if(w==nullptr){
      return;
    }
    for (auto& b:this->bufferTasks) {
      for (auto& ta : b.second) {
        if(ta.w == w){
          ta.title = t;
          if(b.first->isVisible()){
            b.first->refreshSecondaryTaskTabWidget();
          }
        }
      }
    }
  }  // end of SecondaryTaskManager::setSecondaryTaskTitle

  void SecondaryTaskManager::setSecondaryTaskIcon(QWidget* const w,
                                                  const QIcon& i) {
    if(w==nullptr){
      return;
    }
    for (auto& b:this->bufferTasks) {
      for (auto& ta : b.second) {
        if(ta.w == w){
          ta.icon = i;
          if(b.first->isVisible()){
            b.first->refreshSecondaryTaskTabWidget();
          }
        }
      }
    }
  }  // end of SecondaryTaskManager::setSecondaryTaskIcon

  void SecondaryTaskManager::attachSecondaryTask(
      QEmacsBuffer* const b, const SecondaryTask& t) {
    if ((t.w == nullptr) || (b == nullptr)) {
      return;
    }
    auto& v = this->bufferTasks[b];
    if (wfind(v, t.w) == v.end()) {
      v.push_back(t);
    }
    this->setCurrentSecondaryTask(b,t.w);
  }  // end of SecondaryTaskManager::attachSecondaryTask

  const SecondaryTask& SecondaryTaskManager::attachSecondaryTask(
      QEmacsBuffer* const b, QWidget* const w) {
    static const SecondaryTask nulltask;
    if ((w == nullptr) || (b == nullptr)) {
      return nulltask;
    }
    auto& v = this->bufferTasks[b];
    const auto p = wfind(v,w);
    if (p == v.end()) {
      // the widget is not associated with this buffer
      // so we expect
      // the secondary task to be handled by another buffer
      for (const auto& kv : this->bufferTasks) {
        const auto p2 = wfind(kv.second,w);
        if (p2 != kv.second.end()) {
          this->attachSecondaryTask(b,*p2);
          return *p2;
        }
      }
      // the secondary task is not handled by any buffer, this shall not
      // append
      return nulltask;
    }
    // the widget is already associated with the buffer
    this->setCurrentSecondaryTask(b,w);
    return *p;
  }  // end of SecondaryTaskManager::attachSecondaryTask

  void SecondaryTaskManager::detachSecondaryTask(
      QEmacsBuffer* const b, QWidget* const w) {
    if ((w == nullptr) || (b == nullptr)) {
      return;
    }
    const auto p = this->bufferTasks.find(b);
    auto title = QString{};
    if (p != this->bufferTasks.end()) {
      const auto pw = wfind(p->second,w);
      if (pw != p->second.end()) {
        title = pw->title;
        debug("removing task:", title, "for buffer",
              b->getBufferName());
        p->second.erase(pw);
        if(p->second.empty()){
          // the buffer don't have any secondary task anymore
          this->bufferTasks.erase(p);
        }
      } else {
        // w is not associated with b
        return;
      }
    } else {
      // b has no secondary task
      return;
    }
    // now check if the widget is still associated to a task
    for (const auto& bv : this->bufferTasks) {
      const auto pw = wfind(bv.second,w);
      if (pw != bv.second.end()) {
        return;
      }
    }
    // w was associated to b and is no more associated
    // with any other buffer
    debug("deleting widget associated with task:", title);
    w->deleteLater();
  }  // end of SecondaryTaskManager::detachSecondaryTask

  void SecondaryTaskManager::showSecondaryTask(
      QEmacsBuffer* const b, QWidget* const w) {
    if ((w == nullptr) || (b == nullptr)) {
      return;
    }
    const auto p = this->bufferTasks.find(b);
    if (p != this->bufferTasks.end()) {
      const auto pw = wfind(p->second,w);
      if(pw!=p->second.end()){
        pw->visible = true;
      }
    }
  }  // end of SecondaryTaskManager::showSecondaryTask

  void SecondaryTaskManager::hideSecondaryTask(
      QEmacsBuffer* const b, QWidget* const w) {
    if ((w == nullptr) || (b == nullptr)) {
      return;
    }
    const auto p = this->bufferTasks.find(b);
    if (p != this->bufferTasks.end()) {
      const auto pw = wfind(p->second,w);
      if(pw!=p->second.end()){
        pw->visible = false;
      }
    }
  }  // end of SecondaryTaskManager::hideSecondaryTask

  const std::vector<SecondaryTask>&
  SecondaryTaskManager::getSecondaryTasks(QEmacsBuffer* const b) {
    static const std::vector<SecondaryTask> empty;
    const auto p = this->bufferTasks.find(b);
    if (p != this->bufferTasks.end()) {
      return p->second;
    }
    return empty;
  }  // end of getSecondaryTasks

}  // end of namespace qemacs
