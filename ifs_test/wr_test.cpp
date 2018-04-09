/* Simple Write/Read Test
 *
 * - open a file
 * - write some content
 * - close
 * - open the same file in read mode
 * - read the content
 * - check if the content match
 * - close
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main(int argc, char* argv[]) {

    string p = "/tmp/mountdir/file"s;
    char buffIn[] = "oops.";
    char *buffOut = new char[strlen(buffIn)];
    int fd;

    fd = open(p.c_str(), O_RDONLY);
    if(fd >= 0 ){
        cerr << "ERROR: Succeeded on opening non-existing file" << endl;
        return -1;
    }
    if(errno != ENOENT){
        cerr << "ERROR: wrong error number while opening non-existing file: " << errno << endl;
        return -1;
    }
    
    /* Write the file */

    fd = open(p.c_str(), O_WRONLY | O_CREAT, 0777);
    if(fd < 0){
        cerr << "Error opening file (write)" << endl;
        return -1;
    }
    auto nw = pwrite(fd, buffIn, strlen(buffIn), 10000);
    if(nw != strlen(buffIn)){
        cerr << "Error writing file" << endl;
        return -1;
    }
    
    if(close(fd) != 0){
        cerr << "Error closing file" << endl;
        return -1;
    }


    /* Read the file back */

    fd = open(p.c_str(), O_RDONLY);
    if(fd < 0){
        cerr << "Error opening file (read)" << endl;
        return -1;
    }

    auto nr = read(fd, buffOut, strlen(buffIn));
    if(nr != strlen(buffIn)){
        cerr << "Error reading file" << endl;
        return -1;
    }

    if(strncmp( buffIn, buffOut, strlen(buffIn)) != 0){
        cerr << "File content mismatch" << endl;
        return -1;
    }
    close(fd);
}
