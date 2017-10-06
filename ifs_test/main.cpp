#include <iostream>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <chrono>
using namespace std;

using ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;

int main(int argc, char* argv[]) {

    auto filen = strtol(argv[1], NULL, 20);

//    cout << mkdir("/tmp/mountdir/bla", 0775) << endl;
//    auto buf = "BUFFERINO2";
//    struct stat attr;
//    cout << creat("/tmp/mountdir/creat.txt", 0666) << endl;
//    cout << creat("/tmp/mountdir/#test-dir.0/mdtest_tree.0/file.mdtest.0000000.0000000005", 0666) << endl;
//    cout << stat("/tmp/mountdir/creat.txt", &attr) << endl;
//    cout << unlink("/tmp/mountdir/creat.txt") << endl;


//    auto pid = fork();

//    if (pid == 0) {
//        auto fd2 = open("/tmp/child_open.txt", O_WRONLY|O_CREAT|O_TRUNC,0666);
//        write(fd2, buf, 9);
//        cout << close(fd2) << endl;
//    } else if (pid > 0) {
//        auto fd3 = open("/tmp/parent_open.txt", O_WRONLY|O_CREAT|O_TRUNC,0666);
//        write(fd3, buf, 9);
//        cout << close(fd3) << endl;
//    } else {
//        printf("fork failed");
//        return 1;
//    }

//    int filen = 3;

    auto start_t = get_time::now();

    for (int i = 0; i < filen; ++i) {
        string p = "/tmp/mountdir/file2" + to_string(i);
        creat(p.c_str(), 0666);
        if (i % 25000 == 0)
            cout << i << " files processed.2" << endl;
//        cout << p << endl;
    }

    auto end_t = get_time::now();
    auto diff = end_t - start_t;

    auto diff_count = chrono::duration_cast<ns>(diff).count();

    cout << diff_count << "ns\t" << (diff_count) / 1000000. << "ms" << endl;
    cout << filen / ((diff_count) / 1000000000.) << " files per second" << endl;




//    cout << "done" << endl;
    return 0;

}