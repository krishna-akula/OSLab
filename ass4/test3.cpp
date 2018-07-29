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
    restore_myfs("mydump_3.backup");
    ls_myfs();
    string s="";
    int fd=open_myfs("myfile.txt",'r');    
    char buff[1024];
    while(!eof_myfs(fd))
    {
        bzero(buff,1024);
        read_myfs(fd,100,buff);
        s+=buff;
    }
    close_myfs(fd);
    stringstream iss(s);
    int k;
    vector<int>v;
    for(int i=0;i<100;i++)
    {
        iss>>k;
        v.push_back(k);
    }
    sort(v.begin(),v.end());
    int new_fd=open_myfs("sorted.txt",'w');
    for(int i=0;i<100;i++)
    {
        sprintf(buff,"%d ",v[i]);
        int k=strlen(buff);
        write_myfs(new_fd,k,buff);
    }
    close(new_fd);
        cout<<"-----------------------------------------------\n";        
    
    showfile_myfs("myfile.txt");
        cout<<"-----------------------------------------------\n";        
    
    showfile_myfs("sorted.txt");

}