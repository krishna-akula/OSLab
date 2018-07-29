#include <bits/stdc++.h>
#include "myfs.h"
#include <fstream>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

using namespace std;


int main()
{

    int shm_id=shmget(IPC_PRIVATE,sizeof(file_system),0777|IPC_CREAT);
    f_sys=(file_system*)shmat(shm_id,0,0);
    create_myfs(10000000);
    int fd=open_myfs("myfile.txt",'w');
    cout<<fd<<endl;
    char num[100];
    int size=0;
    for(int i=0;i<100;i++)
    {
        // int k=rand()%100;
        sprintf(num,"%d ",rand()%100);
        int len=strlen(num);
        size+=len;
        int k=write_myfs(fd,len,num);
    }
    close_myfs(fd);
    int n;
    ls_myfs();
    char filename[100];
    cout<<"n: "<<size<<endl;
    cin>>n;
    for(int i=0;i<n;i++)
    {
        char buff[1024];
        bzero(buff,1024);
        bzero(filename,100);
        sprintf(filename,"mytext%d.txt",i);
        fd=open_myfs("myfile.txt",'r');
        int new_fd=open_myfs(filename,'w');
        while(eof_myfs(fd)==0)
        {
                bzero(buff,1024);
               int k=read_myfs(fd,100,buff);
                write_myfs(new_fd,k,buff);
                       
        }
        close_myfs(fd);
        close_myfs(new_fd);
    }
    ls_myfs();
    // status_myfs();
    dump_myfs("mydump_3.backup");

}