#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>

#include "shrmem.h"

using namespace std;

int main(int argc, char **argv) {
  int map_fd = shm_open(BackingFile, O_RDWR, AccessPerms);
  if (map_fd < 0) {
	perror("SHM_OPEN");
	exit(EXIT_FAILURE);
  }
  struct stat statbuf;
  fstat(map_fd, &statbuf);
  const size_t map_size = statbuf.st_size;
  caddr_t memptr = static_cast<char*>(mmap(
	  NULL,
	  map_size,
	  PROT_READ | PROT_WRITE,
	  MAP_SHARED,
	  map_fd,
	  0));
  if (memptr == MAP_FAILED) {
	perror("MMAP");
	exit(EXIT_FAILURE);
  }
  sem_t *semptr = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);
  if (semptr == SEM_FAILED) {
	perror("SEM_OPEN");
	exit(EXIT_FAILURE);
  }
  if (sem_wait(semptr) != 0) {
	perror("SEM_WAIT");
	exit(EXIT_FAILURE);
  }
  char *out = (char *)calloc(1, sizeof(char));
  size_t m_size = 0;
  int flag = 0;
  string first;
  string second;

  FILE *filename = fopen(argv[1], "w");
  if (filename == NULL) cout << "fopen error";

  for (int i = 0; i + 1 < map_size; ++i) {// преобразование
  	if (flag == 0) {
  		first.push_back(memptr[i]);
  	}
  	else if (flag == 1) {
  		second.push_back(memptr[i]);
  	}
  	if (memptr[i] == ' ' && flag == 0) {
  		flag = 1;
  	}
  	else if ((memptr[i] == ' ' || memptr[i] == '\n') && flag == 1) {
  		if (atoi(second.c_str()) == 0) { 
  			perror("division by zero error\n");
  			break;
  		}
  		first = to_string(atoi(first.c_str())/atoi(second.c_str()));
  		second = "";
  		if (memptr[i] == '\n') {
  			fprintf(filename, "%s\n", first.c_str());
  			flag = 0;
  			//cout << first << endl;
  			first = "";
  			second = "";
  		}
  	}
	//out[m_size] = memptr[i];
	//out = (char *)realloc(out, (++m_size + 1) * sizeof(char));
  }
  fclose(filename);

  out[m_size++] = '\0';
  ftruncate(map_fd, (off_t)m_size);
  memset(memptr, '\0', m_size);
  sprintf(memptr, "%s", out);
  free(out);
  close(map_fd);
  usleep(00150000);
  sem_post(semptr);
  sem_close(semptr);
  return EXIT_SUCCESS;
}