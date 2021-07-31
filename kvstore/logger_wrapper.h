/************************************************************************
Copyright 2017-2019 eBay Inc.
Author/Developer(s): Jung-Sang Ahn

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#pragma once

#include "libnuraft/nuraft.hxx"

// using namespace nuraft;


/**
 * Example implementation of Raft logger, on top of SimpleLogger.
 */
class logger_wrapper : public nuraft::logger {
public:
    logger_wrapper(const std::string& log_file, int log_level = 6) {
        log_level_ = log_level_;
    }

    ~logger_wrapper() {
        destroy();
    }

    void destroy() {
    }

    void put_details(int level,
                     const char* source_file,
                     const char* func_name,
                     size_t line_number,
                     const std::string& msg) override;

    void set_level(int l) override {
        log_level_ = l;
    }

    int get_level() override {
        return log_level_;
    }


private:
int log_level_{6};
};


