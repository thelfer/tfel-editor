/*!
 * \file   SecondaryTaskManager.cxx
 * \brief
 * \author Thomas Helfer
 * \date   13/02/2018
 */

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

  void SecondaryTaskManager::removeBuffer(const QEmacsBuffer* const b){
    const auto pb = this->m.find(b);
    if(pb==this->m.end()){
      return;
    }
    // move the task
    const auto tasks = std::move(pb->second);
    this->m.erase(pb);
    for(const auto& t : tasks){
      auto pbv = this->m.begin();
      while(pbv!=this->m.end()){
        const auto pt = wfind(pbv->second,t.w);
        if(pt!=pbv->second.end()){
          pbv->second.erase(pt);
          if(pbv->second.empty()){
            pbv = this->m.erase(pbv);
          } else {
            ++pbv;
          }
        } else {
          ++pbv;
        }
      }
    }
  }  // end of SecondaryTaskManager::removeBuffer

  void SecondaryTaskManager::setCurrentSecondaryTask(
      const QEmacsBuffer* const b, QWidget* const w) {
    const auto pb = this->m.find(b);
    if (pb == this->m.end()) {
      return;
    }
    if (wfind(pb->second, w) == pb->second.end()) {
      return;
    }
    for (auto& t:pb->second) {
      t.current = (t.w == w);
    }
  }  // end of SecondaryTaskManager::setCurrentSecondaryTask

  void SecondaryTaskManager::setTaskTitle(QWidget* const w,
                                          const QString& t) {
    if(w==nullptr){
      return;
    }
    auto found = false;
    for (auto& b:this->m) {
      for (auto& ta : b.second) {
        if(ta.w == w){
          found = true;
          ta.title = t;
        }
      }
    }
    if (found) {
    }
  }  // end of SecondaryTaskManager::setTaskTitle

  void SecondaryTaskManager::setTaskIcon(QWidget* const w,
                                         const QIcon& i) {
    if(w==nullptr){
      return;
    }
    auto found = false;
    for (auto& b:this->m) {
      for (auto& ta : b.second) {
        if(ta.w == w){
          found = true;
          ta.icon = i;
        }
      }
    }
    if (found) {
    }
  }  // end of SecondaryTaskManager::setTaskIcon

  void SecondaryTaskManager::attachSecondaryTask(
      const QEmacsBuffer* const b, const SecondaryTask& t) {
    if ((t.w == nullptr) || (b == nullptr)) {
      return;
    }
    auto& v = this->m[b];
    const auto p = wfind(v,t.w);
    if(p==v.end()){
      v.push_back(t);
    }
    this->setCurrentSecondaryTask(b,t.w);
  }  // end of SecondaryTaskManager::attachSecondaryTask

  void SecondaryTaskManager::attachExistingSecondaryTask(
      const QEmacsBuffer* const b, QWidget *const w) {
    if ((w == nullptr) || (b == nullptr)) {
      return;
    }
    auto& v = this->m[b];
    const auto p = wfind(v,w);
    if (p == v.end()) {
      // the widget is not associated with this buffer
      // so we expect
      // the secondary task to be handled by another buffer
      for (const auto& kv : this->m) {
        const auto p2 = wfind(kv.second,w);
        if (p2 != kv.second.end()) {
          this->attachSecondaryTask(b,*p2);
        }
      }
      // the secondary task is not handled by any buffer, this shall not
      // append
    } else {
      // the widget is already associated with the buffer, so do nothing
    }
  }  // end of SecondaryTaskManager::attachSecondaryTask

  void SecondaryTaskManager::detachSecondaryTask(
      const QEmacsBuffer* const b, QWidget* const w) {
    if ((w == nullptr) || (b == nullptr)) {
      return;
    }
    const auto p = this->m.find(b);
    if (p != this->m.end()) {
      const auto pw = wfind(p->second,w);
      if (pw != p->second.end()) {
        p->second.erase(pw);
        if(p->second.empty()){
          // the buffer don't have any secondary task anymore
          this->m.erase(p);
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
    for (const auto& bv : this->m) {
      const auto pw = wfind(bv.second,w);
      if (pw != bv.second.end()) {
        return;
      }
    }
    // w was associated to b and is no more associated
    // with any other buffer
    w->deleteLater();
  }  // end of SecondaryTaskManager::detachSecondaryTask

  const std::vector<SecondaryTask>&
  SecondaryTaskManager::getSecondaryTasks(const QEmacsBuffer* const b) {
    static const std::vector<SecondaryTask> empty;
    const auto p = this->m.find(b);
    if (p != this->m.end()) {
      return p->second;
    }
    return empty;
  }  // end of getSecondaryTasks

}  // end of namespace qemacs