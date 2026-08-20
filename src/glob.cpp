#include <voy/glob.hpp>

namespace voy::glob {

GlobMatcher::GlobMatcher(std::string_view pattern)
    : pattern_(pattern),
      regex_(to_regex_string(pattern),
             std::regex_constants::ECMAScript | std::regex_constants::optimize) {}

bool GlobMatcher::matches(std::string_view path) const {
  return std::regex_match(path.begin(), path.end(), regex_);
}

std::string GlobMatcher::to_regex_string(std::string_view glob) {
  std::string regex_str;
  regex_str.reserve(glob.size() * 2);

  regex_str += "^";

  for (size_t i = 0; i < glob.size(); ++i) {
    char c = glob[i];

    switch (c) {
      case '*':
        if (i + 1 < glob.size() && glob[i + 1] == '*') {
          i++;
          if (i + 1 < glob.size() && glob[i + 1] == '/') {
            i++;
            regex_str += "(?:.*/)?";  // Handles **/*.cpp and a/**/b
          } else {
            regex_str += ".*";
          }
        } else {
          regex_str += "[^/]*";  // Single * stops at directories
        }
        break;

      case '?': regex_str += "[^/]"; break;

      case '.':
      case '+':
      case '\\':
      case '$':
      case '^':
      case '(':
      case ')':
      case '[':
      case ']':
      case '{':
      case '}':
      case '|':
        regex_str += '\\';
        regex_str += c;
        break;

      default: regex_str += c; break;
    }
  }

  regex_str += "$";
  return regex_str;
}

std::string GlobMatcher::extract_base_dir(std::string_view glob) {
  size_t first_glob_char = glob.find_first_of("*?");
  if (first_glob_char == std::string_view::npos) {
    return std::string(glob);
  }
  size_t last_slash = glob.find_last_of('/', first_glob_char);
  if (last_slash == std::string_view::npos) {
    return ".";
  }
  return std::string(glob.substr(0, last_slash));
}

}  // namespace voy::glob
