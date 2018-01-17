//
// Created by evie on 1/16/18.
//

#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    auto path = "/tmp/mountdir/testdir";
    auto err = mkdir(path, S_IRUSR | S_IROTH);
    cout << err << endl;
    auto perm = S_IWUSR;
    cout << "perm: " << perm << endl;
    err = access(path, perm);
    cout << err << endl;
    err = rmdir(path);
    cout << err << endl;
    return 0;
}