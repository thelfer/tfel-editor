/*!
 * \file  ConfigurationDialog.cxx
 * \brief dialog which lists the entries of the configuration, by theme, and
 *        writes what the user changes.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFontDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/ConfigurationDialog.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"
#include "TFEL/GUI/ShortCutStyle.hxx"
#include "TFEL/GUI/StandardFunctionCommand.hxx"

namespace tfel::gui {

  namespace {
    // the addresses of the entries whose effect the user has to see under their
    // hands. These are the addresses the `General` and `Spell checking` themes
    // declare in src/CoreSettings.cxx: they are spelled twice because the
    // declaration says what an entry is and this file says what applying it
    // takes, which is the one thing the entry can not say about itself
    static const QString font_address = QStringLiteral("font/textedit");
    static const QString short_cut_style_address =
        QStringLiteral("shortcut style");
    // the language the spell checker of the modes underlines the text with,
    // which the buffers of the editor are asked to switch to when it changes
    static const QString dictionary_address =
        QStringLiteral("hunspell default dictionary");
    // the style the editor uses when the address holds nothing, which is what
    // `ShortCutStyle` reads
    static const QString short_cut_style_default = QStringLiteral("EMACS");

    /*!
     * \brief the widget which edits a font: the button which names the font it
     * holds and opens a font dialog when it is pressed
     *
     * \note this is what \c QFontButton does, and \c QFontButton would be the
     * right widget: it appeared in Qt 6.3 and the editor is built against the
     * 6.2 series, so the dialog has to ask for the font itself. The class holds
     * no signal of its own, so it needs no meta object: it is a plain class of
     * this file, not one of the exported classes of the library
     */
    struct FontSelector : public QPushButton {
      //! \brief constructor
      //! \param[in] p : parent widget
      FontSelector(QWidget *const p) : QPushButton(p), chosen() {
        QObject::connect(this, &QPushButton::clicked, this, [this] {
          auto ok = false;
          const auto f = QFontDialog::getFont(&ok, this->chosen, this,
                                              QObject::tr("Select Font"));
          if (ok) {
            this->showFont(f);
          }
        });
      }
      /*!
       * \brief show the given font, in that font
       * \param[in] f : font
       */
      void showFont(const QFont &f) {
        this->chosen = f;
        auto name = f.family();
        if (f.pointSize() > 0) {
          name += QStringLiteral(" %1").arg(f.pointSize());
        }
        this->setText(name);
        this->setFont(f);
      }
      //! \return the font the button holds
      QFont chosenFont() const { return this->chosen; }

     private:
      //! font the button holds
      QFont chosen;
    };  // end of struct FontSelector

    /*!
     * \return the widget which suits the kind of value the given entry holds,
     * without any value in it yet
     * \param[in] k : entry
     * \param[in] p : parent widget
     */
    QWidget *buildEditor(const SettingsKey &k, QWidget *const p) {
      switch (k.type) {
        case SettingsKey::BOOLEAN:
          return new QCheckBox(p);
        case SettingsKey::INTEGER: {
          auto *const s = new QSpinBox(p);
          s->setRange(k.minimum, k.maximum);
          return s;
        }
        case SettingsKey::STRING:
          return new QLineEdit(p);
        case SettingsKey::STRING_LIST: {
          auto *const e = new QPlainTextEdit(p);
          // Tab has to leave the field: it is how one goes from an entry of the
          // dialog to the next one, and the entries of a list have no business
          // holding tabulations
          e->setTabChangesFocus(true);
          e->setFixedHeight(90);
          return e;
        }
        case SettingsKey::CHOICE: {
          auto *const c = new QComboBox(p);
          for (const auto &ch : k.choices) {
            c->addItem(ch.label, ch.value);
          }
          return c;
        }
        case SettingsKey::FONT:
          return new FontSelector(p);
      }
      return new QLabel(QObject::tr("unknown kind of value"), p);
    }  // end of buildEditor()

    /*!
     * \return the identifier of the kind of value the given value holds
     *
     * \note the identifier is what the two spellings of a type agree on: under
     * the 6.2 series the names of the types are values of the \c
     * QMetaType::Type enumeration only, a \c QMetaType built from one appearing
     * in the 6.3 series, so comparing \c QMetaType objects is what does not
     * compile here
     */
    int kindOf(const QVariant &v) {
      return v.metaType().id();
    }  // end of kindOf()

    /*!
     * \return the text a stored value says, empty when it says nothing
     */
    QString toText(const QVariant &v) {
      if (kindOf(v) == QMetaType::QStringList) {
        // what `QSettings` read as a list of a single entry, which is what an
        // address whose value holds no comma is stored as
        const auto l = v.toStringList();
        return l.isEmpty() ? QString() : l.first();
      }
      return v.toString();
    }  // end of toText()

    /*!
     * \return the list of texts a stored value says, empty when it says nothing
     *
     * \note the format of the configuration file of this platform, which is INI
     * under GNU/Linux, is a plain text format: a value which holds a comma is
     * read back as a list, whether the user meant a list or a text with a comma
     * in it, and a list of one entry is read back as a text. The editor answers
     * both readings with \c toStringList, which converts the one into the
     * other, and so does this function
     */
    QStringList toStringListOf(const QVariant &v) {
      if (kindOf(v) == QMetaType::QStringList) {
        return v.toStringList();
      }
      const auto s = v.toString();
      if (s.isEmpty()) {
        return {};
      }
      return s.split(',');
    }  // end of toStringListOf()
  }    // end of anonymous namespace

  QStringList ConfigurationDialog::keys() const {
    auto ks = QStringList{};
    for (const auto &f : this->fields) {
      ks << f.key.key;
    }
    return ks;
  }  // end of ConfigurationDialog::keys

  ConfigurationDialog::Field *ConfigurationDialog::findField(
      const QString &key) {
    for (auto &f : this->fields) {
      if (f.key.key == key) {
        return &f;
      }
    }
    return nullptr;
  }  // end of ConfigurationDialog::findField

  bool ConfigurationDialog::hasSetting(const QString &key) const {
    for (const auto &f : this->fields) {
      if (f.key.key == key) {
        return true;
      }
    }
    return false;
  }  // end of ConfigurationDialog::hasSetting

  QVariant ConfigurationDialog::valueOf(const Field &f) const {
    switch (f.key.type) {
      case SettingsKey::BOOLEAN:
        if (auto *const c = qobject_cast<QCheckBox *>(f.widget)) {
          return QVariant::fromValue(c->isChecked());
        }
        break;
      case SettingsKey::INTEGER:
        if (auto *const s = qobject_cast<QSpinBox *>(f.widget)) {
          return QVariant::fromValue(s->value());
        }
        break;
      case SettingsKey::STRING:
        if (auto *const l = qobject_cast<QLineEdit *>(f.widget)) {
          return l->text();
        }
        break;
      case SettingsKey::STRING_LIST: {
        if (auto *const e = qobject_cast<QPlainTextEdit *>(f.widget)) {
          auto l = e->toPlainText().split('\n');
          // a list which ends with a line break is the same list which does
          // not: the trailing empty line is how the edit shows the last line it
          // holds
          while (!l.isEmpty() && l.back().isEmpty()) {
            l.pop_back();
          }
          return l;
        }
        break;
      }
      case SettingsKey::CHOICE:
        if (auto *const c = qobject_cast<QComboBox *>(f.widget)) {
          return c->currentData();
        }
        break;
      case SettingsKey::FONT:
        if (auto *const b = dynamic_cast<FontSelector *>(f.widget)) {
          return QVariant::fromValue(b->chosenFont());
        }
        break;
    }
    return {};
  }  // end of ConfigurationDialog::valueOf

  bool ConfigurationDialog::setValueOf(Field &f, const QVariant &v) {
    switch (f.key.type) {
      case SettingsKey::BOOLEAN: {
        auto *const c = qobject_cast<QCheckBox *>(f.widget);
        if (c == nullptr) {
          return false;
        }
        if (kindOf(v) == QMetaType::Bool) {
          c->setChecked(v.toBool());
          return true;
        }
        if (kindOf(v) == QMetaType::QString) {
          // the configuration file is a text file, where a boolean is a word
          const auto s = v.toString().compare(QStringLiteral("true"),
                                              Qt::CaseInsensitive) == 0;
          if (s || (v.toString().compare(QStringLiteral("false"),
                                         Qt::CaseInsensitive) == 0)) {
            c->setChecked(s);
            return true;
          }
        }
        return false;
      }
      case SettingsKey::INTEGER: {
        auto *const s = qobject_cast<QSpinBox *>(f.widget);
        if ((s == nullptr) || (kindOf(v) != QMetaType::Int)) {
          return false;
        }
        s->setValue(v.toInt());
        return true;
      }
      case SettingsKey::STRING: {
        auto *const l = qobject_cast<QLineEdit *>(f.widget);
        if ((l == nullptr) || !v.isValid()) {
          return false;
        }
        l->setText(toText(v));
        return true;
      }
      case SettingsKey::STRING_LIST: {
        auto *const e = qobject_cast<QPlainTextEdit *>(f.widget);
        if ((e == nullptr) || !v.isValid()) {
          return false;
        }
        e->setPlainText(toStringListOf(v).join('\n'));
        return true;
      }
      case SettingsKey::CHOICE: {
        auto *const c = qobject_cast<QComboBox *>(f.widget);
        if ((c == nullptr) || !v.isValid()) {
          return false;
        }
        auto i = c->findData(v);
        if (i < 0) {
          // the value of a choice is an integer or a word, and the
          // configuration file of this platform holds no type which one does
          // not become in the other
          i = c->findData(toText(v));
        }
        if (i < 0) {
          return false;
        }
        c->setCurrentIndex(i);
        return true;
      }
      case SettingsKey::FONT: {
        auto *const b = dynamic_cast<FontSelector *>(f.widget);
        if ((b == nullptr) || (kindOf(v) != QMetaType::QFont)) {
          return false;
        }
        b->showFont(v.value<QFont>());
        return true;
      }
    }
    return false;
  }  // end of ConfigurationDialog::setValueOf

  ConfigurationDialog::ConfigurationDialog(EditorWidget &w, QWidget *const p)
      : QDialog(p), editor(w) {
    this->setWindowTitle(QObject::tr("Configuration"));
    auto *const vl = new QVBoxLayout();
    auto *const hl = new QHBoxLayout();
    auto *const themes = new QListWidget(this);
    themes->setObjectName(QStringLiteral("configuration-themes"));
    themes->setMinimumWidth(180);
    auto *const pages = new QStackedWidget(this);
    pages->setObjectName(QStringLiteral("configuration-pages"));
    // The themes are of two kinds which have nothing in common: those of the
    // editor, always there, and those the major modes export, which come and go
    // with the plugins loaded and are about a program the mode drives, not
    // about the editor. Mixing them in one list makes `clangd` a peer of
    // `General`, which it is not, so the list keeps them in two blocks with a
    // title over each of them.
    const auto groups = SettingsRegistry::getSettingsRegistry().getGroups();
    auto mode_groups = QVector<SettingsGroup>{};
    auto editor_groups = QVector<SettingsGroup>{};
    for (const auto &g : groups) {
      if (g.mode.isEmpty()) {
        editor_groups.append(g);
      } else {
        mode_groups.append(g);
      }
    }
    // A title is not a theme: it names no page, holds nothing to edit, and is
    // given the row of nothing (-1) so that choosing it changes nothing.
    auto add_heading = [themes](const QString &text) {
      auto *const item = new QListWidgetItem(text, themes);
      item->setFlags(Qt::NoItemFlags);
      auto f = item->font();
      f.setBold(true);
      item->setFont(f);
      item->setForeground(
          themes->palette().color(QPalette::Disabled, QPalette::Text));
      item->setData(Qt::UserRole, -1);
    };  // end of add_heading
    auto add_group = [this, themes, pages](const SettingsGroup &g) {
      auto *const page = new QWidget(pages);
      auto *const pl = new QVBoxLayout(page);
      if (!g.mode.isEmpty()) {
        // the page says whose it is: the theme of a mode is named after the
        // program it drives, so without this line the page of `clangd` reads as
        // a page about the editor
        auto *const o =
            new QLabel(QObject::tr("%1 major mode").arg(g.mode), page);
        o->setObjectName(QStringLiteral("configuration-page-mode"));
        auto f = o->font();
        f.setBold(true);
        o->setFont(f);
        // a caption, not a heading of the page: `QLabel` has no
        // `setForeground`, the grey of a disabled piece of text is a palette of
        // the label itself
        auto pal = o->palette();
        pal.setColor(QPalette::WindowText,
                     page->palette().color(QPalette::Disabled, QPalette::Text));
        o->setPalette(pal);
        pl->addWidget(o);
      }
      if (!g.description.isEmpty()) {
        auto *const d = new QLabel(g.description, page);
        d->setWordWrap(true);
        pl->addWidget(d);
      }
      auto *const form = new QFormLayout();
      // the name of an entry above the field which edits it: two columns would
      // give the label column the width of the longest name of the theme and
      // leave the field with what is left, which is how the path of a program
      // and a list of arguments end up unreadable in a 187 pixels wide box on
      // the `clangd` page. The sentence of the entry is the tool tip of the
      // label and of the field, so nothing is lost by giving the field the page
      form->setRowWrapPolicy(QFormLayout::WrapAllRows);
      form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
      form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
      for (const auto &k : g.keys) {
        auto *const e = buildEditor(k, page);
        e->setToolTip(k.description);
        auto *const l = new QLabel(k.label, page);
        l->setToolTip(k.description);
        form->addRow(l, e);
        this->fields.push_back({k, e});
      }
      pl->addLayout(form);
      pl->addStretch(1);
      // a theme holds a list of files a user cannot count and a page has to fit
      // the height a person gave this dialog, so the page scrolls
      auto *const scroll = new QScrollArea(pages);
      scroll->setWidgetResizable(true);
      scroll->setFrameShape(QFrame::NoFrame);
      scroll->setWidget(page);
      auto *const item = new QListWidgetItem(g.name, themes);
      if (!g.icon.isNull()) {
        item->setIcon(g.icon);
      }
      item->setToolTip(g.description);
      // the row of the list is not the page of the stack: the titles in between
      // hold no page, so what a row means is kept in the item itself
      item->setData(Qt::UserRole, pages->count());
      pages->addWidget(scroll);
    };  // end of add_group
    add_heading(QObject::tr("Editor"));
    for (const auto &g : editor_groups) {
      add_group(g);
    }
    if (!mode_groups.isEmpty()) {
      auto *const gap = new QListWidgetItem(themes);
      gap->setFlags(Qt::NoItemFlags);
      gap->setData(Qt::UserRole, -1);
      add_heading(QObject::tr("Major modes"));
      for (const auto &g : mode_groups) {
        add_group(g);
      }
    }
    // choosing a title changes nothing: only a row which names a page shows one
    QObject::connect(themes, &QListWidget::currentRowChanged, this,
                     [themes, pages](const int row) {
                       auto *const item =
                           (row < 0) ? nullptr : themes->item(row);
                       if (item == nullptr) {
                         return;
                       }
                       const auto page = item->data(Qt::UserRole).toInt();
                       if (page >= 0) {
                         pages->setCurrentIndex(page);
                       }
                     });
    // the first theme, which is the row after the title of the first block
    for (auto row = 0; row != themes->count(); ++row) {
      if (themes->item(row)->data(Qt::UserRole).toInt() >= 0) {
        themes->setCurrentRow(row);
        break;
      }
    }
    hl->addWidget(themes);
    hl->addWidget(pages, 1);
    vl->addLayout(hl);
    // the buttons
    auto *const bb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                                 QDialogButtonBox::RestoreDefaults,
                             this);
    QObject::connect(bb, &QDialogButtonBox::accepted, this,
                     &ConfigurationDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, this,
                     &ConfigurationDialog::reject);
    // `QDialogButtonBox::reset` is gone from Qt 6: the button it used to emit
    // is reached through the button itself, which is a `QPushButton` of the box
    if (auto *const r = bb->button(QDialogButtonBox::RestoreDefaults)) {
      QObject::connect(r, &QPushButton::clicked, this,
                       &ConfigurationDialog::restoreDefaults);
    }
    vl->addWidget(bb);
    this->setLayout(vl);
    this->resize(760, 520);
    // what is stored, or the default value of what was never written
    for (auto &f : this->fields) {
      auto v = f.key.read();
      if ((f.key.type == SettingsKey::FONT) && (!v.isValid())) {
        // the fonts of the editor follow the application font until one is
        // stored, which is what the code which reads that address does
        v = QVariant::fromValue(QApplication::font());
      }
      this->setValueOf(f, v);
    }
  }  // end of ConfigurationDialog::ConfigurationDialog

  QVariant ConfigurationDialog::value(const QString &key) const {
    for (const auto &f : this->fields) {
      if (f.key.key == key) {
        return this->valueOf(f);
      }
    }
    return {};
  }  // end of ConfigurationDialog::value

  void ConfigurationDialog::setValue(const QString &key, const QVariant &v) {
    auto *const f = this->findField(key);
    if (f == nullptr) {
      return;
    }
    this->setValueOf(*f, v);
  }  // end of ConfigurationDialog::setValue

  QStringList ConfigurationDialog::apply() {
    auto written = QStringList{};
    const auto settings = QSettings();
    for (auto &f : this->fields) {
      const auto v = this->valueOf(f);
      if (!v.isValid()) {
        continue;
      }
      const auto stored = settings.contains(f.key.key);
      if (v == f.key.default_value) {
        // what the user asks for is what would happen without that entry: the
        // address has nothing to say in the configuration file
        if (stored) {
          f.key.reset();
          written << f.key.key;
        }
        continue;
      }
      if (stored) {
        const auto was = settings.value(f.key.key);
        const auto same = [this, &was, &v](const Field &field) {
          switch (field.key.type) {
            case SettingsKey::STRING:
              return toText(was) == v.toString();
            case SettingsKey::STRING_LIST:
              return toStringListOf(was) == v.toStringList();
            default:
              return was == v;
          }
        };
        if (same(f)) {
          continue;
        }
      }
      f.key.write(v);
      written << f.key.key;
    }
    return written;
  }  // end of ConfigurationDialog::apply

  void ConfigurationDialog::restoreDefaults() {
    for (auto &f : this->fields) {
      auto v = f.key.default_value;
      if ((f.key.type == SettingsKey::FONT) && (!v.isValid())) {
        v = QVariant::fromValue(QApplication::font());
      }
      this->setValueOf(f, v);
    }
  }  // end of ConfigurationDialog::restoreDefaults

  void ConfigurationDialog::applyLiveEntries(const QStringList &written) {
    if (written.isEmpty()) {
      this->editor.displayInformativeMessage(
          QObject::tr("configuration unchanged"));
      return;
    }
    if (written.contains(font_address)) {
      // the font of the main frames is read when the widgets which show it are
      // built, which is why changing it here waits for this line, and why the
      // dialog, not the entry, knows what applying it takes
      auto f = QVariant();
      {
        const auto settings = QSettings();
        if (settings.contains(font_address)) {
          f = settings.value(font_address);
        }
      }
      auto font = f.isValid() ? f.value<QFont>() : QApplication::font();
      font.setStyleStrategy(QFont::PreferAntialias);
      this->editor.changeFont(font);
    }
    if (written.contains(short_cut_style_address)) {
      // the style is held by a singleton which reads the address once, and the
      // menu bar is rebuilt when that singleton says it changed
      auto style = short_cut_style_default;
      {
        const auto settings = QSettings();
        if (settings.contains(short_cut_style_address)) {
          style = settings.value(short_cut_style_address).toString();
        }
      }
      auto &s = ShortCutStyle::getShortCutStyle();
      if (style == QStringLiteral("QT")) {
        s.setStyle(ShortCutStyle::QT);
      } else {
        s.setStyle(ShortCutStyle::EMACS);
      }
    }
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
    if (written.contains(dictionary_address)) {
      // the same as what the entries of the `Dictionaries` menu do: the
      // language has to reach every buffer, not the only one on screen
      const auto settings = QSettings();
      const auto dictionary = settings.value(dictionary_address).toString();
      for (auto *b : this->editor.findChildren<Buffer *>()) {
        b->getMainFrame().setSpellCheckLanguage(dictionary);
      }
    }
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
    this->editor.displayInformativeMessage(
        QObject::tr("%1 entries of the configuration written. Some of them are "
                    "read when the widgets which use them are built: they wait "
                    "for a new buffer or for a restart of the editor.")
            .arg(written.count()));
  }  // end of ConfigurationDialog::applyLiveEntries

  void ConfigurationDialog::accept() {
    const auto written = this->apply();
    this->applyLiveEntries(written);
    QDialog::accept();
  }  // end of ConfigurationDialog::accept

  void configureTfelEditor(EditorWidget &w) {
    auto dialog = ConfigurationDialog(w, &w);
    dialog.exec();
  }  // end of configureTfelEditor

  static StandardFunctionCommandProxy<configureTfelEditor>
      configureTfelEditorProxy("configure-tfel-editor");

}  // end of namespace tfel::gui
