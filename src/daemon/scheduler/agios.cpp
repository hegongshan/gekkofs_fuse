/*
  Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <daemon/scheduler/agios.hpp>

unsigned long long int
generate_unique_id() {
    // Calculates the hash of this request
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int id = ts.tv_sec * 1000000000L + ts.tv_nsec;

    return id;
}