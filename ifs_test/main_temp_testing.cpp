#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main(int argc, char* argv[]) {

    char buf[] = "test123\n";
    string p = "/tmp/mountdir/file"s;
    auto fd = open(p.c_str(), O_CREAT | O_WRONLY, 0777);
    auto nw = write(fd, &buf, strlen(buf));
    close(fd);

    char buf_a[] = "456esta\n";
    auto fd_a = open(p.c_str(), O_WRONLY | O_APPEND, 0777);
    auto nw_a = write(fd, &buf_a, strlen(buf));
    close(fd);

    fd = open(p.c_str(), O_RDONLY, 0777);
    char buf_read[17] = {0};
    auto rs = read(fd, &buf_read, strlen(buf) * 2);
    buf_read[17] = '\0';
    printf("buffer read: %s\n", buf_read);
    close(fd);

    //    auto fd2 = open("/tmp/rootdir/data/chunks/file/data2", O_RDONLY, 0777);
//    char buf_read2[9] = {0};
//    auto rs2 = read(fd2, &buf_read2, 8);
//    close(fd2);
//    buf_read2[8] = '\0';
//
//    string bla = buf_read2;
//    cout << bla << endl;

    return 0;

}