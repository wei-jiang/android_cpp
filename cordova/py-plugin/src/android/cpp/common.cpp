
#include "common.h"

using namespace std;
std::atomic<uint16_t> g_channel_id = 0;
std::shared_ptr<boost::asio::io_context> g_io, g_socks_io;
TSQueue<std::string> cpp2java_que;
// boost::signals2::signal<void (uint16_t, const std::vector<uint8_t>& )> g_sig;