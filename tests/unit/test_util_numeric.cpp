/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <catch2/catch.hpp>
#include <global/chunk_calc_util.hpp>
#include <fmt/format.h>

constexpr auto test_repetitions = 250u;

SCENARIO(" offsets can be left-aligned to block size boundaries ",
         "[utils][numeric][chnk_lalign]") {

    using namespace gkfs::util;

    GIVEN(" a block size ") {

        const std::size_t block_size =
                GENERATE(filter([](uint64_t bs) { return is_power_of_2(bs); },
                                range(0, 100000)));

        WHEN(" offset is 0 ") {

            const uint64_t offset = 0;

            CAPTURE(offset, block_size);

            THEN(" the left-aligned offset is 0 ") {
                const uint64_t aligned_offset = chnk_lalign(offset, block_size);
                REQUIRE(aligned_offset == 0);
            }
        }

        WHEN(" offset is smaller than block size ") {

            const uint64_t offset = GENERATE_COPY(
                    take(test_reps, random(std::size_t{0}, block_size - 1)));

            CAPTURE(offset, block_size);

            THEN(" the left-aligned offset is 0 ") {
                const uint64_t aligned_offset = chnk_lalign(offset, block_size);
                REQUIRE(aligned_offset == 0);
            }
        }

        WHEN(" offset is larger than block size ") {

            const uint64_t offset = GENERATE_COPY(
                    take(test_reps, random(block_size, block_size * 31)));

            CAPTURE(offset, block_size);

            THEN(" the left-aligned offset is the left boundary of the "
                 "containing block ") {
                const uint64_t aligned_offset = chnk_lalign(offset, block_size);
                const uint64_t exp_offset =
                        static_cast<uint64_t>(offset / block_size) * block_size;
                REQUIRE(aligned_offset == exp_offset);
            }
        }
    }
}
