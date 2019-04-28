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
    struct stat st1;
    struct stat st2;
    printf("the source_dirname: %s\n",argv[1]);
    printf("the des_dirname is: %s\n",argv[2]);
    stat(argv[1],&st1);
    stat(argv[2],&st2);
    if(!S_ISDIR(st1.st_mode))
    {
        perror(" don't exit !");
        exit(EXIT_FAILURE);
    }
     if(!S_ISDIR(st2.st_mode))
    {
        perror(" don't exit !");
        exit(EXIT_FAILURE);
    }
    DIR * dir1 = opendir(argv[1]);
    if(dir1==NULL)
    {
        printf("%s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    DIR * dir2 = opendir(argv[2]);
    if(dir2==NULL)
    {
        printf("%s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    chdir(argv[1]);
    chdir(argv[2]);
    struct dirent *ent1;
    struct dirent *ent2;
    if(((ent1 = readdir(dir1)) != NULL)&&((ent1 = readdir(dir1)) != NULL))
        system("cp -rf 'ent1->d_name' 'ent2->d_name'");
    closedir(dir1);
    closedir(dir2);

    int i,j;
    int len_s = strlen(argv[1]);
    int len_d = strlen(argv[2]);
    int temp;
     for( i = len_s-1;i >= 0 ;i--){
        if(argv[1][i] == '/'){
            temp = i;
            break;
        }
    }

    for(i = len_d,j = temp;j<len_s;i++,j++){
        argv[2][i] =argv[1][j];
    }
    DIR *dir=opendir(argv[2]);
    if(dir==NULL)
    {
        printf("%s\n",strerror(errno));
        return;
    }
    chdir(argv[2]);
    struct dirent *ent;

    if((ent = readdir(dir)) != NULL)
    {
         closedir(dir);
         getFileName("ent->d_name");
    }
    return 0;
}
