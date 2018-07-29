#include "myfs.h"
#include <bits/stdc++.h>
#include <stdlib.h>
#include <fstream>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <ctime>
#include <fcntl.h>
using namespace std;

#define lk(s) semop(s,&pop,1);
#define  re(s) semop(s,&vop,1);



/* creating semaphores */
int sm1,sm2,sm3;
sembuf pop,vop;

//file system variable name
file_system *f_sys;
int rt_ind=0; //root inode
// pointing to the current directory
int curr_ind=0;
inode *curr_dir;

// extern file_system f_sys;
/* getting the time */

string get_tme(time_t &t)
{
    string ti;
    ti = ctime(&t);
    ti[ti.length()-1]='\0';
    return ti;
}

void init_sem()
{
    sm1=semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    sm2=semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    sm3=semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    semctl(sm1, 0, SETVAL, 1);
	semctl(sm2, 0, SETVAL, 1);
	semctl(sm3, 0, SETVAL, 1);
    pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
    
}


void init_file_system()
{
    init_sem();
    int perblock_inodes = 256/sizeof(inode);
    f_sys->sb.max_inodes=10*perblock_inodes;
    f_sys->sb.used_bks=0;
    memset(f_sys->sb.free_inodes,0,sizeof(f_sys->sb.free_inodes));   
    memset(f_sys->sb.free_blks,0,sizeof(f_sys->sb.free_blks));   
}

int get_free_inode()
{
    lk(sm1);
    for(int i=0;i<f_sys->sb.max_inodes;i++)
    {
        if(f_sys->sb.free_inodes[i]==0)
        {
            f_sys->sb.free_inodes[i]=1;
            re(sm1);
            return i;
        }
           
    }
    re(sm1);
    return -1;
}

int get_free_block()
{
    lk(sm2);
    for(int i=0;i<f_sys->sb.max_bks;i++)
    {
        if(f_sys->sb.free_blks[i]==0)
        {
            f_sys->sb.free_blks[i]=1;
            f_sys->sb.used_bks++;
            re(sm2);
            return i;
        }  
    }
    re(sm2);
    return -1;
}

void set_ids(int a[],int size)
{
    for(int i=0;i<size;i++)
        a[i]=-1;
}

void  create_inode(int ind,int type,int size)
{
    f_sys->sb.free_inodes[ind]=1;
    f_sys->indb[ind].file_type=type;
    f_sys->indb[ind].file_size=size;
    time_t temp=time(0);
    f_sys->indb[ind].last_md=time(0);
    // f_sys->indb[ind].tot_files=0;
    f_sys->indb[ind].permissions=731;
    set_ids(f_sys->indb[ind].blk1_id,8);
    f_sys->indb[ind].blk2_id=-1;
    f_sys->indb[ind].blk3_id=-1;
}

void set_ids2(block &temp,int size)
{
    short s=-1;
    for(int i=0;i<size;i++)
    {
        temp.dir_[i].inode_number=s;
    }

}

int create_myfs(int size)
{
    init_file_system();
    int tot_blocks=size/SIZE;
    f_sys->sb.max_bks=tot_blocks-11;
    f_sys->sb.tot_fs_size=size;
    if(tot_blocks-2<0)
        return -1;
    //creating the blocks
    // creating free_blks in superblock
    f_sys->sb.used_bks=0;
    //creating the root inode o is root inode
    f_sys->sb.free_inodes[0]=1;
    create_inode(rt_ind,DIR,256);
    curr_dir = &(f_sys->indb[rt_ind]);
    //assigning the first 0th block (offset) to the root directory
    f_sys->indb[0].blk1_id[0]=get_free_block();
    set_ids2(f_sys->Data[f_sys->indb[0].blk1_id[0]],8);
    return 1;
}




int search_free_entry(block &blk)
{
    // lk(sm3);
    for(int i=0;i<8;i++)
    {
        if(blk.dir_[i].inode_number==-1)
        {
            // re(sm3);
            return i;
        }
        
    }
    // re(sm3);
    return -1;
}


