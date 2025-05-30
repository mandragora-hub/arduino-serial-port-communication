#include "spm_app_window.h"

#include <iostream>
#include <set>
#include <stdexcept>

#include "resources.c"
#include "spm_app.h"
#include "spm_open_serial_port_dialog.h"

SPMAppWindow::SPMAppWindow(BaseObjectType *cobject,
                           const Glib::RefPtr<Gtk::Builder> &refBuilder)
    : Gtk::ApplicationWindow(cobject), m_refBuilder(refBuilder) {
  // Get widgets from Gtk::Builder file
  m_stack = m_refBuilder->get_widget<Gtk::Stack>("stack");
  if (!m_stack) throw std::runtime_error("No \"stack\" object in window.ui");

  m_search = m_refBuilder->get_widget<Gtk::ToggleButton>("search");
  if (!m_search) throw std::runtime_error("No \"search\" object in window.ui");

  m_searchbar = m_refBuilder->get_widget<Gtk::SearchBar>("searchbar");
  if (!m_searchbar)
    throw std::runtime_error("No \"searchbar\" object in window.ui");

#if HAS_SEARCH_ENTRY2
  m_searchentry = m_refBuilder->get_widget<Gtk::SearchEntry2>("searchentry");
#else
  m_searchentry = m_refBuilder->get_widget<Gtk::SearchEntry>("searchentry");
#endif
  if (!m_searchentry)
    throw std::runtime_error("No \"searchentry\" object in window.ui");

  m_gears = m_refBuilder->get_widget<Gtk::MenuButton>("gears");
  if (!m_gears) throw std::runtime_error("No \"gears\" object in window.ui");

  m_sidebar = m_refBuilder->get_widget<Gtk::Revealer>("sidebar");
  if (!m_sidebar)
    throw std::runtime_error("No \"sidebar\" object in window.ui");

  m_words = m_refBuilder->get_widget<Gtk::ListBox>("words");
  if (!m_words) throw std::runtime_error("No \"words\" object in window.ui");

  m_lines = m_refBuilder->get_widget<Gtk::Label>("lines");
  if (!m_lines) throw std::runtime_error("No \"lines\" object in window.ui");

  m_lines_label = m_refBuilder->get_widget<Gtk::Label>("lines_label");
  if (!m_lines_label)
    throw std::runtime_error("No \"lines_label\" object in window.ui");

  // Bind settings
  m_settings = Gio::Settings::create("org.gtkmm.spmonitor");
  m_settings->bind("transition", m_stack->property_transition_type());
  m_settings->bind("show-words", m_sidebar->property_reveal_child());

  // Bind properties
  m_binding_search_enabled = Glib::Binding::bind_property(
      m_search->property_active(), m_searchbar->property_search_mode_enabled(),
      Glib::Binding::Flags::BIDIRECTIONAL);

  // Connect signal handler
  m_searchentry->signal_search_changed().connect(
      sigc::mem_fun(*this, &SPMAppWindow::on_search_text_changed));
  m_stack->property_visible_child().signal_changed().connect(
      sigc::mem_fun(*this, &SPMAppWindow::on_visible_child_changed));
  m_sidebar->property_reveal_child().signal_changed().connect(
      sigc::mem_fun(*this, &SPMAppWindow::on_reveal_child_changed));

  // Connect the menu to MenuButton.
  auto menu_builder = Gtk::Builder::create_from_resource(
      "/org/gtkmm/spmonitor/resources/gears_menu.ui");
  auto menu = menu_builder->get_object<Gio::MenuModel>("menu");
  if (!menu) throw std::runtime_error("No \"menu\" object in gears_menu.ui");

  m_gears->set_menu_model(menu);
  add_action("open-serial-port",
             sigc::mem_fun(*this, &SPMAppWindow::on_action_open_serial_port));
  add_action(m_settings->create_action("show-words"));

  // Bind the "visible" property of m_lines to the win.show-lines action, to
  // the "Lines" menu item and to the "visible" property of m_lines_label.
  add_action(
      Gio::PropertyAction::create("show-lines", m_lines->property_visible()));
  m_binding_lines_visible = Glib::Binding::bind_property(
      m_lines->property_visible(), m_lines_label->property_visible());

  // Set window icon
  Gtk::IconTheme::get_for_display(get_display())
      ->add_resource_path("/org/gtkmm/spmonitor/resources");
  set_icon_name("logo");

  // Remove this line and create a action.
  // createView();
}

