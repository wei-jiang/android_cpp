
#include "http_svr.h"
#include <unzipper.h>
#include "util.h"
#include "udp.h"
#include "peer.h"
#include "tunnel.h"
using namespace std;
using namespace boost::property_tree;

string g_ms;
HttpSvr::HttpSvr(int port, const std::string& dir)
:assets_dir_(dir),store_path_("/sdcard/mystore/"),port_(port)
{
    g_ms = dir + "/magic.mgc";
    // boost::replace_all(g_ms, "www", "magic.mgc");
    // g_ms = regex_replace(assets_dir_, regex("www"), "magic.mgc");
    // LOGI("magic_source = %s", g_ms.c_str());
    server_ = make_shared<HttpServer>();
    server_->config.port = port;
    server_->io_service = g_io;
    udp_ = make_shared<UdpSvr>(port); 
    peer_ = make_shared<Peer>(this);
    init();
}
HttpSvr::~HttpSvr()
{
    server_->stop();
}
void HttpSvr::ws_to_all(const std::string &json)
{
    g_io->post([this, json] {
        ws_svr_.to_all(json);
    });
}
void HttpSvr::res_json(std::shared_ptr<HttpServer::Response> response, json &data)
{
    SimpleWeb::CaseInsensitiveMultimap header;
    header.emplace("Content-Type", "application/json;charset=utf-8");
    header.emplace("Connection", "keep-alive");
    response->write(data.dump(), header);
}
void HttpSvr::init()
{    
    if( !fs::exists(store_path_) )
    {
        fs::create_directory(store_path_);
    }
    handle_upload();
    handle_upload_home();
    serve_res();
    static_dir(assets_dir_ + "/www");
    get_files();
    emplace_ws();
    client_info();
    server_->start();
    static boost::asio::deadline_timer routine_timer(*g_io, boost::posix_time::seconds(1));
    // can not use std::bind
    routine_timer.async_wait(boost::bind(&HttpSvr::routine, this, boost::asio::placeholders::error, &routine_timer));
}

void HttpSvr::routine(const boost::system::error_code& /*e*/, boost::asio::deadline_timer* t)
{
    auto tid = Util::get_tid();
    vector<shared_ptr<CNNBase>> for_del;
    // LOGI("thread[%s] in routine...", tid.c_str()); 
    // ws_svr_.to_all("cpp ws server alive");
    json noty_proxy_info;    
    for( auto const& [k, v] : SocksCnn1::s_peer2cnn )
    {
        noty_proxy_info[k] = v.size();
        // for(auto const& [k, n] : v) 
        // {
        //     if(n && n->elapsed_in_seconds() > 7)
        //     {
        //         for_del.push_back(n);
        //     }  
        // }
    }   
    // for(auto const& [k, n] : SocksCnn0::s_id2cnn) 
    // {
    //     if(n)
    //     {
    //         LOGI("%u elapsed_in_seconds: %ld", k, n->elapsed_in_seconds()); 
    //         if(n->elapsed_in_seconds() > 7)
    //         {
    //             for_del.push_back(n);
    //         }          
    //     }  
    // }
    noty_proxy_info["local_socks_cnn_count"] = SocksCnn0::s_id2cnn.size();
    noty_proxy_info["cmd"] = "noty_proxy_info";
    cpp2java_que.push(noty_proxy_info.dump());
    for(auto& n : for_del) {
        if(n)
        {
            n->remove_self();
        }        
    }
    t->expires_at(t->expires_at() + boost::posix_time::seconds(2));
    t->async_wait(boost::bind(&HttpSvr::routine, this,
                              boost::asio::placeholders::error, t));
}
void HttpSvr::emplace_ws()
{
    // https://gitlab.com/eidheim/desktop-stream/blob/master/main.cpp
    auto &ws_server = ws_svr_.get_ws_svr();
    server_->on_upgrade = [&ws_server](unique_ptr<SimpleWeb::HTTP> &socket, shared_ptr<HttpServer::Request> request) {
        auto connection = std::make_shared<WsServer::Connection>(std::move(socket));
        connection->method = std::move(request->method);
        connection->path = std::move(request->path);
        connection->http_version = std::move(request->http_version);
        connection->header = std::move(request->header);
        ws_server.upgrade(connection);
    };
}

