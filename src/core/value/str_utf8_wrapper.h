#pragma once

#include <memory>
#include <iterator>
#include <string>

class str_utf8_wrapper{
private:
  // store the cached length in glong, paired with its string
  struct str_utf8_t {
    static constexpr glong LENGTH_UNKNOWN = -1;
    str_utf8_t() : u8str(), u8len(0) { }
    str_utf8_t(const std::string& s) : u8str(s) { }
    str_utf8_t(const char *cstr) : u8str(cstr) { }
    str_utf8_t(const char *cstr, size_t size, glong u8len) : u8str(cstr, size), u8len(u8len) { }
    const std::string u8str;
    glong u8len = LENGTH_UNKNOWN;
  };
  // private constructor for copying members
  explicit str_utf8_wrapper(const std::shared_ptr<str_utf8_t>& str_in) : str_ptr(str_in) { }

public:
  class iterator
  {
public:
    // iterator_traits required types:
    using iterator_category = std::forward_iterator_tag;
    using value_type = str_utf8_wrapper;
    using difference_type = void;
    using reference = value_type; // type used by operator*(), not actually a reference
    using pointer = void;
    iterator() : ptr(&nullterm) {} // DefaultConstructible
    iterator(const str_utf8_wrapper& str) : ptr(str.c_str()), len(char_len()) { }
    iterator(const str_utf8_wrapper& str, bool /*end*/) : ptr(str.c_str() + str.size()) { }

    iterator& operator++() { ptr += len; len = char_len(); return *this; }
    reference operator*() { return {ptr, len}; } // Note: returns a new str_utf8_wrapper **by value**, representing a single UTF8 character.
    bool operator==(const iterator& other) const { return ptr == other.ptr; }
    bool operator!=(const iterator& other) const { return ptr != other.ptr; }
private:
    size_t char_len();
    static const char nullterm = '\0';
    const char *ptr;
    size_t len = 0;
  };

  iterator begin() const { return iterator(*this); }
  iterator end() const { return iterator(*this, true); }
  str_utf8_wrapper() : str_ptr(std::make_shared<str_utf8_t>()) { }
  str_utf8_wrapper(const std::string& s) : str_ptr(std::make_shared<str_utf8_t>(s)) { }
  str_utf8_wrapper(const char *cstr) : str_ptr(std::make_shared<str_utf8_t>(cstr)) { }
  // for enumerating single utf8 chars from iterator
  str_utf8_wrapper(const char *cstr, size_t clen) : str_ptr(std::make_shared<str_utf8_t>(cstr, clen, 1)) { }
  str_utf8_wrapper(const str_utf8_wrapper&) = delete; // never copy, move instead
  str_utf8_wrapper& operator=(const str_utf8_wrapper&) = delete; // never copy, move instead
  str_utf8_wrapper(str_utf8_wrapper&&) = default;
  str_utf8_wrapper& operator=(str_utf8_wrapper&&) = default;
  str_utf8_wrapper clone() const { return str_utf8_wrapper(this->str_ptr); } // makes a copy of shared_ptr

  bool operator==(const str_utf8_wrapper& rhs) const { return this->str_ptr->u8str == rhs.str_ptr->u8str; }
  bool operator!=(const str_utf8_wrapper& rhs) const { return this->str_ptr->u8str != rhs.str_ptr->u8str; }
  bool operator<(const str_utf8_wrapper& rhs) const { return this->str_ptr->u8str < rhs.str_ptr->u8str; }
  bool operator>(const str_utf8_wrapper& rhs) const { return this->str_ptr->u8str > rhs.str_ptr->u8str; }
  bool operator<=(const str_utf8_wrapper& rhs) const { return this->str_ptr->u8str <= rhs.str_ptr->u8str; }
  bool operator>=(const str_utf8_wrapper& rhs) const { return this->str_ptr->u8str >= rhs.str_ptr->u8str; }
  bool empty() const { return this->str_ptr->u8str.empty(); }
  const char *c_str() const { return this->str_ptr->u8str.c_str(); }
  const std::string& toString() const { return this->str_ptr->u8str; }
  size_t size() const { return this->str_ptr->u8str.size(); }

  glong get_utf8_strlen() const {
    if (str_ptr->u8len == str_utf8_t::LENGTH_UNKNOWN) {
      str_ptr->u8len = g_utf8_strlen(str_ptr->u8str.c_str(), str_ptr->u8str.size());
    }
    return str_ptr->u8len;
  }

private:
  std::shared_ptr<str_utf8_t> str_ptr;
};