// adding the entry into the current directory
int add_entry(char *name,short ind_index)
{
    int free_index,in_free_index,db_free_index;
    //searching in blk_1 id
    int i=0,j=0;
    for(i=0;i<8;i++)
    {
        if(curr_dir->blk1_id[i]!=-1)
        {
            free_index=search_free_entry(f_sys->Data[curr_dir->blk1_id[i]]);
            if(free_index!=-1)
                break;
        }
        else 
        {
            curr_dir->blk1_id[i]=get_free_block();
            set_ids2(f_sys->Data[f_sys->indb[curr_ind].blk1_id[i]],8);
            if(curr_dir->blk1_id[i]==-1)
                return -1;
            free_index=search_free_entry(f_sys->Data[curr_dir->blk1_id[i]]);
            break;
        }
    }
    if(free_index!=-1)
    {
        f_sys->Data[curr_dir->blk1_id[i]].dir_[free_index].inode_number=ind_index;
        strcpy(f_sys->Data[curr_dir->blk1_id[i]].dir_[free_index].file_name,name);
        return 1;
    }
    
    //searching in blk2_d
    if(curr_dir->blk2_id==-1)
    {
        curr_dir->blk2_id=get_free_block();
        set_ids(f_sys->Data[curr_dir->blk2_id].blk_ids,64);
    }
    for( i=0;i<64;i++)
    {
        if(f_sys->Data[curr_dir->blk2_id].blk_ids[i]!=-1)
        {
            in_free_index=i;
            free_index=search_free_entry(f_sys->Data[f_sys->Data[curr_dir->blk2_id].blk_ids[i]]);   
            if(free_index!=-1)
                break;          
        }
        else
        {
            f_sys->Data[curr_dir->blk2_id].blk_ids[i]=get_free_block();
            set_ids2(f_sys->Data[f_sys->Data[curr_dir->blk2_id].blk_ids[i]],8);
            free_index=search_free_entry(f_sys->Data[f_sys->Data[curr_dir->blk2_id].blk_ids[i]]);   
            break;
        }
    }

    if(free_index!=-1)
    {
        f_sys->Data[f_sys->Data[curr_dir->blk2_id].blk_ids[i]].dir_[free_index].inode_number=ind_index;
        strcpy(f_sys->Data[f_sys->Data[curr_dir->blk2_id].blk_ids[i]].dir_[free_index].file_name,name);
        return 1;
    }
    // // searching in blk3_id

    if(curr_dir->blk3_id==-1)
    {
        curr_dir->blk3_id=get_free_block();
        set_ids(f_sys->Data[curr_dir->blk3_id].blk_ids,64);
    }
    for( i=0;i<64;i++)
    {
         if(f_sys->Data[curr_dir->blk3_id].blk_ids[i]==-1)
        {
            f_sys->Data[curr_dir->blk3_id].blk_ids[i]=get_free_block();
            // set_ids(f_sys->Data[f_sys->Data[curr_dir->blk3_id].blk_ids[i]],64);
        }
        for(j=0;j<64;j++)
        {
            if(f_sys->Data[ f_sys->Data[curr_dir->blk3_id].blk_ids[i]].blk_ids[j]==-1)
            {
                f_sys->Data[ f_sys->Data[curr_dir->blk3_id].blk_ids[i]].blk_ids[j]=get_free_block();
                 set_ids2(f_sys->Data[f_sys->Data[ f_sys->Data[curr_dir->blk3_id].blk_ids[i]].blk_ids[j]],8);
            }
            free_index=search_free_entry(f_sys->Data[f_sys->Data[ f_sys->Data[curr_dir->blk3_id].blk_ids[i]].blk_ids[j]]);
            if(free_index!=-1)
                break;
        }
        if(free_index!=-1)
            break;
        
    }

    if(free_index!=-1)
    {
        f_sys->Data[f_sys->Data[ f_sys->Data[curr_dir->blk3_id].blk_ids[i]].blk_ids[j]].dir_[free_index].inode_number=ind_index;
        strcpy( f_sys->Data[f_sys->Data[ f_sys->Data[curr_dir->blk3_id].blk_ids[i]].blk_ids[j]].dir_[free_index].file_name,name);
        return 1;
    }
    return -1;
}

