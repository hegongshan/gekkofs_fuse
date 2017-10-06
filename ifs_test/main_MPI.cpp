#include <iostream>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <chrono>
#include <mpi.h>
using namespace std;

using ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;

int main(int argc, char* argv[]) {

    auto filen = strtol(argv[1], NULL, 20);

    MPI_Init(NULL, NULL);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


//    int filen = 3;

    cout << "Hello from rank " << rank << endl;

    auto start_t = get_time::now();

    for (int i = 0; i < filen; ++i) {
        string p = "/tmp/mountdir/file" + to_string(rank) + "_" + to_string(i);
        creat(p.c_str(), 0666);
        if (i % 25000 == 0)
            cout << i << " files processed." << endl;
//        cout << p << endl;
    }

    auto end_t = get_time::now();
    auto diff = end_t - start_t;

    auto diff_count = chrono::duration_cast<ns>(diff).count();

    cout << diff_count << "ns\t" << (diff_count) / 1000000. << "ms" << endl;
    cout << filen / ((diff_count) / 1000000000.) << " files per second" << endl;


    MPI_Finalize();

//    cout << "done" << endl;
    return 0;

}