void HttpSvr::static_dir(const std::string &dir)
{
    server_->default_resource["GET"] = [dir](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            auto web_root_path = boost::filesystem::canonical(dir);
            auto path = boost::filesystem::canonical(web_root_path / request->path);
            // Check if path is within web_root_path
            if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
                !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw invalid_argument("path must be within root path");
            if (boost::filesystem::is_directory(path))
                path /= "index.html";

            SimpleWeb::CaseInsensitiveMultimap header;
            if( Util::is_pac(path.string()) )
            {
                header.emplace("Cache-Control", "no-cache, no-store, must-revalidate");
            }
            else
            {
                header.emplace("Cache-Control", "max-age=86400");
            }
            
            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

            if (*ifs)
            {
                auto length = ifs->tellg();
                ifs->seekg(0, ios::beg);

                header.emplace("Content-Length", to_string(length));
                // determine type by extension
                header.emplace("Content-Type", Util::mime_type(path.string()) );
                response->write(header);

                // Trick to define a recursive function within this scope (for example purposes)
                class FileServer
                {
                  public:
                    static void read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs)
                    {
                        // Read and send 128 KB at a time
                        static vector<char> buffer(131072); // Safe when server is running on one thread
                        streamsize read_length;
                        if ((read_length = ifs->read(&buffer[0], static_cast<streamsize>(buffer.size())).gcount()) > 0)
                        {
                            response->write(&buffer[0], read_length);
                            if (read_length == static_cast<streamsize>(buffer.size()))
                            {
                                response->send([response, ifs](const SimpleWeb::error_code &ec) {
                                    if (!ec)
                                        read_and_send(response, ifs);
                                    else
                                        cerr << "Connection interrupted" << endl;
                                });
                            }
                        }
                    }
                };
                FileServer::read_and_send(response, ifs);
            }
            else
                throw invalid_argument("could not read file");
        }
        catch (const exception &e)
        {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
        }
    };
}
void HttpSvr::get_files()
{
    server_->resource["^/get_files$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try {
            ptree pt;
            read_json(request->content, pt);
            auto path = pt.get<string>("path");
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "application/json;charset=utf-8");
            header.emplace("Connection", "keep-alive");
            response->write( Util::get_files_json(path), header );
        }
        catch(const exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
        }
    };
}
void HttpSvr::client_info()
{
    server_->resource["^/info$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    stringstream stream;
    stream << "<h1>Request from " << request->remote_endpoint().address().to_string() << ":" << request->remote_endpoint().port() << "</h1>";

    stream << request->method << " " << request->path << " HTTP/" << request->http_version;

    stream << "<h2>Query Fields</h2>";
    auto query_fields = request->parse_query_string();
    for(auto &field : query_fields)
      stream << field.first << ": " << field.second << "<br>";

    stream << "<h2>Header Fields</h2>";
    for(auto &field : request->header)
      stream << field.first << ": " << field.second << "<br>";

    response->write(stream);
  };
}
void HttpSvr::handle_upload()
{
    server_->resource["^/upload$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            const string& data = request->content.string();
            // LOGD( Util::hexStr(data) );
            const string file_name{ data.c_str() };
            // LOGD("file_name: %s", file_name.c_str());
            auto flag = static_cast<int>(data[data.length() - 1]);
            // cout<< "flag=" <<flag <<endl;
            auto buff_len = data.length() - file_name.length() -2;
            // cout<< "buff_len=" <<buff_len <<endl;
            const string& buff = data.substr(file_name.length()+1, buff_len);
            const auto path = store_path_ + file_name;
            // cout<< "buff=" <<buff <<endl;
            std::shared_ptr<std::ofstream> writer;
            switch(flag)
            {
                case 0:
                    writer = writers_[file_name] = make_shared<std::ofstream>(path, std::ofstream::binary);
                    break;
                case 1:
                    writer = writers_[file_name];
                    break;
                case 2:
                    auto it = writers_.find(file_name);
                    if( it != writers_.end() )
                    {   
                        writer = it->second;
                        writers_.erase(it);
                    }
                    else 
                    {
                        writer = make_shared<ofstream>(path, std::ofstream::binary);
                    }
                    ws_to_all( Util::get_files_json(store_path_) );
                    break;
            }
            writer->write( buff.c_str(), buff.length() );
            // cout << Util::string_to_hex( data ) << endl; 

            string rs = "ok";
            *response << "HTTP/1.1 200 OK\r\n"
                      << "Content-Type: text/html; charset=utf-8\r\n"
                      << "Content-Length: " << rs.length() << "\r\n\r\n"
                      << rs;
        }
        catch (const exception &e)
        {
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
                      << e.what();
        }
    };
}
void HttpSvr::read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs, size_t len)
{
    if(len <= 0) return;
    // Read and send 128 KB at a time
    static const size_t buff_len = 131072;
    static vector<char> buffer(buff_len); // Safe when server is running on one thread
    size_t read_len = std::min(len, buff_len);
    streamsize read_length;
    if ((read_length = ifs->read( &buffer[0], static_cast<streamsize>(read_len) ).gcount()) > 0)
    {
        response->write(&buffer[0], read_length);
        // cout << "write buff len = " << read_length << endl;
        response->send([=](const SimpleWeb::error_code &ec) {
            if (!ec)
                read_and_send(response, ifs, len - read_length);
            else
                cerr << "Connection interrupted: " << ec.message() << endl;
        });  
    }
}
void HttpSvr::serve_res()
{
    server_->resource["^/store/(.+)$"]["GET"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            auto fn = request->path_match[1].str();
            fn = Util::urlDecode(fn);
            auto store_root_path = fs::canonical(store_path_);
            auto path = fs::canonical(store_root_path / fn);
            if (fs::is_directory(path))
                path /= "index.html";

            SimpleWeb::CaseInsensitiveMultimap header;
            // enable Cache-Control, except pac file
            if( Util::is_pac(path.string()) )
            {
                header.emplace("Cache-Control", "no-cache, no-store, must-revalidate");
            }
            else
            {
                header.emplace("Cache-Control", "max-age=86400");
            }
            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

            if (*ifs)
            {
                header.emplace("Content-Type", Util::mime_type(path.string()) );
                size_t file_len = ifs->tellg();
                ifs->seekg(0, ios::beg);
                size_t length = file_len;
                auto it = request->header.find("Range");
                if(it != request->header.end()) {
                    auto range = it->second;
                    boost::replace_all(range, "bytes=", "");
                    auto vs = Util::split(range, "-");
                    size_t begin = stoi( vs[0] );
                    size_t end = vs[1] == "" ? file_len : stoi( vs[1] );
                    // [begin, end] not [begin, end)
                    end = std::min( file_len - 1, end );
                    length = (end - begin) + 1;
                    range = "bytes " + vs[0] + "-" + to_string(end) + "/" + to_string(file_len);
                    header.emplace("Content-Range", range); 
                    header.emplace("Accept-Ranges", "bytes");
                    header.emplace("Content-Length", to_string(length));
                    response->write(SimpleWeb::StatusCode::success_partial_content, header);
                    ifs->seekg(begin, ios::beg);
                } 
                else 
                {
                    header.emplace("Content-Length", to_string(length));
                    response->write(header);
                }
                read_and_send(response, ifs, length);
            }
            else
                throw invalid_argument("could not read file");
        }
        catch (const exception &e)
        {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path_match[1].str() + ": " + e.what());
        }
    };
}
void HttpSvr::handle_upload_home()
{
    server_->resource["^/upload_home$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            const string& data = request->content.string();
            // LOGD( Util::hexStr(data) );
            const string file_name{ data.c_str() };
            // LOGD("file_name: %s", file_name.c_str());
            auto flag = static_cast<int>(data[data.length() - 1]);
            // cout<< "flag=" <<flag <<endl;
            auto buff_len = data.length() - file_name.length() -2;
            // cout<< "buff_len=" <<buff_len <<endl;
            const string& buff = data.substr(file_name.length()+1, buff_len);
            auto path = store_path_ + "home/";
            if( !fs::exists(path) )
            {
                fs::create_directory(path);
            }
            path += file_name;
            // cout<< "buff=" <<buff <<endl;
            std::shared_ptr<std::ofstream> writer;
            switch(flag)
            {
                case 0:
                    writer = writers_[file_name] = make_shared<std::ofstream>(path, std::ofstream::binary);
                    break;
                case 1:
                    writer = writers_[file_name];
                    break;
                case 2:
                    auto it = writers_.find(file_name);
                    if( it != writers_.end() )
                    {   
                        writer = it->second;
                        writers_.erase(it);
                    }
                    else 
                    {
                        writer = make_shared<ofstream>(path, std::ofstream::binary);
                    }
                    ws_to_all( Util::get_files_json(store_path_) );
                    break;
            }
            writer->write( buff.c_str(), buff.length() );
            // cout << Util::string_to_hex( data ) << endl; 
            if(2 == flag)
            {
                writer->close();
                string to_dir = assets_dir_+"/home";
                LOGI("extractZip=%s to %s", path.c_str(), to_dir.c_str() );
                using namespace zipper;
                Unzipper unzipper(path);
                unzipper.extract(to_dir);
                unzipper.close();
            }
            string rs = "ok";
            *response << "HTTP/1.1 200 OK\r\n"
                      << "Content-Type: text/html; charset=utf-8\r\n"
                      << "Content-Length: " << rs.length() << "\r\n\r\n"
                      << rs;
        }
        catch (const exception &e)
        {
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
                      << e.what();
        }
    };
}