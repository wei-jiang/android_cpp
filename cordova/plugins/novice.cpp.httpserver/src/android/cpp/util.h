#pragma once
#include "common.h"
namespace Util
{
    std::vector<std::string> split(const std::string &s, std::string delim = ",");
    std::string join(const std::vector<std::string> &ss, std::string delim = ",");
    std::string byte2str(uint8_t *bytes, int size);
    std::string hexStr(const std::string& buff);
    std::string string_to_hex(const std::string& input);
    std::string random_string( size_t length );
    std::string decodeHex(const std::string & source);
    std::string get_files_json(const std::string &path = "/sdcard/mystore/");
    std::string urlEncode(const std::string &url);
    std::string urlDecode(const std::string &encoded);
    std::string charToHex(unsigned char c);
    unsigned char hexToChar(const std::string &str);
    std::string refresh_files_noty();
    std::string file_type(const std::string &path);
    std::string get_tid();
    std::string mime_type(const std::string &path);
    bool is_pac(const std::string &path);
};
