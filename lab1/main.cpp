#include "unistd.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

/* */

int main()
{
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
        printf("[%d] It's child\n", getpid());
        fflush(stdout);
        std::vector<int> vec;
        int res;
        std::string fileName;
        read(fd[0], &vec, sizeof(std::vector<int>));
        read(fd[0], &fileName, sizeof(std::string));
        for (int i = 0; i < vec.size(); ++i) {
            if (i == 0) res = vec[i];
            else if (vec[i] == 0) exit(-1);
            res /= vec[i];
        }
        std::ofstream out(fileName);
        out << res;
        out.close();
        write(fd[1], &res, sizeof(int));
        close(fd[0]);
        close(fd[1]);
    }
    else
    {
        printf("[%d] It's parent. Child id: %d\n", getpid(), id);
        fflush(stdout);
        int num;
        std::string fileName;
        scanf("%s", &fileName);
        std::vector<int> vec;
        while (std::cin.peek() != '\n') {
            vec.push_back(num);
        }
        write(fd[1], &vec, sizeof(std::vector<int>));
        write(fd[1], &fileName, sizeof(std::string));

        int res;
        read(fd[0], &res, sizeof(int));
        printf("[%d] Result from child: %d\n", getpid(), res);
        fflush(stdout);
        close(fd[0]);
        close(fd[1]);
    }
    return 0;
}