// static
SPMAppWindow *SPMAppWindow::create() {
  auto refBuilder = Gtk::Builder::create_from_resource(
      "/org/gtkmm/spmonitor/resources/window.ui");

  auto window =
      Gtk::Builder::get_widget_derived<SPMAppWindow>(refBuilder, "app_window");
  if (!window)
    throw std::runtime_error("No \"app_window\" object in window.ui");

  return window;
}

void SPMAppWindow::createView() {
  // const Glib::ustring basename = file->get_basename();
  const Glib::ustring basename = "file->get_basename()";

  auto refBuilder = Gtk::Builder::create_from_resource(
      "/org/gtkmm/spmonitor/resources/view.ui");
  auto view_box = refBuilder->get_widget<Gtk::Box>("view-box");
  if (!view_box)
    throw std::runtime_error("No \"lines_label\" object in window.ui");

  auto view = refBuilder->get_widget<Gtk::TextView>("text-view");
  if (!view) throw std::runtime_error("No \"lines_label\" object in window.ui");

  m_stack->add(*view_box, basename, basename);

  // auto buffer = view->get_buffer();
  // try {
  //   char *contents = nullptr;
  //   gsize length = 0;

  //   file->load_contents(contents, length);
  //   buffer->set_text(contents, contents + length);
  //   g_free(contents);
  // } catch (const Glib::Error &ex) {
  //   std::cout << "ExampleAppWindow::open_file_view(\"" <<
  //   file->get_parse_name()
  //             << "\"):\n  " << ex.what() << std::endl;
  // }

  // auto tag = buffer->create_tag();
  // m_settings->bind("font", tag->property_font());
  // buffer->apply_tag(tag, buffer->begin(), buffer->end());

  // m_search->set_sensitive(true);

  // update_words();
  // update_lines();
}

void SPMAppWindow::open_file_view(const Glib::RefPtr<Gio::File> &file) {
  const Glib::ustring basename = file->get_basename();

  auto refBuilder = Gtk::Builder::create_from_resource(
      "/org/gtkmm/spmonitor/resources/view.ui");
  auto view_box = refBuilder->get_widget<Gtk::Box>("view-box");
  if (!view_box)
    throw std::runtime_error("No \"lines_label\" object in view.ui");

  auto view = refBuilder->get_widget<Gtk::TextView>("text-view");
  if (!view) throw std::runtime_error("No \"lines_label\" object in view.ui");

  auto clearOutputButton =
      refBuilder->get_widget<Gtk::Button>("clear-output-button");
  if (!clearOutputButton)
    throw std::runtime_error("No \"clear-output-button\" object in view.ui");

  auto input_entry = refBuilder->get_widget<Gtk::Entry>("input-entry");
  if (!input_entry)
    throw std::runtime_error("No \"input-entry\" object in view.ui");

  m_stack->add(*view_box, basename, basename);

  // SerialPort *sp = new SerialPort(file->get_path(), SP_MODE_READ_WRITE);
  auto worker = std::make_shared<SPWorker>();
  worker->m_dispatcher.connect(sigc::bind(
      sigc::mem_fun(*this, &SPMAppWindow::on_text_view_update), worker, view));
  worker->thread = new std::thread([this, worker] { worker->do_work(this); });

  // Connect gui signal with slots
  input_entry->signal_insert_text();
  clearOutputButton->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &SPMAppWindow::on_clear_output), view));

  // m_WorkerTable.insert({basename, m_Worker});
  // view->set_buffer(m_Worker.get_rx_buffer());
  // auto buffer = view->get_buffer();
  // try {
  //   char *contents = nullptr;
  //   gsize length = 0;

  //   file->load_contents(contents, length);
  //   buffer->set_text(contents, contents + length);
  //   g_free(contents);
  // } catch (const Glib::Error &ex) {
  //   std::cout << "ExampleAppWindow::open_file_view(\"" <<
  //   file->get_parse_name()
  //             << "\"):\n  " << ex.what() << std::endl;
  // }

  // auto tag = buffer->create_tag();
  // m_settings->bind("font", tag->property_font());
  // buffer->apply_tag(tag, buffer->begin(), buffer->end());

  // m_search->set_sensitive(true);

  // update_words();
  // update_lines();
}

