
#include "http_home.h"
#include "util.h"

using namespace std;

HttpHome::HttpHome(int port, const std::string& dir)
:assets_dir_(dir),home_path_(dir + "/home"),port_(port)
{
    server_ = make_shared<HttpServer>();
    db_ = make_shared<DB>(dir);
    server_->config.port = port;
    server_->io_service = g_io_home;
    init();
}
HttpHome::~HttpHome()
{
    server_->stop();
}

void HttpHome::init()
{    
    if( !fs::exists(home_path_) )
    {
        fs::create_directory(home_path_);
        ofstream index (home_path_+"/index.html");
        if (index.is_open())
        {
            index << "<h1>Hello World!</h1>";
            index.close();
        }
    }
    handle_sql();
    static_dir();
    server_->start();
}

void HttpHome::static_dir()
{
    server_->default_resource["GET"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            auto home_root = boost::filesystem::canonical(home_path_);
            auto path = boost::filesystem::canonical(home_root / request->path);

            if (boost::filesystem::is_directory(path))
                path /= "index.html";

            SimpleWeb::CaseInsensitiveMultimap header; 
            header.emplace("Cache-Control", "max-age=86400");           
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
                        vector<char> buffer(131072);
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
                                        LOGE("get home file connection interrupted");
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

void HttpHome::handle_sql()
{
    server_->resource["^/sql$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json;charset=utf-8");
        header.emplace("Connection", "keep-alive");
        // throw exception hurt performance, so try to not use: try...catch...throw
        auto data = json::parse(request->content);
        auto pass = data["pass"].get<string>();
        auto sql = data["sql"].get<string>();           
        // LOGI("handle_sql : pass=%s; sql=%s", pass.c_str(), sql.c_str());
        if(db_->get_pass() == pass) 
        {
            response->write(db_->exec_sql(sql), header);
        }
        else 
        {
            json res;
            res["ret"] = -1;
            res["msg"] = "invalid pass";
            response->write(res.dump(), header);
        }
    };
}