int copy_pc2myfs(char *source,char *dest)
{
    ifstream src;
    char buf[257];
    src.open(source,ios::in|ios::binary);
    int fsize,b,g;
    b = src.tellg();         
    src.seekg(0, ios::end);
    g =  src.tellg() ;
    fsize=g-b;
    src.close();

    int fd=open(source,O_RDONLY, 0);
    int required_size=fsize;
    int tot_required=fsize%256==0?fsize/256:(fsize/256+1);
    // cout<<"tot_required: "<<tot_required<<endl;
    if(f_sys->sb.max_bks-f_sys->sb.used_bks<=tot_required)
        return -1;
    int ind_index=get_free_inode();
    if(ind_index==-1)
        return -1;
    // entering the entry into the block
    // cout<<266<<endl;
    add_entry(dest,(short)ind_index);
    create_inode(ind_index,FIL,fsize);
    // cout<<267<<endl;
    inode * cur_id=&(f_sys->indb[ind_index]);
    for(int i=0;i<min(8,tot_required);i++)
    {
        bzero(buf,257);
        read(fd,buf,min(required_size,256));
        required_size-=min(required_size,256);
        cur_id->blk1_id[i]=get_free_block();
        memcpy(f_sys->Data[cur_id->blk1_id[i]].data,buf,256);
    }
    if(tot_required>8)
    {
        cur_id->blk2_id=get_free_block();
        set_ids(f_sys->Data[cur_id->blk2_id].blk_ids,64);
        for(int i=0;i<min(64,(tot_required-8));i++)
        {
            bzero(buf,257);
            (f_sys->Data[cur_id->blk2_id]).blk_ids[i]=get_free_block();
            read(fd,buf,min(required_size,256));
            required_size-=min(required_size,256);
            memcpy(f_sys->Data[(f_sys->Data[cur_id->blk2_id]).blk_ids[i]].data,buf,256);
        }
    }

    if(tot_required>72)
    {
        cur_id->blk3_id=get_free_block();
        set_ids(f_sys->Data[cur_id->blk3_id].blk_ids,64);        
        int db_req=(tot_required-72)/(64);
        if((tot_required-72)%(64)!=0)
        {
            db_req++;
        }
        int curr_req=tot_required-72;
        for(int i=0;i<db_req;i++)
        {
            f_sys->Data[cur_id->blk3_id].blk_ids[i]=get_free_block();
            set_ids(f_sys->Data[f_sys->Data[cur_id->blk3_id].blk_ids[i]].blk_ids,64);
            for(int j=0;j<min(64,curr_req);j++)
            {
                f_sys->Data[f_sys->Data[cur_id->blk3_id].blk_ids[i]].blk_ids[j]=get_free_block();
                bzero(buf,257);
                read(fd,buf,min(required_size,256));
                required_size-=min(required_size,256);
                memcpy(f_sys->Data[f_sys->Data[f_sys->Data[cur_id->blk3_id].blk_ids[i]].blk_ids[j]].data,buf,256);
            }
            curr_req=curr_req-64;
            
        }
    }
    close(fd);
    return 1;
}


/* printing the info of the file */
void print_char(int digit)
{
    cout<<((digit&4)==0?'-':'r');
    cout<<((digit&2)==0?'-':'w');
    cout<<((digit&1)==0?'-':'x');
}

void print_permissions(int perm)
{
    int d1,d2,d3;
    d1=perm%10;
    perm/=10;
    d2=perm%10;
    perm/=10;
    d3=perm%10;
    print_char(d3);
    print_char(d2);
    print_char(d1);

}

void print_info(inode &temp,char *file_name)
{
    if(temp.file_type==DIR)
    {
        cout<<'d';
    }
    else
    {
        cout<<'-';
    }
    print_permissions(temp.permissions);
    cout<<'\t';
    cout<<temp.file_size<<"\t";
    cout<<file_name<<"\t";
    cout<<get_tme(temp.last_md)<<"\n";

}

void print_list(block &blk)
{
    short s=-1;
    for(int i=0;i<8;i++)
    {
        if((blk.dir_[i]).inode_number!=s)
        {
            print_info(f_sys->indb[(blk.dir_[i]).inode_number],(blk.dir_[i]).file_name);
        }

    }
}

int ls_myfs()
{
    int i=0,j=0;
    for(i=0;i<8;i++)
    {
        if(curr_dir->blk1_id[i]!=-1)
        {
            print_list(f_sys->Data[curr_dir->blk1_id[i]]);
        }
    }
    
    //searching in blk2_d
    if(curr_dir->blk2_id!=-1)
    {
        for( i=0;i<64;i++)
        {
            if(f_sys->Data[curr_dir->blk2_id].blk_ids[i]!=-1)
            {
                print_list(f_sys->Data[f_sys->Data[curr_dir->blk2_id].blk_ids[i]]);
            }
            
        }
    }
    // searching in blk3_id
    if(curr_dir->blk3_id!=-1)
    {
       for( i=0;i<64;i++)
        {
            if(f_sys->Data[curr_dir->blk3_id].blk_ids[i]!=-1)
            {
                for(j=0;j<64;j++)
                {
                    if(f_sys->Data[ f_sys->Data[curr_dir->blk3_id].blk_ids[i]].blk_ids[j]!=-1)
                    {
                       print_list(f_sys->Data[f_sys->Data[f_sys->Data[curr_dir->blk3_id].blk_ids[i]].blk_ids[j]]); 
                    }
                    
                }

            }
        }
            
    }
    
    return 1;
}

