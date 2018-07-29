#ifndef MYFS_H
#define MYFS_H

#include<bits/stdc++.h>
#include <ctime>
using namespace std;

#define SIZE 256
#define FIL 1
#define DIR 0



struct super_block
{
    int tot_fs_size;
    int max_inodes;
    int max_bks;
    int used_bks;
    bool free_inodes[SHRT_MAX];
    bool free_blks[125000];//max for 32mb
};



struct inode
{
    int file_type;
    int file_size;
    // int tot_files;
    int permissions;
    int blk1_id[8];//direct list
    int blk2_id;//indierect list
    int blk3_id;//double indirect list
    time_t last_md;
    time_t last_ad;
};


struct entry
{
    char file_name[30];
    short inode_number;
};



union block
{
    char data[256];
    int blk_ids[64];
    entry dir_[8];
};

struct file_system
{
    super_block sb;//block 0 virtually for superblock
    inode indb[30];//block 1to5 for inodes list in the file system;
    block Data[125000];//block to hold the data starting fromblock3 offset is used to store
    //into the blocks
    // file_system();
};

struct file_descrpt
{
    int fd_no;
    inode *ind;
    char mode;
    int offset;//no of bytes read
    bool fd_status;
    file_descrpt()
    {
        fd_status=0;
    }

};

extern file_system* f_sys;
 

int ls_myfs();
int create_myfs(int size);
int copy_pc2myfs(char * source,char *dest);
int copy_myfs2pc(char * source,char * dest);
int rm_myfs(char *filename);
int showfile_myfs(char *filename);
int mkdir_myfs(char *dir_name);
int chdir_myfs(char *dir_name);
int rmdir_myfs(char *dir_name);
int open_myfs(char *filename,char mode);
int close_myfs(int fd);
int read_myfs(int fd,int nbytes,char *buff);
int write_myfs(int fd,int nbytes,char *buff);
int eof_myfs(int fd);
int dump_myfs(char *dumpfile);
int restore_myfs(char *dumpfile);
int status_myfs();
int chmod_myfs(char *filename,int mode);
#endif