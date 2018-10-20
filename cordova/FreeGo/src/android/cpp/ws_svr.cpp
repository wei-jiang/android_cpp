#include "ws_svr.h"
#include "util.h"
using namespace std;
WsSvr::WsSvr()
{
    init();
}
void WsSvr::init()
{
    handlers_["del_file"] = [this](pt::ptree &json, std::shared_ptr<WsServer::Connection> cnn) {
        const string &path = json.get<std::string>("path");
        fs::remove(path);
        to_all( Util::get_files_json() );
        json.put("ret", 0);
        return_json(cnn, json);
    };
    handlers_["rename_file"] = [this](pt::ptree &json, std::shared_ptr<WsServer::Connection> cnn) {
        const string &path = json.get<std::string>("path");
        const string &new_name = json.get<std::string>("new_name");
        g_logger->info("c++ rename {} to {}", path, new_name);
        fs::rename(path, new_name);
        to_all( Util::get_files_json() );
        json.put("ret", 0);
        return_json(cnn, json);
    };
    ep_for_cpp();
}
void WsSvr::to_all(const std::string &json)
{
    auto &cpp_channel_endpoint = ws_server_.endpoint["^/cpp_channel/?$"];
    for (auto &a_connection : cpp_channel_endpoint.get_connections())
        a_connection->send(json);
}
void WsSvr::ep_for_cpp()
{
    auto &id_card_endpoint = ws_server_.endpoint["^/cpp_channel/?$"];
    id_card_endpoint.on_open = [&](shared_ptr<WsServer::Connection> connection) {
        auto out_message = make_shared<string>(Util::get_files_json());
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
    id_card_endpoint.on_message = [this](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message) {
        auto cmd_json = in_message->string();
        std::stringstream ss;
        ss << cmd_json;
        pt::ptree json;
        pt::read_json(ss, json);
        const string &cmd = json.get<std::string>("cmd");
        auto it = handlers_.find(cmd);
        if (it != handlers_.end())
            return it->second(json, connection);
        // cout << "Server: Message received: \"" << out_message << "\" from " << connection.get() << endl;
        // cout << "Server: Sending message \"" << out_message << "\" to " << connection.get() << endl;
        // connection->send is an asynchronous function
        json.put("ret", -1);
		json.put("msg", boost::format( R"foo(can not find command dealer of %1%")foo") % cmd);
        return_json(connection, json);
    };
}
void WsSvr::return_json(std::shared_ptr<WsServer::Connection> cnn, pt::ptree& json)
{
    stringstream ss;
    pt::write_json(ss, json);
    cnn->send(ss.str(), [](const SimpleWeb::error_code &ec) {
        if (ec)
        {
            cout << "Server: Error sending message. " <<
                // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                "Error: " << ec << ", error message: " << ec.message() << endl;
        }
    });
}
// void return_nok(std::shared_ptr<WsServer::Connection> cnn, const std::string &msg)
// {
//     string ret_json = boost::format(R"foo(
//         {
//             "ret": -1,
//             "msg": "%1%"
//         }
//         )foo") % msg;
//     cnn->send(ret_json, [](const SimpleWeb::error_code &ec) {
//         if (ec)
//         {
//             cout << "Server: Error sending message. " <<
//                 // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
//                 "Error: " << ec << ", error message: " << ec.message() << endl;
//         }
//     });
// }