void free_block(int index)
{
    f_sys->sb.free_blks[index]=0;
    f_sys->sb.used_bks--;
}


/* removing the file */
int remove_file(block &blk,char *filename)
{
    short s=-1;
    for(int i=0;i<8;i++)
    {
        if((blk.dir_[i]).inode_number!=-1)
        {
           if(strcmp((blk.dir_[i]).file_name,filename)==0)
           {
               int index=(int)(blk.dir_[i]).inode_number;
               (blk.dir_[i]).inode_number=s;
               return index;
           }
        }
    }
    return -1;
}


void de_alloc_blocks(int ind_index)
{
    // cout<<ind_index<<" 452"<<endl;
    inode *temp=&(f_sys->indb[ind_index]);
    // cout<<"filesizze"<<temp->file_size<<" "<<temp->file_type<<endl;
    // cout<<temp->blk3_id<<endl;
    int i=0,j=0;
    for(i=0;i<8;i++)
    {
        if(temp->blk1_id[i]!=-1)
        {
          free_block(temp->blk1_id[i]);
        }
        
    }
    
    //deallocating in blk2_d
    if(temp->blk2_id!=-1)
    {
        for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk2_id].blk_ids[i]!=-1)
            {
                free_block(f_sys->Data[temp->blk2_id].blk_ids[i]);
            } 
           
        }
        free_block(temp->blk2_id);
    }
    // deallocating in blk3_id
    if(temp->blk3_id!=-1)
    {
       for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk3_id].blk_ids[i]!=-1)
            {
                for(j=0;j<64;j++)
                {
                    if(f_sys->Data[ f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]!=-1)
                    {
                       free_block(f_sys->Data[ f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]);
                    }
           
                    
                }
                free_block(f_sys->Data[temp->blk3_id].blk_ids[i]);
            }
        }
        free_block(temp->blk3_id);      
    }

}


int rm_myfs_ind(char *filename,int par_ind)
{
    inode *temp=&(f_sys->indb[par_ind]);
    int i=0,j=0,status;
    for(i=0;i<8;i++)
    {
        if(temp->blk1_id[i]!=-1)
        {
           status=remove_file(f_sys->Data[temp->blk1_id[i]],filename);
           if(status!=-1)
            {
                de_alloc_blocks(status);
                f_sys->sb.free_inodes[status]=0;
                return 1;
            }
        }
    }
    
    //searching in blk2_d
    if(temp->blk2_id!=-1)
    {
        for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk2_id].blk_ids[i]!=-1)
            {
                status=remove_file(f_sys->Data[f_sys->Data[temp->blk2_id].blk_ids[i]],filename);
                if(status!=-1)
                {
                    de_alloc_blocks(status);
                    f_sys->sb.free_inodes[status]=0;
                    return 1;
                }
            }
            
        }
    }
    // searching in blk3_id
    if(temp->blk3_id!=-1)
    {
       for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk3_id].blk_ids[i]!=-1)
            {
                for(j=0;j<64;j++)
                {
                    if(f_sys->Data[ f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]!=-1)
                    {
                       status=remove_file(f_sys->Data[f_sys->Data[f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]],filename);
                        if(status!=-1)
                        {
                            de_alloc_blocks(status);
                            f_sys->sb.free_inodes[status]=0;
                            return 1;
                        } 
                    }
                    
                }

            }
        }
            
    }
    return -1;
}

int rm_myfs(char *filename)
{
    return rm_myfs_ind(filename,curr_ind);
}


/* showing the file */

int search_entry(block &blk,char *filename)
{
    for(int i=0;i<8;i++)
    {
        if((blk.dir_[i]).inode_number!=-1)
        {
           if(strcmp((blk.dir_[i]).file_name,filename)==0)
           {
               int index=(int)(blk.dir_[i]).inode_number;
               return index;
           }
        }
    }
    return -1;
}

