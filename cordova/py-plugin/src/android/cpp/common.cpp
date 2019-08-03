
#include "common.h"

using namespace std;

std::shared_ptr<boost::asio::io_context> g_io, g_socks_io;
TSQueue<std::string> cpp2java_que;