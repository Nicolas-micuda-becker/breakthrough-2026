#include <cstdlib>   
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <chrono>
#include "bkbb64.h"

int main(int _ac, char**_av) {
  print_playout_perf_per_sec(false);
  print_playout_perf_per_sec(false);
  print_playout_perf_per_sec(true);
  return 0;
}
