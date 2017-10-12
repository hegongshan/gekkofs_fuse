#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <cstring>

using namespace std;

using ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;

int main(int argc, char* argv[]) {

    auto filen = atoi(argv[1]);

//    cout << mkdir("/tmp/mountdir/bla", 0775) << endl;
//    auto buf = "BUFFERINO2";
//    struct stat attr;
//    cout << creat("/tmp/mountdir/creat.txt", 0666) << endl;
//    cout << creat("/tmp/mountdir/#test-dir.0/mdtest_tree.0/file.mdtest.0000000.0000000005", 0666) << endl;
//    cout << stat("/tmp/mountdir/creat.txt", &attr) << endl;
//    cout << unlink("/tmp/mountdir/creat.txt") << endl;

//    char buf[] = "test123\n";
//    string p = "/tmp/mountdir/file"s;
//    auto fd = open(p.c_str(), O_CREAT|O_WRONLY, 0777);
//    auto nw = write(fd, &buf, strlen(buf));
//    close(fd);
//
//    fd = open(p.c_str(), O_RDONLY, 0777);
//    char buf_read[9] = {0};
//    auto rs = read(fd, &buf_read, strlen(buf));
//    buf_read[8] = '\0';
//    printf("buffer read: %s\n", buf_read);
//    close(fd);

//    auto fd2 = open("/tmp/rootdir/data/chunks/file/data2", O_RDONLY, 0777);
//    char buf_read2[9] = {0};
//    auto rs2 = read(fd2, &buf_read2, 8);
//    close(fd2);
//    buf_read2[8] = '\0';
//
//    string bla = buf_read2;
//    cout << bla << endl;



    auto start_t = get_time::now();

    for (int i = 0; i < filen; ++i) {
        string p = "/tmp/mountdir/file" + to_string(i);
        creat(p.c_str(), 0666);
        if (i % 25000 == 0)
            cout << i << " files processed." << endl;
    }

    auto end_t = get_time::now();
    auto diff = end_t - start_t;

    auto diff_count = chrono::duration_cast<ns>(diff).count();

    cout << diff_count << "ns\t" << (diff_count) / 1000000. << "ms" << endl;
    cout << filen / ((diff_count) / 1000000000.) << " files per second" << endl;

    return 0;

}