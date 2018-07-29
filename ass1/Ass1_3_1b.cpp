#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<string>
#include<cstring>
#include<sstream>
#include<iostream>
using namespace std;


int main()
{

	pid_t id_child;
	int cstatus;
	pid_t c;


	while(1)
	{

		string command_line;
		string word,quit="quit";
	    getline(cin,command_line);
	    istringstream iss(command_line);
	    // string word;
	    char *argv[1024];
	    char argc[1024];
	    char *command;
	    int i=0;
	    //splitting the string for the arguments
	    iss>>word;
	    if(word.size()>0){
	    	command=new char[word.length()+1];
	    	strcpy(command,word.c_str());
	    	argv[i]=command;
	    	i++;
	    }
	    else command=" ";
	    if(word=="quit")//exiting when quit is entered
	    	break;
	    while(iss >> word) {
	       argv[i]=new char[word.length()+1];
	    	strcpy(argv[i],word.c_str());
	        i++;
	    }
	    
	    if(i==0)
	    {
	    	argv[0]=" ";
	    	i++;
	    }
	    argv[i]='\0';
	    //printf("%s",command);
	    // for(int j=0;j<i;j++)
	    // {
	    // 	printf("%s",argv[j]);
	    // }
	    //strcpy(argc,command_line.c_str());
	    //creating the child processes
	    id_child=fork();
	    if(id_child==0)
	    {
	    	//execlp("/bin/sh",argc);
	    	execvp(command,argv);
	    	printf("Child process could not do execlp.\n");//error if the child process is not executed
			exit(1);
	    }
	    else if(id_child==-1)
	    {
	    	//if forking is failed
	    	printf("Fork failed\n");
	    }
	    else
	    {
	    	//waiting for the child processes to be completed
	    	c=wait(&cstatus);

	    }

			
	}






}