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
    char filename[30];
    for(int i=0;i<12;i++)
    {
        sprintf(filename,"test%d.txt",i);
        copy_pc2myfs("text2.txt",filename);
    }
    ls_myfs();
    cout<<"-----------------------------------------------\n";        
    cout<<"enter file name to be deleted : ";
    cin>>filename;
    rm_myfs(filename);
    mkdir_myfs("mycode");
    chdir_myfs("mycode");
    ls_myfs();
    cout<<"-----------------------------------------------\n";         
    chdir_myfs("..");
    ls_myfs();
    cout<<"-----------------------------------------------\n";        
    // showfile_myfs(filename);


}