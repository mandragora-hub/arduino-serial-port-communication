#include <bits/stdc++.h>

namespace Utils {

// TODO: move template t in a way no only works with std::vector but every type
// could be using with find
template <typename T>
int indexOf(std::vector<T> v, T el) {
  auto it = find(v.begin(), v.end(), el);
  int index = it - v.begin();
  return index;
}

int find_index_in_string_list(const Glib::RefPtr<Gtk::StringList>& string_list,
                              const Glib::ustring& target_string) {
  // Check if the list is valid
  if (!string_list) return -1;

  // Iterate through each item in the list
  for (guint i = 0; i < string_list->get_n_items(); ++i) {
    // Get the item at the current index
    Glib::RefPtr<Glib::ObjectBase> item_base = string_list->get_object(i);

    // Attempt to cast the item to a Gtk::StringObject
    // Use dynamic_pointer_cast because Glib::ObjectBase is a virtual base
    Glib::RefPtr<Gtk::StringObject> string_object =
        std::dynamic_pointer_cast<Gtk::StringObject>(item_base);

    // If the cast was successful and the string matches, return the index
    if (string_object && string_object->get_string() == target_string) {
      return static_cast<int>(i);  // Found the string at this index
    }
  }

  // If the loop finishes, the string was not found in the list
  return -1;
}

// Helper function to find a child widget by its type and relative position
// within a parent Gtk::Box, using GTK4 traversal methods.
// I dont like look for child widget in that way. Replace if we catch some way better
Gtk::Widget* get_nth_child_of_box(Gtk::Box* parent_box, int n)
{
    if (!parent_box) return nullptr;

    Gtk::Widget* current_child = parent_box->get_first_child();
    int count = 0;
    while (current_child)
    {
        if (count == n)
        {
            return current_child;
        }
        current_child = current_child->get_next_sibling();
        count++;
    }
    return nullptr; // Child not found at this position
}

};  // namespace Utils