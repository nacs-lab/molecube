/*************************************************************************
 *   Copyright (c) 2015 - 2015 Yichao Yu <yyc1992@gmail.com>             *
 *                                                                       *
 *   This library is free software; you can redistribute it and/or       *
 *   modify it under the terms of the GNU Lesser General Public          *
 *   License as published by the Free Software Foundation; either        *
 *   version 3.0 of the License, or (at your option) any later version.  *
 *                                                                       *
 *   This library is distributed in the hope that it will be useful,     *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU    *
 *   Lesser General Public License for more details.                     *
 *                                                                       *
 *   You should have received a copy of the GNU Lesser General Public    *
 *   License along with this library. If not,                            *
 *   see <http://www.gnu.org/licenses/>.                                 *
 *************************************************************************/

#include <fcgio.h>
#include <fcgi_config.h>
// gcc through (pure virtual method called) error at runtime when
// using std::thread.
#include <pthread.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

#include <stdio.h>

using namespace std::literals;

std::atomic<int> counter(0);

static void*
process_request(void*)
{
    FCGX_Request request;
    FCGX_InitRequest(&request, 0, 0);
    std::cerr << "Thread " << std::this_thread::get_id()
              << " initialized" << std::endl;
    while (FCGX_Accept_r(&request) == 0) {
        std::cerr << "Start processing in " << std::this_thread::get_id()
                  << std::endl;
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        std::ostream out(&cout_fcgi_streambuf);
        out << "Content-type: text/plain; charset=UTF-8\r\n\r\n";
        out << "Start sleeping" << counter << std::endl;
        std::this_thread::sleep_for(10s);
        std::cerr << "Thread " << std::this_thread::get_id()
                  << " woke up" << std::endl;
        out << "Reply" << ++counter << std::endl;
        std::cerr << "Finish processing in " << std::this_thread::get_id()
                  << std::endl;
    }
    return nullptr;
}

static constexpr int num_worker = 10;

int
main()
{
    FCGX_Init();
    fprintf(stderr, "Program started\n");
    pthread_t threads[num_worker];
    for (int i = 0;i < num_worker;i++) {
        pthread_create(threads + i, nullptr, process_request, nullptr);
    }
    for (int i = 0;i < num_worker;i++) {
        pthread_join(threads[i], nullptr);
    }
    return 0;
}
