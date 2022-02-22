/*
  Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS.

  GekkoFS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  GekkoFS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GKFS_COMMON_STATS_HPP
#define GKFS_COMMON_STATS_HPP

#include <cstdint>
#include <unistd.h>
#include <cassert>
#include <map>
#include <vector>
#include <deque>
#include <chrono>
#include <initializer_list>
/**
 * Provides storage capabilities to provide stats about GekkoFS
 * The information is per server. 
 * We do not provide accurate stats for 1-5-10 minute stats
 * 
 */
namespace gkfs::utils {

/*
    Number of operations (Create, write/ read, remove, mkdir...)
    Size of database (metadata keys, should be not needed, any)
    Size of data (+write - delete)
    Server Bandwidth (write / read operations)

    mean, (lifetime of the server)
    1 minute mean
    5 minute mean
    10 minute mean

    To provide the stats that we need, 
    we need to store the info and the timestamp to calculate it
    A vector should work, with a maximum of elements, 
    The stats will only be calculated when requested
    a cached value will be send (with a deadline)
    */
class Stats{
    enum class IOPS_OP {
        IOPS_CREATE,
        IOPS_WRITE,
        IOPS_READ,
        IOPS_MKDIR,
        IOPS_RMDIR,
        IOPS_REMOVE,
    };

    constexpr static const std::initializer_list<Stats::IOPS_OP> all_IOPS_OP = {IOPS_OP::IOPS_CREATE, IOPS_OP::IOPS_WRITE, IOPS_OP::IOPS_READ, IOPS_OP::IOPS_MKDIR,IOPS_OP::IOPS_RMDIR, IOPS_OP::IOPS_REMOVE};
   
    enum class SIZE_OP {
        METADATA_SIZE,
        WRITE_SIZE,
        READ_SIZE,
        DATA_SIZE      
    };

    constexpr static const std::initializer_list<Stats::SIZE_OP> all_SIZE_OP = {SIZE_OP::METADATA_SIZE, SIZE_OP::DATA_SIZE, SIZE_OP::WRITE_SIZE, SIZE_OP::READ_SIZE};

    std::chrono::time_point<std::chrono::steady_clock> last_cached;
    /* Measures when we started the server */
    std::chrono::time_point<std::chrono::steady_clock> start;
    // How many stats will be stored 
    const unsigned int MAX_STATS = 1000000; 

    // Stores total value for global mean
    std::map <IOPS_OP, unsigned long>  IOPS;
    std::map <SIZE_OP, unsigned long>  SIZE;


    // Stores timestamp when an operation comes
    // removes if first operation if > 10 minutes 
    // Different means will be stored and cached 1 minuted
    std::map <IOPS_OP, std::deque<  std::chrono::time_point<std::chrono::steady_clock> > > TIME_IOPS;
    // We will store 1, 5, and 10 minute mean;
    std::map <IOPS_OP, std::vector<double> > CACHED_IOPS;

    // For size operations we need to store the timestamp and
    // the size
    std::map <enum SIZE_OP, 
                std::deque < 
                    std::pair <  std::chrono::time_point<std::chrono::steady_clock> , unsigned long long > >
             > TIME_SIZE;
    // We will store 1, 5, and 10 minute mean;
    std::map < enum SIZE_OP, std::vector <double> > CACHED_SIZE; 

/**
 * @brief Starts the Stats module and initializes structures
 * 
 */
public: 
    Stats();


/**
 * Add a new value for a IOPS, that does not involve any size
 * No value needed as they are simple (1 create, 1 read...)
 * Size operations internally call this operation (read,write)
 *
 * @param IOPS_OP Which operation to add
 */

void add_value_iops (enum IOPS_OP);   

/**
 * @brief Store a new stat point, with a size value.
 * If it involves a IO operations it will call the corresponding
 * operation 
 * 
 * @param SIZE_OP Which operation we refer
 * @param value to store (SIZE_OP) 
 */
void add_value_size (enum SIZE_OP, unsigned long long value);

/**
 * @brief Get the total mean value of the asked stat
 * This can be provided inmediately without cost
 * @return mean value
 */
double get_mean (enum IOPS_OP);


/**
 * @brief Get the total mean value of the asked stat
 * This can be provided inmediately without cost
 * @return mean value
 */
double get_mean (enum SIZE_OP);

/**
 * @brief Get all the means (total, 1,5 and 10 minutes) for a SIZE_OP
 * Returns precalculated values if we just calculated them 1 minute ago
 * 
 * @return std::vector< double > with 4 means
 */
std::vector< double > get_four_means (enum SIZE_OP);

/**
 * @brief Get all the means (total, 1,5 and 10 minutes) for a IOPS_OP
 * Returns precalculated values if we just calculated them 1 minute ago
 * 
 * @return std::vector< double > with 4 means
 */
std::vector< double > get_four_means (enum IOPS_OP);
};

} // namespace gkfs::utils

#endif // GKFS_COMMON_STATS_HPP