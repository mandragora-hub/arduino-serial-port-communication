#ifndef SPM_APP_PREFS_DIALOG_H
#define SPM_APP_PREFS_DIALOG_H

#include <gtkmm-4.0/gtkmm.h>

#ifdef GLIBMM_CHECK_VERSION
#define HAS_GIO_SETTINGS_BIND_WITH_MAPPING GLIBMM_CHECK_VERSION(2, 75, 0)
#else
#define HAS_GIO_SETTINGS_BIND_WITH_MAPPING 0
#endif

class SPMAppPrefsDialog : public Gtk::Window {
 public:
  SPMAppPrefsDialog(BaseObjectType *cobject,
                    const Glib::RefPtr<Gtk::Builder> &refBuilder);
  static SPMAppPrefsDialog *create(Gtk::Window &parent);

 protected:
#if HAS_GIO_SETTINGS_BIND_WITH_MAPPING
  // Mapping from Gio::settings to properties
  static std::optional<unsigned int> map_from_ustring_to_int(
      const Glib::ustring &transition);
  static std::optional<Glib::ustring> map_from_int_to_ustring(
      const unsigned int &pos);
#else
  // Signal handlers
  void on_font_setting_changed(const Glib::ustring &key);
  void on_font_selection_changed();
  void on_transition_setting_changed(const Glib::ustring &key);
  void on_transition_selection_changed();
#endif
  Glib::RefPtr<Gtk::Builder> m_refBuilder;
  Glib::RefPtr<Gio::Settings> m_settings;
  Gtk::FontDialogButton *m_font{nullptr};
  Gtk::DropDown *m_transition{nullptr};
};
#endif // SPM_APP_PREFS_DIALOG_H