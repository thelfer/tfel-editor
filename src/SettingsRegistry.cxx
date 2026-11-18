/*!
 * \file  SettingsRegistry.cxx
 * \brief gathering of the entries of the configuration, by theme.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#include <algorithm>
#include <QtCore/QDebug>
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"

namespace tfel::gui {

  StandardSettingsGroupProxy::StandardSettingsGroupProxy(
      const QString &group,
      const QString &what,
      const QVector<SettingsKey> &entries,
      const QString &sub_theme_of,
      const QString &section_of,
      const bool b)
      : name(group),
        description(what),
        keys(entries),
        parent(sub_theme_of) {
    if (b) {
      auto g = SettingsGroup{};
      g.name = group;
      g.description = description;
      g.keys = entries;
      g.parent = sub_theme_of;
      // The section is the one thing a theme says about the list it is shown in
      // which its name does not: a theme named after a program the editor runs
      // beside itself belongs to no block of its own kind
      g.section = section_of;
      SettingsRegistry::getSettingsRegistry().addSettingsGroup(g);
    }
  }  // end of StandardSettingsGroupProxy::StandardSettingsGroupProxy

  QString StandardSettingsGroupProxy::getName() const {
    return this->name;
  }  // end of StandardSettingsGroupProxy::getName

  QString StandardSettingsGroupProxy::getDescription() const {
    return this->description;
  }  // end of StandardSettingsGroupProxy::getDescription

  QVector<SettingsKey> StandardSettingsGroupProxy::getSettingsKeys() const {
    return this->keys;
  }  // end of StandardSettingsGroupProxy::getSettingsKeys

  StandardSettingsGroupProxy::~StandardSettingsGroupProxy() = default;

  SettingsRegistry &SettingsRegistry::getSettingsRegistry() {
    static SettingsRegistry r;
    return r;
  }  // end of SettingsRegistry::getSettingsRegistry

  SettingsRegistry::SettingsRegistry() = default;

  SettingsRegistry::~SettingsRegistry() = default;

  namespace {
    /*!
     * \return true if two declarations of the same address show it the same way
     *
     * \note a mode which derives from another mode inherits the declarations of
     * the latter: the mode `MTest` asks the very same entries as the mode
     * `C++`, and reporting that as a clash would fill the console of every
     * session with a complaint about a situation which is exactly what it
     * should be
     */
    bool agree(const SettingsKey &a, const SettingsKey &b) {
      // the theme an entry belongs to is not part of the comparison: which of
      // two modes shows a shared address is decided by which was loaded first,
      // and that is worth neither a warning nor a different dialog
      return (a.label == b.label) && (a.description == b.description) &&
             (a.type == b.type) && (a.default_value == b.default_value) &&
             (a.minimum == b.minimum) && (a.maximum == b.maximum) &&
             (a.choices == b.choices);
    }  // end of agree()
  }    // end of anonymous namespace

  const SettingsKey *SettingsRegistry::findKey(
      const QString &key, const QVector<SettingsGroup> &gs) {
    for (const auto &h : gs) {
      for (const auto &k : h.keys) {
        if (k.key == key) {
          return &k;
        }
      }
    }
    return nullptr;
  }  // end of SettingsRegistry::findKey

  void SettingsRegistry::appendGroup(const SettingsGroup &g,
                                     QVector<SettingsGroup> &gs) {
    // the theme the entries which name none belong to
    auto filled = g;
    for (auto &k : filled.keys) {
      if (k.group.isEmpty()) {
        k.group = filled.name;
      }
    }
    // an address is shown once, by the first declaration it got. What is left
    // out is reported only when the two declarations disagree, since that is
    // the case a person has to be told about: two of them promise different
    // things about one address, and which of them wins is the order the plugins
    // were loaded in
    auto kept = QVector<SettingsKey>{};
    for (const auto &k : filled.keys) {
      const auto *const already = findKey(k.key, gs);
      if (already != nullptr) {
        if (!agree(*already, k)) {
          qDebug() << "SettingsRegistry: the entry" << k.key
                   << "is declared twice with different promises, the first "
                      "declaration is kept";
        }
        continue;
      }
      kept.append(k);
    }
    filled.keys = kept;
    for (auto &h : gs) {
      if (h.name.compare(filled.name, Qt::CaseInsensitive) != 0) {
        continue;
      }
      // the theme is already there: what it holds is completed, its name,
      // its description and its icon are those of the first declaration
      h.keys.append(filled.keys);
      if (h.description.isEmpty() && !filled.description.isEmpty()) {
        h.description = filled.description;
      }
      if (h.icon.isNull()) {
        h.icon = filled.icon;
      }
      // the mode which exported the theme is the one it belongs to, and a theme
      // of the editor has none: two modes declaring the same theme keep the
      // first attribution, as they keep its name
      if (h.mode.isEmpty()) {
        h.mode = filled.mode;
      }
      // and so does the theme it is a submenu of: what a theme is a corner of
      // is what its first declaration said
      if (h.parent.isEmpty()) {
        h.parent = filled.parent;
      }
      // and the block of the list it asks for, which is the same promise: two
      // declarations of one theme, one of them naming a block and the other
      // naming none, agree on the block the first one named
      if (h.section.isEmpty()) {
        h.section = filled.section;
      }
      return;
    }
    // A theme which holds nothing says nothing: it is not worth a page of the
    // configuration dialog, and a mode which exports no entry must not appear
    // in it at all
    if (filled.keys.isEmpty()) {
      return;
    }
    gs.append(filled);
  }  // end of SettingsRegistry::appendGroup

  void SettingsRegistry::addSettingsGroup(const SettingsGroup &g) {
    appendGroup(g, this->groups);
  }  // end of SettingsRegistry::addSettingsGroup

  namespace {
    /*!
     * \brief write a theme and, after it, every theme which names it as its
     * parent
     * \param[in] gs : all the themes
     * \param[in] k : index of the theme to write
     * \param[in,out] ordered : themes written so far
     * \param[in,out] done : which themes were written
     */
    void writeTheme(const QVector<SettingsGroup> &gs,
                    const int k,
                    QVector<SettingsGroup> &ordered,
                    QVector<bool> &done) {
      ordered.append(gs[k]);
      done[k] = true;
      for (auto m = 0; m != gs.size(); ++m) {
        if ((not done[m]) &&
            (gs[m].parent.compare(gs[k].name, Qt::CaseInsensitive) == 0)) {
          writeTheme(gs, m, ordered, done);
        }
      }
    }  // end of writeTheme

    /*!
     * \return the given themes, each sub-theme directly after the theme it is a
     * submenu of
     * \param[in] gs : themes to order
     *
     * \note the order the declarations were gathered in is no order a person can
     * guess, and it says nothing about which theme is a corner of which other:
     * the entries of a proofreader belong under the choice of that proofreader,
     * whatever file declared them and whatever static initialiser ran first
     *
     * \note a theme whose parent is not declared, a theme nobody declared a
     * parent for, and the themes caught in a cycle of promises are all kept, in
     * the order they came in: an order is nothing to lose a theme over
     */
    QVector<SettingsGroup> orderUnderParents(const QVector<SettingsGroup> &gs) {
      auto ordered = QVector<SettingsGroup>{};
      auto done = QVector<bool>(gs.size(), false);
      // \c true when a theme of the list bears the given name
      const auto declared = [&gs](const QString &name) {
        for (const auto &g : gs) {
          if (g.name.compare(name, Qt::CaseInsensitive) == 0) {
            return true;
          }
        }
        return false;
      };  // end of declared
      for (auto k = 0; k != gs.size(); ++k) {
        if (done[k]) {
          continue;
        }
        // a sub-theme is written with its parent; what names no parent, or a
        // theme which is not there to hold it, stands on its own
        if (gs[k].parent.isEmpty() || (!declared(gs[k].parent))) {
          writeTheme(gs, k, ordered, done);
        }
      }
      // what is left is a chain of promises which leads nowhere but onto itself:
      // it keeps its order rather than being lost
      for (auto k = 0; k != gs.size(); ++k) {
        if (!done[k]) {
          ordered.append(gs[k]);
          done[k] = true;
        }
      }
      return ordered;
    }  // end of orderUnderParents

    /*!
     * \brief order themes by name, case insensitively
     * \param[in,out] gs : themes to order
     */
    void orderByName(QVector<SettingsGroup> &gs) {
      std::sort(gs.begin(), gs.end(), [](const SettingsGroup &a,
                                         const SettingsGroup &b) {
        return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
      });
    }  // end of orderByName
  }    // end of anonymous namespace

  QVector<SettingsGroup> SettingsRegistry::getGroups() const {
    auto gs = this->groups;
    // the order the static initialisers of the libraries ran in is unspecified,
    // so the themes of the editor would be listed in an order which changes
    // with how the library was linked: what a person looks for in a list of
    // themes is a name, and the list is therefore sorted by name
    orderByName(gs);
    auto &f = MajorModeFactory::getMajorModeFactory();
    // the modes, oldest declaration first, so that the first declaration of a
    // theme shared by two of them is the one which is shown
    const auto names = f.getAvailableMajorModesNames();
    QVector<SettingsGroup> modes;
    for (auto it = names.rbegin(); it != names.rend(); ++it) {
      for (const auto &g : f.getMajorModeSettingsGroups(*it)) {
        appendGroup(g, modes);
      }
    }
    // the themes of the modes come last and in a fixed order: the order the
    // plugins happen to be scanned in is no order a user can guess
    orderByName(modes);
    for (const auto &g : modes) {
      appendGroup(g, gs);
    }
    return orderUnderParents(gs);
  }  // end of SettingsRegistry::getGroups

  QStringList SettingsRegistry::getGroupsNames() const {
    auto n = QStringList{};
    for (const auto &g : this->getGroups()) {
      n << g.name;
    }
    return n;
  }  // end of SettingsRegistry::getGroupsNames

  QVector<SettingsKey> SettingsRegistry::getSettingsKeysOfGroup(
      const QString &group) const {
    for (const auto &g : this->getGroups()) {
      if (g.name.compare(group, Qt::CaseInsensitive) == 0) {
        return g.keys;
      }
    }
    return {};
  }  // end of SettingsRegistry::getSettingsKeysOfGroup

  QVector<SettingsKey> SettingsRegistry::getSettingsKeys() const {
    auto ks = QVector<SettingsKey>{};
    for (const auto &g : this->getGroups()) {
      ks.append(g.keys);
    }
    return ks;
  }  // end of SettingsRegistry::getSettingsKeys

  bool SettingsRegistry::hasSettingsKey(const QString &key) const {
    return this->getSettingsKey(key).has_value();
  }  // end of SettingsRegistry::hasSettingsKey

  std::optional<SettingsKey> SettingsRegistry::getSettingsKey(
      const QString &key) const {
    for (const auto &g : this->getGroups()) {
      for (const auto &k : g.keys) {
        if (k.key == key) {
          return k;
        }
      }
    }
    return std::nullopt;
  }  // end of SettingsRegistry::getSettingsKey

}  // end of namespace tfel::gui