int search_file(char* filename,int par_ind)
{

    int i=0,j=0,status;
    inode *par_dir = &(f_sys->indb[par_ind]);
    for(i=0;i<8;i++)
    {
        if(par_dir->blk1_id[i]!=-1)
        {
           status=search_entry(f_sys->Data[par_dir->blk1_id[i]],filename);
           if(status!=-1)
            {
                return status;
            }
        }
    }
    
    //searching in blk2_d
    if(par_dir->blk2_id!=-1)
    {
        for( i=0;i<64;i++)
        {
            if(f_sys->Data[par_dir->blk2_id].blk_ids[i]!=-1)
            {
                status=search_entry(f_sys->Data[f_sys->Data[par_dir->blk2_id].blk_ids[i]],filename);
                if(status!=-1)
                {
                    return status;
                }
            }
            
        }
    }
    // searching in blk3_id
    if(par_dir->blk3_id!=-1)
    {
       for( i=0;i<64;i++)
        {
            if(f_sys->Data[par_dir->blk3_id].blk_ids[i]!=-1)
            {
                for(j=0;j<64;j++)
                {
                    if(f_sys->Data[ f_sys->Data[par_dir->blk3_id].blk_ids[i]].blk_ids[j]!=-1)
                    {
                       status=search_entry(f_sys->Data[f_sys->Data[f_sys->Data[par_dir->blk3_id].blk_ids[i]].blk_ids[j]],filename);
                        if(status!=-1)
                        {
                           return status;
                        } 
                    }
                    
                }

            }
        }
            
    }
    return -1;
}

int showfile_myfs(char *filename)
{
    int file_index=search_file(filename,curr_ind);
    cout<<" file index "<<file_index<<endl;
    if(file_index==-1)
        return -1;
    inode *temp = &( f_sys->indb[file_index] );
    int req_size=temp->file_size;
    int i=0,j=0;
    char buf[257];
    /* reading in the inode */
    for(i=0;i<8;i++)
    {
        if(temp->blk1_id[i]!=-1)
        {
            bzero(buf,257);
            memcpy(buf,f_sys->Data[temp->blk1_id[i]].data,min(256,req_size));
            cout<<buf;
            req_size-=min(256,req_size);
            if(req_size==0)
            {
                cout<<endl;
                return 1;
            }
        
        }
        
    }
    
    //readig  in blk2_d
    if(temp->blk2_id!=-1)
    {
        for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk2_id].blk_ids[i]!=-1)
            {
                bzero(buf,257);
                memcpy(buf,f_sys->Data[f_sys->Data[temp->blk2_id].blk_ids[i]].data,min(256,req_size));
                cout<<buf;
                req_size-=min(256,req_size);
                if(req_size==0)
                {
                    cout<<endl;
                    return 1;
                }
            } 
           
        }
    }
    // reading in blk3_id
    if(temp->blk3_id!=-1)
    {
       for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk3_id].blk_ids[i]!=-1)
            {
                for(j=0;j<64;j++)
                {
                    if(f_sys->Data[ f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]!=-1)
                    {
                      bzero(buf,257);
                        memcpy(buf,f_sys->Data[f_sys->Data[ f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]].data,min(256,req_size));
                        cout<<buf;
                        req_size-=min(256,req_size);
                        if(req_size==0)
                        {
                            cout<<endl;
                            return 1;
                        }
                    }
           
                }
            }
        }
    }

}

int copy_myfs2pc(char * source,char * dest)
{
    int file_index=search_file(source,curr_ind);
    inode *temp=&(f_sys->indb[file_index]);
    if(file_index==-1)
        return file_index;
    ofstream outfile;
    outfile.open(dest);
    int req_size=temp->file_size;
    int i=0,j=0;
    char buf[257];
    /* reading in the inode */
    for(i=0;i<8;i++)
    {
        if(temp->blk1_id[i]!=-1)
        {
            bzero(buf,257);
            memcpy(buf,f_sys->Data[temp->blk1_id[i]].data,min(256,req_size));
            outfile<<buf;
            req_size-=min(256,req_size);
            if(req_size==0)
            {
                outfile.close();
                return 1;
            }
        
        }
        
    }
    //readig  in blk2_d
    if(temp->blk2_id!=-1)
    {
        for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk2_id].blk_ids[i]!=-1)
            {
                bzero(buf,257);
                memcpy(buf,f_sys->Data[f_sys->Data[temp->blk2_id].blk_ids[i]].data,min(256,req_size));
                outfile<<endl;
                req_size-=min(256,req_size);
                if(req_size==0)
                {
                    outfile.close();
                    return 1;
                }
            } 
           
        }
    }
    // reading in blk3_id
    if(temp->blk3_id!=-1)
    {
       for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk3_id].blk_ids[i]!=-1)
            {
                for(j=0;j<64;j++)
                {
                    if(f_sys->Data[ f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]!=-1)
                    {
                        bzero(buf,257);
                        memcpy(buf,f_sys->Data[f_sys->Data[ f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]].data,min(256,req_size));
                        outfile<<endl;
                        req_size-=min(256,req_size);
                        if(req_size==0)
                        {
                            outfile.close();
                            return 1;
                        }
                    }
           
                }
            }
        }
    }
    return -1;

}

