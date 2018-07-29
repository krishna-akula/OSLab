#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<string>
#include<cstring>
#include<sstream>
#include<vector>
#include<iostream>
using namespace std;


struct cmd_info{
	int type;
	string infilename,outfilename,filename;
	string cmd;
	string fcmd;
	string path;
	char *argv[1024];
	int size;
	cmd_info()
	{
		size=0;
	}

};



void run_internal_cmd(cmd_info &rd)
{
	int status=-1;
	if(rd.fcmd=="mkdir")
	{
		status=mkdir((rd.path).c_str(),0777);

	}
	else if(rd.fcmd=="rm")
	{
		status=rmdir((rd.path).c_str());

	}
	else if(rd.fcmd=="cd")
	{
		char cwd[256];
		getcwd(cwd,sizeof(cwd));
		cout<<"Original directory:\t"<<cwd<<"\n";
		status=chdir((rd.path).c_str());
		getcwd(cwd,sizeof(cwd));		
		cout<<"Changed directory:\t"<<cwd<<"\n";

	}
	if(status==-1)
		cout<<"enter correct command\n";
	else
		cout<<"command executed successfully\n";
}



void run_exec(cmd_info &rd)
{
	pid_t id_child;
	int cstatus=0;
	pid_t c;
	int in,out;
	
	id_child=fork();
    if(id_child==0)
    {
    	if(rd.type==3)
		{
			in  = open(rd.filename.c_str(), O_RDONLY);
			dup2(in,0);

		}
		else if(rd.type==4)
		{
			out = open(rd.filename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			dup2(out,1);
		}
    	// execlp("/bin/sh",argc);
    	// if(rd.type==5)
    	// {
    	// 	//setpgid(0,0);
    	// 	fclose(stdin);
    	// }
		// setpgid(0,0);

    	execvp((rd.cmd).c_str(),rd.argv);

    	cout<<"Child process could not do execlp.\n";//error if the child process is not executed
		exit(2);
    }
    else if(id_child==-1)
    {
    	//if forking is failed
    	cout<<"Fork failed\n";
    }
    else
    {


    	if(rd.type==5)
    		waitpid(id_child,&cstatus,WNOHANG);
    	else
    		waitpid(id_child,&cstatus,0);


    	
	}

}

void get_tokens(cmd_info &rd,string command_line)
{
	istringstream iss(command_line);
	string word;
	iss>>rd.cmd;
    if((rd.cmd).size()>0){
    	rd.argv[rd.size]=new char[(rd.cmd).length()+1];
    	strcpy(rd.argv[rd.size],(rd.cmd).c_str());
    	rd.size++;
    }
    else rd.cmd=" ";
    while(iss >> word) {
       if(word==">" or word=="<")
       {
       		iss>>rd.filename;
       		rd.type=word==">"?4:3;
       		break;	
       }
       else if(word=="&")
       {
       		break;
       }
       rd.argv[rd.size]=new char[word.length()+1];
    	strcpy(rd.argv[rd.size],word.c_str());
        rd.size++;
    }
    
    if(rd.size==0)
    {
    	rd.argv[0]=" ";
    	rd.size++;
    }
    rd.argv[rd.size]='\0';

}


void connect_cmds(int in,int out,cmd_info & rd,int last)
{
	pid_t id_child;
	pid_t c;
	int cstatus;

	id_child=fork();
	if(id_child==0)
	{
		if(in!=0)
		{
			dup2(in,0);
			close(in);
		}
		if(out!=1 and !last)
		{
		//	close(in);
			dup2(out,1);
			close(out);
		}
	
		execvp((rd.cmd).c_str(),rd.argv);
		cout<<" chld process could not be constructed\n";
		exit(1);
	}
	else
	{
		
		c=wait(&cstatus);
		close(in);
		close(out);
		return;
	}
}



void run_pipe_cmds(vector<cmd_info> &rds)
{
	int n=rds.size();
	int pipes[n-1][2];
	int in=0;
	// pipe(pipes);
	int stdo,stdi;
	stdi=dup(0);
	stdo=dup(1);
	for(int i=0;i<n-1;i++)
	{
		pipe(pipes[i]);
	}

	for(int i=0;i<n;i++)
	{
		if(i==n-1)
		{
			connect_cmds(in,1,rds[i],1);
			dup2(stdi,0);
			dup2(stdo,1);

		}
		else
		{
			dup2(in,0);
			connect_cmds(in,pipes[i][1],rds[i],0);
			in = pipes[i][0];
		}
		
	}

}


int main()
{
	cout<<"----------------------------------------------------------------------------------------------\n";
		cout<<"select the option for excecution\n";
		cout<<"1.\tInternal command\n";
		cout<<"2:\tExternal command\n";
		cout<<"3:\tExternal command by redirectting standard input\n";
		cout<<"4:\tExternal command by redirectting standard output\n";
		cout<<"5:\tExternal command in the background\n";
		cout<<"6:\tExternal command by for pipelining execution\n";
		cout<<"7:\tQuit the program\n";
	while(1)
	{
		int choice;
		pid_t par;
		par=getpid();
		cmd_info rd;
		
		cout<<">option\t\t:";
		cin>>choice;
		cout<<"\n";
		if(choice<1 or choice>=7)
			break;
		string command_line,word;
		cin.ignore(256, '\n');
		cout<<">command\t:";
    	getline(cin,command_line);
    	cout<<"\n";
    	rd.type=choice;
		if(choice==1)
		{
			istringstream iss(command_line);
			iss>>rd.fcmd;
			iss>>rd.path;
			run_internal_cmd(rd);
		}
		else if( (choice>=2 and choice <=4) or choice==5)
		{
			
			get_tokens(rd,command_line);
		    run_exec(rd);

		}
		else if(choice==6)
		{
			vector<string>pipe_cmds;
			string delim="|";
			size_t pos = 0;
		    string token;
		    while ((pos = command_line.find(delim)) != string::npos) {
		        token = command_line.substr(0, pos);
		        pipe_cmds.push_back(token);
		        command_line.erase(0, pos + delim.length());
		    }
		    if(command_line.size()!=0)
		    {
		    	pipe_cmds.push_back(command_line);
		    }
		    vector< cmd_info >rds(pipe_cmds.size());
		    for(int i=0;i<pipe_cmds.size();i++)
		    {
		    	rds[i].type=2;
		    	get_tokens(rds[i],pipe_cmds[i]);
   		    }
   		    // cout<<rds.size()<<endl;

   		    run_pipe_cmds(rds);

		}
		if(getpid()!=par)
		{
			exit(1);
		}
	}


}