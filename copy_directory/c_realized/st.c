#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
const int N = 1024;

void getFileName(char * dirPath)
{
    DIR *dir=opendir(dirPath);
    if(dir==NULL)
    {
        printf("%s\n",strerror(errno));
        return;
    }
    chdir(dirPath);
    struct dirent *ent;
    while((ent=readdir(dir))!=NULL)
    {
        if(strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0)
        {
            continue;
        }
        struct stat st;
        stat(ent->d_name,&st);
        if(S_ISDIR(st.st_mode))
        {
            getFileName(ent->d_name);
        }
        else
        {
            remove(ent->d_name);
        }
    }
    closedir(dir);
    chdir("..");
}

int main(int argc, char *argv[])
{
    system("cp -rf t1 t2");
    getFileName("t1/t7");
    return 0;
}