void SPMAppWindow::on_search_text_changed() {
  // const auto text = m_searchentry->get_text();
  // auto tab = dynamic_cast<Gtk::ScrolledWindow
  // *>(m_stack->get_visible_child()); if (!tab) {
  //   std::cout << "SPMAppWindow::on_search_text_changed(): No visible "
  //                "child."
  //             << std::endl;
  //   return;
  // }
  // auto view = dynamic_cast<Gtk::TextView *>(tab->get_child());
  // if (!view) {
  //   std::cout << "SPMAppWindow::on_search_text_changed(): No visible text
  //   view"
  //             << std::endl;
  //   return;
  // }

  // // Very simple-minded search implementation
  // auto buffer = view->get_buffer();
  // Gtk::TextIter match_start;
  // Gtk::TextIter match_end;
  // if (buffer->begin().forward_search(text,
  //                                    Gtk::TextSearchFlags::CASE_INSENSITIVE,
  //                                    match_start, match_end)) {
  //   buffer->select_range(match_start, match_end);
  //   view->scroll_to(match_start);
  // }
}

void SPMAppWindow::on_visible_child_changed() {
  m_searchbar->set_search_mode(false);
  update_words();
  update_lines();
}

void SPMAppWindow::on_find_word(const Gtk::Button *button) {
  m_searchentry->set_text(button->get_label());
}

void SPMAppWindow::on_reveal_child_changed() { update_words(); }

void SPMAppWindow::on_clear_output(Gtk::TextView *textView) {
  textView->get_buffer()->set_text("");  // TODO; this is the correct clear way?
}

void SPMAppWindow::on_text_view_update(std::shared_ptr<SPWorker> worker,
                                       Gtk::TextView *textView) {
  auto buffer = textView->get_buffer();
  Gtk::TextBuffer::iterator iter = buffer->end();

  Glib::ustring entry = Glib::ustring(worker->get_rx_buffer()->data(),
                                      worker->get_rx_buffer()->size());

  buffer->insert(iter, entry);

  worker->clearRX();
}

void SPMAppWindow::update_words() {
  // auto tab = dynamic_cast<Gtk::ScrolledWindow
  // *>(m_stack->get_visible_child()); if (!tab) return;

  // auto view = dynamic_cast<Gtk::TextView *>(tab->get_child());
  // if (!view) {
  //   std::cout << "SPMAppWindow::update_words(): No visible text view"
  //             << std::endl;
  //   return;
  // }

  // auto buffer = view->get_buffer();
  // // Find all words in the text buffer
  // std::set<Glib::ustring> words;
  // auto start = buffer->begin();
  // Gtk::TextIter end;
  // while (start) {
  //   while (start && !start.starts_word()) start++;
  //   if (!start) break;

  //   end = start;
  //   end.forward_word_end();
  //   if (start == end) break;

  //   auto word = buffer->get_text(start, end, false);
  //   words.insert(word.lowercase());
  //   start = end;
  // }
  // while (auto child = m_words->get_first_child()) m_words->remove(*child);

  // for (const auto &word : words) {
  //   auto row = Gtk::make_managed<Gtk::Button>(word);
  //   row->signal_clicked().connect(
  //       sigc::bind(sigc::mem_fun(*this, &SPMAppWindow::on_find_word), row));
  //   m_words->append(*row);
  // }
}
void SPMAppWindow::update_lines() {
  // auto tab = dynamic_cast<Gtk::ScrolledWindow
  // *>(m_stack->get_visible_child()); if (!tab) return;

  // auto view = dynamic_cast<Gtk::TextView *>(tab->get_child());
  // if (!view) {
  //   std::cout << "ExampleAppWindow::update_lines(): No visible text view."
  //             << std::endl;
  //   return;
  // }
  // auto buffer = view->get_buffer();

  // int count = 0;
  // auto iter = buffer->begin();
  // while (iter) {
  //   ++count;
  //   if (!iter.forward_line()) break;
  // }
  // m_lines->set_text(Glib::ustring::format(count));
}

// Gtk::TextView *SPMAppWindow::get_text_view() {
//   // TODO: implemented
// }

void SPMAppWindow::on_action_open_serial_port() {
  try {
    auto window = dynamic_cast<Gtk::Window *>(this);
    auto open_serial_port_dialog = SPMOpenSerialPortDialog::create(*window);
    open_serial_port_dialog->present();

    open_serial_port_dialog->signal_hide().connect(
        [open_serial_port_dialog]() { delete open_serial_port_dialog; });
  } catch (const Glib::Error &ex) {
    std::cerr << "SPM::on_action_open_serial_port(): " << ex.what()
              << std::endl;
  } catch (const std::exception &ex) {
    std::cerr << "SPM::on_action_open_serial_port(): " << ex.what()
              << std::endl;
  }
}