/* creating a directory */
int mkdir_myfs(char *dir_name)
{
    int ind_index=get_free_inode();
    if(ind_index==-1)
        return -1;
    add_entry(dir_name,(short)ind_index);
    create_inode(ind_index,DIR,256);
    int parent_inode=curr_ind;
    chdir_myfs(dir_name);
    add_entry(".",ind_index);
    add_entry("..",parent_inode);
    chdir_myfs("..");
    return 1;
}

/* changing the directory */
int chdir_myfs(char *dir_name)
{
    int dir_index=search_file(dir_name,curr_ind);
    if(dir_index==-1)
        return -1;
    curr_dir=&(f_sys->indb[dir_index]);
    curr_ind=dir_index;
}


/* removing thr directory */
int rem_dir_files(block &blk,int par_inode);

int rmdir_myfs_ind(char *dir_name,int par_ind)
{
    int dir_ind=search_file(dir_name,par_ind);
    if(dir_ind==-1)
        return -1;
    inode *temp= &(f_sys->indb[dir_ind]);
    int i=0,j=0,status;
    for(i=0;i<8;i++)
    {
        if(temp->blk1_id[i]!=-1)
        {
           rem_dir_files(f_sys->Data[temp->blk1_id[i]],dir_ind);
           
        }
    }
    
    //searching in blk2_d
    if(temp->blk2_id!=-1)
    {
        for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk2_id].blk_ids[i]!=-1)
            {
                rem_dir_files(f_sys->Data[temp->blk1_id[i]],dir_ind);
            }
            
        }
    }
    // searching in blk3_id
    if(temp->blk3_id!=-1)
    {
       for( i=0;i<64;i++)
        {
            if(f_sys->Data[temp->blk3_id].blk_ids[i]!=-1)
            {
                for(j=0;j<64;j++)
                {
                    if(f_sys->Data[ f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]!=-1)
                    {
                       rem_dir_files(f_sys->Data[temp->blk1_id[i]],dir_ind);
                    }
                    
                }

            }
        }
            
    }
    rm_myfs_ind(dir_name,par_ind);
    return 1;
}


int rem_dir_files(block &blk,int par_inode)
{
    short s=-1;
    for(int i=0;i<8;i++)
    {
        if((blk.dir_[i]).inode_number!=-1)
        {
            if(strcmp((blk.dir_[i]).file_name,"..")==0 or strcmp((blk.dir_[i]).file_name,".")==0)
                continue;
            if(f_sys->indb[(blk.dir_[i]).inode_number].file_type==FIL)
            {
                de_alloc_blocks((blk.dir_[i]).inode_number);
                f_sys->sb.free_inodes[(blk.dir_[i]).inode_number]=0;
            }
            else
            {
                rmdir_myfs_ind((blk.dir_[i]).file_name,par_inode);
            }
            
        }
    }
}


int rmdir_myfs(char *dir_name)
{
    return rmdir_myfs_ind(dir_name,curr_ind);
}

file_descrpt file_table[20];
int max_fd=20;
void set_file_descrpt(int fd,inode *f_ind,char m)
{
    file_table[fd].offset=0;
    file_table[fd].ind=f_ind;
    file_table[fd].mode=m;
}
int open_myfs(char *filename,char mode)
{
    int fil_nd=search_file(filename,curr_ind);
    if(fil_nd==-1 and mode=='r')
        return -1;
    if(mode=='w')
    {
        fil_nd=get_free_inode();
        create_inode(fil_nd,FIL,0);
        add_entry(filename,fil_nd);
    }
    int new_fd=-1;
    for(int i=0;i<max_fd;i++)
    {
        if(file_table[i].fd_status==0)
        {
            new_fd=i;
            file_table[i].fd_status=1;
            break;
        }
    }
    if(new_fd==-1)
        return -1;
    set_file_descrpt(new_fd,&(f_sys->indb[fil_nd]),mode);
    return new_fd;
}

int close_myfs(int fd)
{
    if(file_table[fd].fd_status==0)
        return -1;
    file_table[fd].fd_status=0;
    return 1;
}

int search_filetable(int fd)
{

    if(file_table[fd].fd_status==1)
        return fd;
    return -1;
}

