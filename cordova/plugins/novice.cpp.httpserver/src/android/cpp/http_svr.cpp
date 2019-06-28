
#include "http_svr.h"
#include "util.h"
using namespace std;
using namespace boost::property_tree;

string g_ms;
HttpSvr::HttpSvr(int port, const std::string& dir)
:pub_dir_(dir),store_path_("/sdcard/mystore/")
{
    g_ms = dir;
    boost::replace_all(g_ms, "www", "magic.mgc");
    // g_ms = regex_replace(pub_dir_, regex("www"), "magic.mgc");
    g_logger->debug("magic_source = {}", g_ms);
    server_.config.port = port;
    server_.io_service = g_io;
    init();
}
void HttpSvr::ws_to_all(const std::string &json)
{
    g_io->post([this, json] {
        ws_svr_.to_all(json);
    });
}
void HttpSvr::init()
{    
    if( !fs::exists(store_path_) )
    {
        fs::create_directory(store_path_);
    }
    handle_upload();
    serve_res();
    static_dir(pub_dir_);
    post_test();
    emplace_ws();
    server_.start();
}
void HttpSvr::emplace_ws()
{
    auto &ws_server = ws_svr_.get_ws_svr();
    server_.on_upgrade = [&ws_server](unique_ptr<SimpleWeb::HTTP> &socket, shared_ptr<HttpServer::Request> request) {
        auto connection = std::make_shared<WsServer::Connection>(std::move(socket));
        connection->method = std::move(request->method);
        connection->path = std::move(request->path);
        connection->http_version = std::move(request->http_version);
        connection->header = std::move(request->header);
        connection->remote_endpoint = std::move(*request->remote_endpoint);
        ws_server.upgrade(connection);
    };
}

void HttpSvr::static_dir(const std::string &dir)
{
    server_.default_resource["GET"] = [dir](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
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
            // Uncomment the following line to enable Cache-Control
            // header.emplace("Cache-Control", "max-age=86400");

            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

            if (*ifs)
            {
                auto length = ifs->tellg();
                ifs->seekg(0, ios::beg);

                header.emplace("Content-Length", to_string(length));
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
void HttpSvr::post_test()
{
    server_.resource["^/json$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            ptree pt;
            read_json(request->content, pt);

            auto name = pt.get<string>("firstName") + " " + pt.get<string>("lastName");
            // cout<< name <<endl;
            name = "some chinese"; 
            *response << "HTTP/1.1 200 OK\r\n"
                      << "Content-Type: charset=gbk\r\n"
                      << "Content-Length: " << name.length() << "\r\n\r\n"
                      << name;
        }
        catch (const exception &e)
        {
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
                      << e.what();
        }

        // Alternatively, using a convenience function:
        // try {
        //     ptree pt;
        //     read_json(request->content, pt);

        //     auto name=pt.get<string>("firstName")+" "+pt.get<string>("lastName");
        //     response->write(name);
        // }
        // catch(const exception &e) {
        //     response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
        // }
    };
}
void HttpSvr::handle_upload()
{
    server_.resource["^/upload$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            const string& data = request->content.string();
            // g_logger->info( Util::hexStr(data) );
            const string file_name{ data.c_str() };
            // g_logger->info("file_name: {}", file_name);
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
            // cout<< name <<endl;
            string name = "蒋维"; //Util::gbk_to_utf8("��ά");
            *response << "HTTP/1.1 200 OK\r\n"
                    //   << "Content-Type: charset=gbk\r\n"
                      << "Content-Length: " << name.length() << "\r\n\r\n"
                      << name;
        }
        catch (const exception &e)
        {
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
                      << e.what();
        }
    };
}
void HttpSvr::serve_res()
{
    server_.resource["^/store/(.+)$"]["GET"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            auto fn = request->path_match[1].str();
            fn = Util::urlDecode(fn);
            auto store_root_path = fs::canonical(store_path_);
            auto path = fs::canonical(store_root_path / fn);
            if (fs::is_directory(path))
                path /= "index.html";

            SimpleWeb::CaseInsensitiveMultimap header;
            // enable Cache-Control
            // header.emplace("Cache-Control", "max-age=86400");
            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

            if (*ifs)
            {
                auto length = ifs->tellg();
                ifs->seekg(0, ios::beg);

                header.emplace("Content-Length", to_string(length));
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
            response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path_match[1].str() + ": " + e.what());
        }
    };
}
