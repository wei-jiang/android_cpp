#include "ws_svr.h"
#include "util.h"
using namespace std;
WsSvr::WsSvr()
{
    init();
}
void WsSvr::init()
{
    ep_for_cpp();
}
void WsSvr::to_all(const std::string& json)
{
    auto &cpp_channel_endpoint = ws_server_.endpoint["^/cpp_channel/?$"];
    for(auto &a_connection : cpp_channel_endpoint.get_connections())
      a_connection->send(json);
}
void WsSvr::ep_for_cpp()
{
    auto &id_card_endpoint = ws_server_.endpoint["^/cpp_channel/?$"];
    id_card_endpoint.on_open = [&](shared_ptr<WsServer::Connection> connection) {
        auto out_message = make_shared<string>( Util::get_files_json("/sdcard/mystore/") );
        connection->send(*out_message);
        cout << "Server: Opened connection " << connection.get() << endl;
    };

    id_card_endpoint.on_close = [&](shared_ptr<WsServer::Connection> connection, int status, const string &reason) {
        cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
    };

    id_card_endpoint.on_error = [&](shared_ptr<WsServer::Connection> connection, const boost::system::error_code &ec) {
        cerr << "Server: Error in connection " << connection.get() << ". "
             << "Error: " << ec << ", error message: " << ec.message() << endl;
    };
    id_card_endpoint.on_message = [](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message) {
        auto out_message = in_message->string();
        cout << "Server: Message received: \"" << out_message << "\" from " << connection.get() << endl;
        cout << "Server: Sending message \"" << out_message << "\" to " << connection.get() << endl;
        // connection->send is an asynchronous function
        connection->send(out_message, [](const SimpleWeb::error_code &ec) {
            if (ec)
            {
                cout << "Server: Error sending message. " <<
                    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                    "Error: " << ec << ", error message: " << ec.message() << endl;
            }
        });
    };
}