#pragma once
#ifndef _GNU_SOURCE 
#define _GNU_SOURCE 
#endif 

#include <iostream>
#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

namespace error_handler{

typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;

template <class E>
void throw_with_trace(const E& e) {
    throw boost::enable_error_info(e)
        << traced(boost::stacktrace::stacktrace());
}

template <class E>
void print_stacktrace(const E& e){
    const auto* st = boost::get_error_info<traced>(e);
    if (st) {
        std::cerr << *st << '\n';
    }
}

}



