#pragma once

#include <voy/config.hpp>

namespace voy::cli {

std::expected<voy::config::VoyConfig, std::string> parse_config_file(const std::string& path);

}
