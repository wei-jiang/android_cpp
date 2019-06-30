
#include <magic.h>
#include "util.h"

using namespace std;

namespace Util
{

vector<string> split(const string &s, std::string delim)
{
	vector<string> strs;
	boost::split(strs, s, boost::is_any_of(delim));
	return strs;
}
std::string join(const std::vector<std::string> &ss, std::string delim)
{
	std::string joined = boost::algorithm::join(ss, delim);
	return joined;
}
std::string byte2str(uint8_t *bytes, int size)
{
	static char const hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	std::string str;
	for (int i = 0; i < size; ++i)
	{
		const char ch = bytes[i];
		str.append(&hex[(ch & 0xF0) >> 4], 1);
		str.append(&hex[ch & 0xF], 1);
	}
	return str;
}
std::string hexStr(const std::string &buff)
{
	constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
							   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	std::string s(buff.size() * 2, ' ');
	for (auto i = 0; i < buff.size(); ++i)
	{
		s[2 * i] = hexmap[(buff[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[buff[i] & 0x0F];
	}
	return s;
}

std::string get_files_json(const std::string &path)
{
	static magic_t handle = NULL;
	if (!handle)
	{
		handle = ::magic_open(MAGIC_NONE | MAGIC_MIME_TYPE);
		if (handle == NULL)
		{
			LOGD("initialize magic library failed");
		}
		if( ::magic_load( handle, g_ms.c_str() ) != 0 )
		{
			LOGD("cannot load magic database -%s", magic_error(handle));
			magic_close(handle);
		}
		else
		{
			LOGD("load magic database succeed!");
		}
	}
	pt::ptree root;
	stringstream ss;
	root.put("cmd", "get_file_list");
	fs::path p(path);
	std::vector<fs::path> v;

	for (auto &&x : fs::directory_iterator(p))
		v.push_back(x.path());
	// leave client to sort
	// std::sort(v.begin(), v.end());
	pt::ptree files;
	for (auto &&x : v)
	{
		pt::ptree fi;
		string fn = x.filename().native();
		
		std::time_t t = fs::last_write_time(x.native());
		std::tm tm = *std::localtime(&t);
		std::ostringstream ss;
		ss << std::put_time(&tm, "%F %T");
		std::string time_str = ss.str();
		fi.put("name", fn);
		fi.put("time", time_str);
		fi.put("path", x.native());
		if( fs::is_directory(x) )
		{
			fi.put("is_dir", 'Y');
			files.push_back(std::make_pair("", fi));
			continue;
		}
		auto type = ::magic_file(handle, x.native().c_str());
		fi.put("type", (type ? type : "UNKOWN"));
		fi.put("size", fs::file_size(x));
		fi.put("ext", x.extension());
		files.push_back(std::make_pair("", fi));
	}
	root.add_child("files", files);

	pt::write_json(ss, root);
	return ss.str();
}
std::string refresh_files_noty()
{
	pt::ptree root;
	stringstream ss;
	root.put("cmd", "refresh_file_list");
	pt::write_json(ss, root);
	return ss.str();
}
std::string string_to_hex(const std::string &input)
{
	static const char *const lut = "0123456789ABCDEF";
	size_t len = input.length();

	std::string output;
	output.reserve(2 * len);
	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = input[i];
		output.push_back(lut[c >> 4]);
		output.push_back(lut[c & 15]);
	}
	return output;
}
std::string random_string(size_t length)
{
	auto randchar = []() -> char {
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}
std::string decodeHex(const std::string &source)
{
	if (std::string::npos != source.find_first_not_of("0123456789ABCDEFabcdef"))
	{
		// you can throw exception here
		return {};
	}

	union {
		uint64_t binary;
		char byte[8];
	} value{};

	auto size = source.size(), offset = (size % 16);
	std::vector<uint8_t> binary{};
	binary.reserve((size + 1) / 2);

	if (offset)
	{
		value.binary = std::stoull(source.substr(0, offset), nullptr, 16);

		for (auto index = (offset + 1) / 2; index--;)
		{
			binary.emplace_back(value.byte[index]);
		}
	}

	for (; offset < size; offset += 16)
	{
		value.binary = std::stoull(source.substr(offset, 16), nullptr, 16);
		for (auto index = 8; index--;)
		{
			binary.emplace_back(value.byte[index]);
		}
	}

	return string((char *)&binary[0], binary.size());
}

std::string urlEncode(const std::string &toEncode)
{
	std::ostringstream out;

	for (std::string::size_type i = 0; i < toEncode.length(); ++i)
	{
		short t = toEncode.at(i);

		if (
			t == 45 ||				 // hyphen
			(t >= 48 && t <= 57) ||  // 0-9
			(t >= 65 && t <= 90) ||  // A-Z
			t == 95 ||				 // underscore
			(t >= 97 && t <= 122) || // a-z
			t == 126				 // tilde
		)
		{
			out << toEncode.at(i);
		}
		else
		{
			out << charToHex(toEncode.at(i));
		}
	}

	return out.str();
}

std::string urlDecode(const std::string &toDecode)
{
	std::ostringstream out;

	for (std::string::size_type i = 0; i < toDecode.length(); ++i)
	{
		if (toDecode.at(i) == '%')
		{
			std::string str(toDecode.substr(i + 1, 2));
			out << hexToChar(str);
			i += 2;
		}
		else
		{
			out << toDecode.at(i);
		}
	}

	return out.str();
}

std::string charToHex(unsigned char c)
{
	short i = c;

	std::stringstream s;

	s << "%" << std::setw(2) << std::setfill('0') << std::hex << i;

	return s.str();
}

unsigned char hexToChar(const std::string &str)
{
	short c = 0;

	if (!str.empty())
	{
		std::istringstream in(str);

		in >> std::hex >> c;

		if (in.fail())
		{
			throw std::runtime_error("stream decode failure");
		}
	}

	return static_cast<unsigned char>(c);
}
} // namespace Util
