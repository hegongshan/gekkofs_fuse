#include <iostream>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>

using namespace std;

int main() {

//    std::fstream fs;
//    fs.open("testcpp_open.txt", std::fstream::in | std::fstream::out | std::fstream::app);
//
//    fs << "lorem ipsum";
//    fs.close();

//    ofstream ofs("/tmp/test_ofstream");
//
//    ofs << "lorem upsum";
//    ofs.close();
//
//    auto buf = "BUFFERINO2";
//    auto fd2 = open("/tmp/test_open.txt", O_WRONLY|O_CREAT|O_TRUNC,0666);
//    write(fd2, buf, 9);
//    cout << close(fd2) << endl;
//    auto fd3 = fopen("test_fopen", "w");
//    fputs(buf, fd3);
//    fclose(fd3);


//    for (int i = 0; i < 200; ++i) {
//        printf("%d\n", i);
//        auto fd = open("test2.txt", O_WRONLY);
//        write(fd, buf, 9);
//        close(fd);
//        sleep(1);
//    }

    auto buf = "BUFFERINO2";


    auto pid = fork();

    if (pid == 0) {
        auto fd2 = open("/tmp/child_open.txt", O_WRONLY|O_CREAT|O_TRUNC,0666);
        write(fd2, buf, 9);
        cout << close(fd2) << endl;
    } else if (pid > 0) {
        auto fd3 = open("/tmp/parent_open.txt", O_WRONLY|O_CREAT|O_TRUNC,0666);
        write(fd3, buf, 9);
        cout << close(fd3) << endl;
    } else {
        printf("fork failed");
        return 1;
    }





    cout << "done" << endl;
    return 0;

}