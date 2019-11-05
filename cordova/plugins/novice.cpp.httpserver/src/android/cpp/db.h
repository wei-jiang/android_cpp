#pragma once
#include "common.h"
#include <sqlite3.h>
class DB
{
private:
    sqlite3 *db_;
    bool is_opened_;
public:
    DB(std::string path);
    ~DB();
    std::string exec_sql(const std::string& sql);
    std::string get_pass();
private:
    void init();
    int usr_count();
};