int read_myfs(int fd,int nbytes,char *buff)
{
    int f_id=search_filetable(fd);

    // cout<<"f_id: "<<f_id<<endl;
    if(f_id==-1 or file_table[f_id].mode=='w' )
        return -1;

    inode *temp=file_table[f_id].ind;
    int curr_dblock=file_table[f_id].offset/256;
    // cout<<"offset: "<<file_table[f_id].offset<<endl;
    // sleep(2);
    /* required variables */
    int blk_start=0;
    int i=0,j=0;
    int tot_blks=temp->file_size/256;
    bool started=0;
    int position=file_table[f_id].offset%256,req_bytes=nbytes;

    if(temp->file_size%256!=0)
        tot_blks++;
    
    for(i=0;i<min(8,tot_blks);i++)
    {
        if(curr_dblock==blk_start or started==1)
        {
            if(curr_dblock==blk_start)
                started=1;
            while(position<256 and req_bytes>0 and temp->file_size> file_table[f_id].offset)
            {
                buff[nbytes-req_bytes]=f_sys->Data[temp->blk1_id[i]].data[position];
                position++;
                file_table[f_id].offset++;
                req_bytes--;
            }
            position%=256;
        }
        blk_start++;
    }
    
    //searching in blk2_d
    if(temp->blk2_id!=-1)
    {
        for( i=0;i<min(64,tot_blks-8);i++)
        {
            if(curr_dblock==blk_start or started==1)
            {
                if(curr_dblock==blk_start)
                    started=1;
                while(position<256 and req_bytes>0 and temp->file_size> file_table[f_id].offset)
                {
                    buff[nbytes-req_bytes]=f_sys->Data[(f_sys->Data[temp->blk2_id]).blk_ids[i]].data[position];
                    position++;
                    file_table[f_id].offset++;
                    req_bytes--;
                }
                position%=256;
            }
            blk_start++;
        }
    }
    // searching in blk3_id
    if(temp->blk3_id!=-1)
    {
       for( i=0;i<min(64,tot_blks-72-64*i);i++)
        {
            if(f_sys->Data[temp->blk3_id].blk_ids[i]!=-1)
            {
                for(j=0;j<min(64,tot_blks-72-64*i-j);j++)
                {
                    if(curr_dblock==blk_start or started==1)
                    {
                        if(curr_dblock==blk_start)
                            started=1;
                        while(position<256 and req_bytes>0 and temp->file_size > file_table[f_id].offset)
                        {
                            buff[nbytes-req_bytes]=f_sys->Data[f_sys->Data[f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]].data[position];
                            position++;
                            file_table[f_id].offset++;
                            req_bytes--;
                        }
                        position%=256;
                    }
                    blk_start++;
                    
                }

            }
        }
            
    }

    buff[nbytes-req_bytes]='\0';
    return (nbytes-req_bytes);
}

