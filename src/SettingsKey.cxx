/*!
 * \file  SettingsKey.cxx
 * \brief description of an entry of the configuration.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#include <QtCore/QSettings>
#include "TFEL/GUI/SettingsKey.hxx"

namespace tfel::gui {

  SettingsKey::Choice::Choice(const QVariant &v, const QString &l)
      : value(v), label(l) {}  // end of SettingsKey::Choice::Choice

  bool SettingsKey::Choice::operator==(const Choice &o) const {
    return (this->value == o.value) && (this->label == o.label);
  }  // end of SettingsKey::Choice::operator==

  SettingsKey::SettingsKey()
      : type(STRING),
        default_value(),
        choices(),
        minimum(0),
        maximum(0) {}  // end of SettingsKey::SettingsKey

  SettingsKey::SettingsKey(const QString &k,
                           const QString &l,
                           const QString &d,
                           const Type t,
                           const QVariant &v)
      : key(k),
        group(),
        label(l),
        description(d),
        type(t),
        default_value(v),
        choices(),
        minimum(0),
        maximum(0) {}  // end of SettingsKey::SettingsKey

  SettingsKey SettingsKey::boolean(const QString &k,
                                   const QString &l,
                                   const QString &d,
                                   const bool v) {
    return SettingsKey{k, l, d, BOOLEAN, QVariant::fromValue(v)};
  }  // end of SettingsKey::boolean

  SettingsKey SettingsKey::integer(const QString &k,
                                   const QString &l,
                                   const QString &d,
                                   const int v,
                                   const int lowest,
                                   const int highest) {
    auto s = SettingsKey{k, l, d, INTEGER, QVariant::fromValue(v)};
    s.minimum = lowest;
    s.maximum = highest;
    return s;
  }  // end of SettingsKey::integer

  SettingsKey SettingsKey::text(const QString &k,
                                const QString &l,
                                const QString &d,
                                const QString &v) {
    return SettingsKey{k, l, d, STRING, v};
  }  // end of SettingsKey::text

  SettingsKey SettingsKey::stringList(const QString &k,
                                      const QString &l,
                                      const QString &d,
                                      const QStringList &v) {
    return SettingsKey{k, l, d, STRING_LIST, v};
  }  // end of SettingsKey::stringList

  SettingsKey SettingsKey::choice(const QString &k,
                                  const QString &l,
                                  const QString &d,
                                  const QVector<Choice> &c,
                                  const QVariant &v) {
    auto s = SettingsKey{k, l, d, CHOICE, v};
    s.choices = c;
    return s;
  }  // end of SettingsKey::choice

  SettingsKey SettingsKey::font(const QString &k,
                                const QString &l,
                                const QString &d) {
    // no default value: the fonts of the editor follow the application font
    // until one is stored
    return SettingsKey{k, l, d, FONT, QVariant()};
  }  // end of SettingsKey::font

  SettingsKey &SettingsKey::inGroup(const QString &g) {
    this->group = g;
    return *this;
  }  // end of SettingsKey::inGroup

  bool SettingsKey::isSet() const {
    return QSettings().contains(this->key);
  }  // end of SettingsKey::isSet

  QVariant SettingsKey::read() const {
    const auto settings = QSettings();
    if (!settings.contains(this->key)) {
      return this->default_value;
    }
    return settings.value(this->key);
  }  // end of SettingsKey::read

  void SettingsKey::write(const QVariant &v) const {
    auto settings = QSettings();
    if (v == this->default_value) {
      // what the user asks for is what would happen without that entry: the
      // address has nothing to say in the configuration file
      settings.remove(this->key);
      return;
    }
    settings.setValue(this->key, v);
  }  // end of SettingsKey::write

  void SettingsKey::reset() const {
    QSettings().remove(this->key);
  }  // end of SettingsKey::reset

}  // end of namespace tfel::gui
