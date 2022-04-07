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


#include <common/statistics/stats.hpp>

using namespace std;

namespace gkfs::utils {

#ifdef GKFS_ENABLE_PROMETHEUS
static std::string
GetHostName() {
    char hostname[1024];

    if(::gethostname(hostname, sizeof(hostname))) {
        return {};
    }
    return hostname;
}
#endif

void
Stats::setup_Prometheus(const std::string& gateway_ip,
                        const std::string& gateway_port) {
// Prometheus Push model. Gateway
#ifdef GKFS_ENABLE_PROMETHEUS
    const auto labels = Gateway::GetInstanceLabel(GetHostName());
    gateway = std::make_shared<Gateway>(gateway_ip, gateway_port, "GekkoFS",
                                        labels);

    registry = std::make_shared<Registry>();
    family_counter = &BuildCounter()
                              .Name("IOPS")
                              .Help("Number of IOPS")
                              .Register(*registry);

    for(auto e : all_IopsOp) {
        iops_Prometheus[e] = &family_counter->Add(
                {{"operation", IopsOp_s[static_cast<int>(e)]}});
    }

    family_summary = &BuildSummary()
                              .Name("SIZE")
                              .Help("Size of OPs")
                              .Register(*registry);

    for(auto e : all_SizeOp) {
        size_Prometheus[e] = &family_summary->Add(
                {{"operation", SizeOp_s[static_cast<int>(e)]}},
                Summary::Quantiles{});
    }

    gateway->RegisterCollectable(registry);
#endif /// GKFS_ENABLE_PROMETHEUS
}

Stats::Stats(bool output_thread, const std::string& stats_file,
             const std::string& prometheus_gateway) {

    // Init clocks
    start = std::chrono::steady_clock::now();

    // To simplify the control we add an element into the different maps
    // Statistaclly will be negligible... and we get a faster flow

    for(auto e : all_IopsOp) {
        IOPS[e] = 0;
        TimeIops[e].push_back(std::chrono::steady_clock::now());
    }

    for(auto e : all_SizeOp) {
        SIZE[e] = 0;
        TimeSize[e].push_back(pair(std::chrono::steady_clock::now(), 0.0));
    }

#ifdef GKFS_ENABLE_PROMETHEUS
    auto pos_separator = prometheus_gateway.find(":");
    setup_Prometheus(prometheus_gateway.substr(0, pos_separator),
                     prometheus_gateway.substr(pos_separator + 1));
#endif

    output_thread_ = output_thread;

    if(output_thread_) {
        t_output = std::thread([this, stats_file] {
            output(std::chrono::duration(10s), stats_file);
        });
    }
}

Stats::~Stats() {
    // We do not need a mutex for that
    if(output_thread_) {
        running = false;
        t_output.join();
    }
}

void
Stats::add_read(const std::string& path, unsigned long long chunk) {
    chunkRead[pair(path, chunk)]++;
}

void
Stats::add_write(const std::string& path, unsigned long long chunk) {
    chunkWrite[pair(path, chunk)]++;
}


void
Stats::output_map(std::ofstream& output) {
    // Ordering
    map<unsigned int, std::set<pair<std::string, unsigned long long>>>
            orderWrite;

    map<unsigned int, std::set<pair<std::string, unsigned long long>>>
            orderRead;

    for(auto i : chunkRead) {
        orderRead[i.second].insert(i.first);
    }

    for(auto i : chunkWrite) {
        orderWrite[i.second].insert(i.first);
    }

    auto chunkMap =
            [](std::string caption,
               map<unsigned int,
                   std::set<pair<std::string, unsigned long long>>>& order,
               std::ofstream& output) {
                output << caption << std::endl;
                for(auto k : order) {
                    output << k.first << " -- ";
                    for(auto v : k.second) {
                        output << v.first << " // " << v.second << endl;
                    }
                }
            };

    chunkMap("READ CHUNK MAP", orderRead, output);
    chunkMap("WRITE CHUNK MAP", orderWrite, output);
}

void
Stats::add_value_iops(enum IopsOp iop) {
    IOPS[iop]++;
    auto now = std::chrono::steady_clock::now();


    if((now - TimeIops[iop].front()) > std::chrono::duration(10s)) {
        TimeIops[iop].pop_front();
    } else if(TimeIops[iop].size() >= gkfs::config::stats::max_stats)
        TimeIops[iop].pop_front();

    TimeIops[iop].push_back(std::chrono::steady_clock::now());
#ifdef GKFS_ENABLE_PROMETHEUS
    iops_Prometheus[iop]->Increment();
#endif
}

void
Stats::add_value_size(enum SizeOp iop, unsigned long long value) {
    auto now = std::chrono::steady_clock::now();
    SIZE[iop] += value;
    if((now - TimeSize[iop].front().first) > std::chrono::duration(10s)) {
        TimeSize[iop].pop_front();
    } else if(TimeSize[iop].size() >= gkfs::config::stats::max_stats)
        TimeSize[iop].pop_front();

    TimeSize[iop].push_back(pair(std::chrono::steady_clock::now(), value));
#ifdef GKFS_ENABLE_PROMETHEUS
    size_Prometheus[iop]->Observe(value);
#endif
    if(iop == SizeOp::read_size)
        add_value_iops(IopsOp::iops_read);
    else if(iop == SizeOp::write_size)
        add_value_iops(IopsOp::iops_write);
}

/**
 * @brief Get the total mean value of the asked stat
 * This can be provided inmediately without cost
 * @return mean value
 */
double
Stats::get_mean(enum SizeOp sop) {
    auto now = std::chrono::steady_clock::now();
    auto duration =
            std::chrono::duration_cast<std::chrono::seconds>(now - start);
    double value = (double) SIZE[sop] / (double) duration.count();
    return value;
}

double
Stats::get_mean(enum IopsOp iop) {
    auto now = std::chrono::steady_clock::now();
    auto duration =
            std::chrono::duration_cast<std::chrono::seconds>(now - start);
    double value = (double) IOPS[iop] / (double) duration.count();
    return value;
}


std::vector<double>
Stats::get_four_means(enum SizeOp sop) {
    std::vector<double> results = {0, 0, 0, 0};
    auto now = std::chrono::steady_clock::now();
    for(auto e : TimeSize[sop]) {
        auto duration =
                std::chrono::duration_cast<std::chrono::minutes>(now - e.first)
                        .count();
        if(duration > 10)
            break;

        results[3] += e.second;
        if(duration > 5)
            continue;
        results[2] += e.second;
        if(duration > 1)
            continue;
        results[1] += e.second;
    }
    // Mean in MB/s
    results[0] = get_mean(sop) / (1024.0 * 1024.0);
    results[3] /= 10 * 60 * (1024.0 * 1024.0);
    results[2] /= 5 * 60 * (1024.0 * 1024.0);
    results[1] /= 60 * (1024.0 * 1024.0);

    return results;
}


std::vector<double>
Stats::get_four_means(enum IopsOp iop) {
    std::vector<double> results = {0, 0, 0, 0};
    auto now = std::chrono::steady_clock::now();
    for(auto e : TimeIops[iop]) {
        auto duration =
                std::chrono::duration_cast<std::chrono::minutes>(now - e)
                        .count();
        if(duration > 10)
            break;

        results[3]++;
        if(duration > 5)
            continue;
        results[2]++;
        if(duration > 1)
            continue;
        results[1]++;
    }

    results[0] = get_mean(iop);
    results[3] /= 10 * 60;
    results[2] /= 5 * 60;
    results[1] /= 60;

    return results;
}

void
Stats::dump(std::ofstream& of) {
    for(auto e : all_IopsOp) {
        auto tmp = get_four_means(e);

        of << "Stats " << IopsOp_s[static_cast<int>(e)]
           << " IOPS/s (avg, 1 min, 5 min, 10 min) \t\t";
        for(auto mean : tmp) {
            of << std::setprecision(4) << std::setw(9) << mean << " - ";
        }
        of << std::endl;
    }
    for(auto e : all_SizeOp) {
        auto tmp = get_four_means(e);

        of << "Stats " << SizeOp_s[static_cast<int>(e)]
           << " MB/s (avg, 1 min, 5 min, 10 min) \t\t";
        for(auto mean : tmp) {
            of << std::setprecision(4) << std::setw(9) << mean << " - ";
        }
        of << std::endl;
    }
    of << std::endl;
}
void
Stats::output(std::chrono::seconds d, std::string file_output) {
    int times = 0;
    std::ofstream of(file_output, std::ios_base::openmode::_S_trunc);

    while(running) {
        dump(of);
        std::chrono::seconds a = 0s;

        times++;
#ifdef GKFS_CHUNK_STATS
        if(times % 4 == 0)
            output_map(of);
#endif
#ifdef GKFS_ENABLE_PROMETHEUS
        // Prometheus Output
        gateway->Push();
#endif
        while(running and a < d) {
            a += 1s;
            std::this_thread::sleep_for(1s);
        }
    }
}

} // namespace gkfs::utils
