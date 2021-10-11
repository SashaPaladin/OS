#include "unistd.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

int main()
{
    std::cout << "Enter file name:" << std::endl;
    std::string fileName;
    std::cin >> fileName;
    int fd[2];
    pipe(fd);
    int id = fork();
    if (id == -1)
        {
        perror("fork error");
        return -1;
        }
    else if (id == 0)
        {
        //printf("[%d] It's child\n", getpid());
        int res;
        int n = 1;
        std::ofstream out(fileName);
        while (read(fd[0], &n, sizeof(int)) && n != 0) {
            int *p = new int[n];
            read(fd[0], p, sizeof(int[n]));
            res = p[0];
            for (int i = 1; i < n; i++){
                if (p[i] == 0) exit(-1);
                res = res / p[i];
            }
            std::cout << "res: " << res << std::endl;
            out << res << std::endl;
            delete [] p;
        }
        out.close();
        close(fd[0]);
        close(fd[1]);
        }
    else
    {
        //printf("[%d] It's parent. Child id: %d\n", getpid(), id);
        int num;
        std::string line;
        std::vector<int> vec;
        std::cout << "Enter numbers:" << std::endl;
        std::cin.ignore();
        while(std::cin.good() && getline(std::cin, line)){
            std::stringstream inp(line);
            while (inp >> num) {
                vec.push_back(num);
            }
            int n = vec.size();
            int *p = new int[n];
            for (int i = 0; i < vec.size(); i++){
                p[i] = vec[i];
            }
            write(fd[1], &n, sizeof(int));
            write(fd[1], p, sizeof(int[n]));
            std::vector<int>().swap(vec);
            delete [] p;
        }
        close(fd[0]);
        close(fd[1]);
    }
    return 0;
}