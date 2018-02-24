#pragma once

#include <chrono>
#include <string>
#include <iostream>

extern "C" {
#include "statsd-client.h"
}

namespace fxbattle {

class statsd {
public:
  class timing_scope{

    using clock = std::chrono::high_resolution_clock;
    statsd &_statsd;
    const std::string &stat;
    clock::time_point start;
    bool finished = false;
  public:
    timing_scope(statsd &sd, std::string&&) = delete;

    timing_scope(statsd &sd, const std::string& s)
        : _statsd(sd), stat(s), start(clock::now()) 
    {
    }

    ~timing_scope()
    {
      if (finished)
        return;
      auto end = clock::now();
      auto elapsed =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count();
      _statsd.timing(stat, elapsed);
    }

    timing_scope(timing_scope&& ts):_statsd(ts._statsd), stat(ts.stat), start(ts.start){
      ts.finished = true;
    }
  };

private:
  statsd_link *_link = nullptr;

  void start_impl(const std::string &host, int port, const std::string &namesp) {
    _link = statsd_init_with_namespace(const_cast<char *>(host.c_str()), port,
                                       const_cast<char *>(namesp.c_str()));
  }

public:
  bool start(const std::string &host, int port,
             const std::string &namesp = "fxbattle") {
    if (host.empty() || port <= 0)
      return false;

    std::cout << "logging stats to host: " << host << ':' << port << '\n';

    start_impl(host, port, namesp);

    return true;
  }

  ~statsd() {
    if (!_link)
      return;
    statsd_finalize(_link);
  }

  int inc(const std::string &stat, float rate = 1.0f) {
    if (!_link)
      return 1;
    return statsd_inc(_link, const_cast<char *>(stat.c_str()), rate);
  }

  int dec(const std::string &stat, float rate = 1.0f) {
    if (!_link)
      return 1;
    return statsd_dec(_link, const_cast<char *>(stat.c_str()), rate);
  }

  int count(const std::string &stat, std::size_t count, float rate = 1.0f) {
    if (!_link)
      return 1;
    return statsd_count(_link, const_cast<char *>(stat.c_str()), count, rate);
  }

  int gauge(const std::string &stat, std::size_t value) {
    if (!_link)
      return 1;
    return statsd_gauge(_link, const_cast<char *>(stat.c_str()), value);
  }

  int timing(const std::string &stat, std::size_t timeunits) {
    if (!_link)
      return 1;
    return statsd_timing(_link, const_cast<char *>(stat.c_str()), timeunits);
  }

  int timing(const std::string &stat, std::size_t timeunits, float rate) {
    if (!_link)
      return 1;
    return statsd_timing_with_sample_rate(
        _link, const_cast<char *>(stat.c_str()), timeunits, rate);
  }
};
}


