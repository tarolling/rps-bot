#pragma once

#include <deque>
#include <map>
#include <string>
#include <thread>
#include <vector>

class state_t {
  class RPSModule *creator;

public:
  time_t next_tick;
  bool terminating;
  state_t(const state_t &) = default;
  state_t();
  state_t(class RPSModule *_creator, uint32_t questions, uint32_t currstreak,
          uint64_t lastanswered, uint32_t question_index, uint32_t _interval,
          uint64_t channel_id, bool hintless,
          const std::vector<std::string> &shuffle_list, uint64_t guild_id);
  ~state_t();
  state_t(state_t &&other) noexcept;                 // move constructor
  state_t &operator=(const state_t &other) = delete; // copy assignment
  state_t &operator=(state_t &&other) noexcept;      // move assignment
  void tick();
};