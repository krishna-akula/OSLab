#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<limits.h>
using namespace std;
int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int Min(int a,int b)
{
	if(a<b)
		return a;
	return b;
}
int Min3(int a,int b,int c)
{
	int temp=Min(a,b);
	return Min(temp,c);
}

int main()
{
	
	int smallest; //variable for finding smallest number

	int pip_a[2],pip_b[2],pip_c[2]; //pipes for the processes
	pid_t id_a,id_b,id_c,par;

	int arr[300]; //for storing the sorted numbers
	par=getpid(); //doing the required functions in the parent process
	if(pipe(pip_b)==-1 ) //creating the pipe and checking if there is any error
	{
		printf("sytem error");
	}
	if( pipe(pip_c)==-1 ) //creating the pipe and checking if there is any error
	{
		printf("sytem error");
	}
	if(pipe(pip_a)==-1 ) //creating the pipe and checking if there is any error
	{
		printf("sytem error");
	}

	id_a=fork(); //creatig process a
	if(id_a==0)
	{
		srand(getpid()); //seeding the random umber generator based on the pid of the current program
		int arr1[100];
		for(int i=0;i<100;i++)
		{
			arr1[i]=(rand()%1000+1);
		}
		qsort(arr1,100,sizeof(int),compare);  //sorting the arr1 using inbuit qsort
		//printing the elements in the process a
		for(int i=0;i<100;i++)
		{
			printf(" %d ",arr1[i]);
		}
		printf(" are numbers in a\n");
		//sending elements one by one through the pipe,closing reading end while writing end
		for(int i=0;i<100;i++)
		{
			close(pip_a[0]);
		 write(pip_a[1],arr1+i,sizeof(int));
		}
		
	}
	else
	{
		id_b=fork();
		if(id_b==0)
		{
			srand(getpid());  //seeding the random umber generator based on the pid of the current program
			int arr2[100];
			for(int i=0;i<100;i++)
			{
				arr2[i]=(rand()%1000+1);
			}
			qsort(arr2,100,sizeof(int),compare);
			for(int i=0;i<100;i++)
			{
				printf(" %d ",arr2[i]);
			}
			printf("are numbers in b\n");
			//sending elements one by one through the pipe, closing reading end while writing end
			for(int i=0;i<100;i++)
			{
				close(pip_b[0]);
				write(pip_b[1],arr2+i,sizeof(int));
			}
		
		

		}
		else
		{
			id_c=fork();
			
			if(id_c==0)
			{
				srand(getpid());  //seeding the random umber generator based on the pid of the current program
				int arr3[100];
				for(int i=0;i<100;i++)
				{
					arr3[i]=(rand()%1000+1);
				}
				qsort(arr3,100,sizeof(int),compare);
				for(int i=0;i<100;i++)
				{
					printf(" %d ",arr3[i]);
				}
				printf(" are numbers in c\n");
				//sending elements one by one through the pipe closing reading end while writing end
				for(int i=0;i<100;i++)
				{
					close(pip_c[0]);
					write(pip_c[1],arr3+i,sizeof(int));
				}
				

			}
		}

	}

	if(par==getpid())
	{
		
		//closing writing end when using th reading end
		close(pip_c[1]);
		close(pip_b[1]);
		close(pip_a[1]);
		
		int a,b,c,ca=0,cb=0,cc=0;
		int av,bv,cv,ind=0;
		int changea=1,changeb=1,changec=1;
		
		while(ind<300)
		{

			//heare for all pipes we are reading element by element  and erging using three pointer method
			if(changea)
			{
				if(ca>=100)
					av=INT_MAX;
				else
				{
					read(pip_a[0],&av,sizeof(av));
					changea=0;ca++;
				}
				
			}
			if(changeb)
			{
				if(cb>=100)
					bv=INT_MAX;
				else
				{
					read(pip_b[0],&bv,sizeof(bv));
					changeb=0;cb++;
				}
			}
			if(changec)
			{
				if(cc>=100)
					cv=INT_MAX;
				else
				{
					read(pip_c[0],&cv,sizeof(cv));
					changec=0;cc++;
				}
			}

			int temp=Min3(av,bv,cv);
			smallest=Min(temp,smallest);
			if(temp==av)
			{
				changea=1;
				arr[ind]=av;
				ind++;
			}
			else if(temp==bv)
			{
				changeb=1;
				arr[ind]=bv;
				ind++;
			}
			else if(temp==cv)
			{
				changec=1;
				arr[ind]=cv;
				ind++;
			}

		}

		printf("The smallest integer obtained from the processes is %d\n",arr[0]);
		printf("the sorted array d is \n");
		//printing the array d
		for(int k=0;k<300;k++)
		{
			printf("%d ",arr[k]);
		}

		printf("\n");

	}


}