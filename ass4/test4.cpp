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
    pid_t cid;
    if(fork()==0)
    {
        f_sys=(file_system*)shmat(shm_id,0,0);
        
       mkdir_myfs("mydocs");
       chdir_myfs("mydocs");
       int fd=open_myfs("mytext.txt",'w');
       char c[2]="A";
       for(int i=0;i<26;i++)
       {
           write_myfs(fd,1,c);
            c[0]+=1;
       }
       close_myfs(fd);
       exit(0);

    }
    else
    {
        f_sys=(file_system*)shmat(shm_id,0,0);    
        mkdir_myfs("mycode");
        ls_myfs();
        cout<<"-----------------------------------------------\n";        
        chdir_myfs("mycode");
        copy_pc2myfs("text2.txt","mycode.txt");
        sleep(1);
        ls_myfs();
        cout<<"-----------------------------------------------\n";
        chdir_myfs("..");
        ls_myfs();
        cout<<"-----------------------------------------------\n";        
        chdir_myfs("mydocs");
        ls_myfs();
        cout<<"-----------------------------------------------\n";
        showfile_myfs("mytext.txt");        

    }


}