int write_myfs(int fd,int nbytes,char *buff)
{
    int f_id=search_filetable(fd);
    if(f_id==-1 or file_table[f_id].mode=='r' )
        return -1;
    inode *temp=file_table[f_id].ind;
    int curr_dblock=file_table[f_id].offset/256;
    // sleep(10);
    /* required variables */
    int blk_start=0;
    int i=0,j=0;
    int position=file_table[f_id].offset%256,req_bytes=nbytes;
 
    for(i=min(8,curr_dblock);i<8;i++)
    {
        if(temp->blk1_id[i]==-1 and req_bytes>0)
        {
            temp->blk1_id[i]=get_free_block();
        }

        while(position<256 and req_bytes>0)
        {
            f_sys->Data[temp->blk1_id[i]].data[position]=buff[nbytes-req_bytes];
            position++;
            file_table[f_id].offset++;
            req_bytes--;
            temp->file_size++;
        }
        position%=256;
  
    }
    if(req_bytes>0 and curr_dblock<8)
    {
        curr_dblock++;
    }
    if(curr_dblock>=8 and req_bytes>0)
    {
        //searching in blk2_d
        if(temp->blk2_id==-1 )
        {
            temp->blk2_id=get_free_block();
            set_ids(f_sys->Data[temp->blk2_id].blk_ids,64);
        }

        if(temp->blk2_id!=-1)
        {
            for( i=min(64,(curr_dblock-8));i<64;i++)
            {

                if((f_sys->Data[temp->blk2_id]).blk_ids[i]==-1 and req_bytes>0)
                {
                    (f_sys->Data[temp->blk2_id]).blk_ids[i]=get_free_block();
                }
                while(position<256 and req_bytes>0)
                {
                    f_sys->Data[(f_sys->Data[temp->blk2_id]).blk_ids[i]].data[position]=buff[nbytes-req_bytes];;
                    position++;
                    file_table[f_id].offset++;
                    temp->file_size++;
                    req_bytes--;
                }
                position%=256;
            }
        }

    }
    
    // searching in blk3_id


    if(req_bytes>0 and curr_dblock<72)
    {
        curr_dblock++;
    }
    if(curr_dblock>=72 and req_bytes>0)
    {

        if(temp->blk3_id==-1)
        {
            temp->blk3_id=get_free_block();
            set_ids(f_sys->Data[temp->blk3_id].blk_ids,64); 
        }
        if(temp->blk3_id!=-1 and req_bytes>0)
        {
            int db_curr=(curr_dblock-72)/(64);
            if((curr_dblock-72)%(64)!=0)
            {
                db_curr++;
            }
            int start_id=(curr_dblock-72)%(64);
            for( i=min(64,db_curr);i<64;i++)
            {
                if(f_sys->Data[temp->blk3_id].blk_ids[i]==-1 and req_bytes>0)
                {
                    f_sys->Data[temp->blk3_id].blk_ids[i]=get_free_block();
                     set_ids(f_sys->Data[f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids,64);
                }
                if(f_sys->Data[temp->blk3_id].blk_ids[i]!=-1)
                {

                    for(j=min(64,start_id);j<64;j++)
                    {
                        if(f_sys->Data[f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]==-1 and req_bytes>0)
                        {
                            f_sys->Data[f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]=get_free_block();
                        }
                    
                        while(position<256 and req_bytes>0)
                        {
                            f_sys->Data[f_sys->Data[f_sys->Data[temp->blk3_id].blk_ids[i]].blk_ids[j]].data[position]=buff[nbytes-req_bytes];
                            position++;
                            file_table[f_id].offset++;
                            temp->file_size++;                            
                            req_bytes--;
                        }
                        position%=256;
                        if(start_id>0)
                        {
                            start_id=0;
                        }
                    }

                }
            }

        }
         
    }

    return (nbytes-req_bytes);
}

int eof_myfs(int fd)
{
    int f_ind=search_filetable(fd);
    if(f_ind==-1)
        return -1;
    if(file_table[f_ind].mode=='w')
        return -1;
    if(file_table[f_ind].offset==file_table[f_ind].ind->file_size)
        return 1;
    return 0;
}

int dump_myfs(char *dumpfile)
{
    ofstream out;
    out.open(dumpfile);
    out.write((char*)&f_sys->sb,sizeof(super_block));
    for(int i=0;i<f_sys->sb.max_inodes;i++)
    {
        out.write((char*)&f_sys->indb[i],sizeof(inode));
    }
    for(int i=0;i<f_sys->sb.max_bks;i++)
    {
         out.write((char*)&f_sys->Data[i],sizeof(block));
    }
    out.close();
    return 1;
}

int restore_myfs(char *dumpfile)
{
    init_file_system();    
    ifstream in;
    in.open(dumpfile);
    if(!in)
        return -1;
    in.read((char*)&f_sys->sb,sizeof(super_block));
    // f_sys->indb = new inode[f_sys->sb.max_inodes];
    for(int i=0;i<f_sys->sb.max_inodes;i++)
    {
        in.read((char*)&f_sys->indb[i],sizeof(inode));
    }
    for(int i=0;i<f_sys->sb.max_bks;i++)
    {
         in.read((char*)&f_sys->Data[i],sizeof(block));
    }
    curr_dir = &(f_sys->indb[rt_ind]);    
    in.close();
    return 1;
}

int status_myfs()
{
    if(f_sys->sb.max_bks<=0)
        return -1;
    cout<<"fs info \n";
    cout<<"max inodes: "<<f_sys->sb.max_inodes<<endl;
    cout<<"fs size: "<<f_sys->sb.tot_fs_size<<endl;
    cout<<"used_bks: "<<f_sys->sb.used_bks<<endl;
    cout<<"tot_files: "<<count(f_sys->sb.free_inodes,f_sys->sb.free_inodes+f_sys->sb.max_inodes,1)<<endl;
    cout<<"free_blocks: "<<count(f_sys->sb.free_blks,f_sys->sb.free_blks+f_sys->sb.max_bks,0)<<endl;
    return 1;
}


int chmod_myfs(char *filename,int mode)
{
    int file_ind=search_file(filename,curr_ind);
    if(file_ind==-1)
        return -1;
    f_sys->indb[file_ind].permissions=mode;
    return 1;
}


