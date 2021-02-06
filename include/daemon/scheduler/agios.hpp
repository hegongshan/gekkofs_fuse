/*
  Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef IFS_SCHEDULER_HPP
#define IFS_SCHEDULER_HPP

#include <agios.h>

void
agios_initialize();

void
agios_shutdown();

void*
agios_callback(int64_t request_id);

void*
agios_callback_aggregated(int64_t* requests, int32_t total);

void*
agios_eventual_callback(int64_t request_id, void* info);

unsigned long long int
generate_unique_id();

#endif