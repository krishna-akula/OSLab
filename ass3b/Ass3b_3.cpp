#include<pthread.h>
#include<iostream>
#include<algorithm>
#include<queue>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<time.h>
#include<signal.h>
#include<vector>
#include<stdlib.h>
using namespace std;

///important variables
int N;
vector<int>status(1000);
vector<pthread_t>thread_id(1000);
pthread_t reporter,scheduler;
// pthread_t reporter,scheduler;
// vector<int>terminated(1000,0);
int pre_id,curr_id;
double current_time;
int change;
int Terminate,term_id;
int count_term=0;
// vector<status>thrd_id(1000);
queue<int>pq;
////
void signalHnadler1(int sig)
{
  signal(SIGUSR1,signalHnadler1);
  pause();
  // sleep(1000);

}

void signalHnadler2(int sig)
{
  // pthread_kill(pthread_self(),SIGUSR1)
  signal(SIGUSR2,signalHnadler2);

}


double get_sec(clock_t start,clock_t stop)
{
  return (double)(stop - start)/ CLOCKS_PER_SEC;
}



void *worker(void *param)
{
  signal(SIGUSR1,signalHnadler1);
  signal(SIGUSR2,signalHnadler2);
  pause();
  int t_id=(long)param;
  // cout<<"t_id:"<<t_id<<endl;
  int arr[1000];
  srand(t_id);
  for(int i=0;i<1000;i++)
  {
    arr[i]=rand()%10000+1;
  }
  int wait_time=rand()%10+1;
  // cout<<"wait:"<<wait_time<<endl;
  sort(arr,arr+1000);
  int i=wait_time;
  while(i>0)
  {
    // sleep(1);
    pause();
    // cout<<"tid:"<<t_id<<"wait"<<i<<endl;
    i--;
  }
  // cout<<t_id<<"\tterminated"<<endl;
  status[t_id]=1;
  pthread_kill(scheduler,SIGUSR2);
  pthread_exit(0);
}

void *Reporter(void * param)
{
  pthread_t sid=(long)param;
  while(1)
  {
    if(change==1)
    {
      cout<<"context switch occured bw id:"<<pre_id<<",id:"<<curr_id<<"at time:"<<current_time<<endl;
      change=0;
      // pthread_kill(sid,SIGUSR2);


    }
    if(Terminate==1)
    {
      cout<<"terminated :"<<term_id<<"time:"<<current_time<<endl;
      Terminate=0;
      if(count_term==N)
      	break;
      // pthread_kill(sid,SIGUSR2);

    }
  }
  pthread_exit(0);
}



void *sched(void *param)
{

  signal(SIGUSR1,signalHnadler1);
  signal(SIGUSR2,signalHnadler2);
  clock_t start;
  start=clock();
  current_time=get_sec(start,clock());
  double time_slice=1.0;
  // cout<<pq.size()<<endl;
  int lkp=0;
  while(!pq.empty())
  {
      int id=pq.front();
      pq.pop();
      // pthread_kill(pthread_self(),SIGCONT);
      // cout<<pq.size()<<endl;
      // lkp++;
      clock_t begin,end;
      begin=clock();
      current_time=get_sec(start,clock());
      pthread_kill(thread_id[id],SIGUSR2);
      // cout<<"id\n";
      // while(true)
      // {
        // if((clock()-begin)>CLOCKS_PER_SEC)
        // {
      sleep(1);
            // if(status[id]==0)
              pthread_kill(thread_id[id],SIGUSR1);
            pre_id=id;
            if(status[id]==1)
            {


              term_id=id;
              // cout<<"Terminated:"<<id<<endl;
              count_term++;
              current_time=get_sec(start,clock());
              Terminate=1;
              // break;
            }
            if(status[id]==0)
            {
              pq.push(id);
              curr_id=pq.front();
              // cout<<"pre_id"<<pre_id<<"curr_id:"<<curr_id<<endl;
              current_time=get_sec(start,clock());
              change=1;
              // break;
            }
            usleep(10);
        // }
      // }
      // usleep(10);
  }
  usleep(10);
  pthread_kill(reporter,SIGKILL);
  pthread_exit(0);
}


int main()
{
  // sleep(2);
  cout<<"N:\n";
  cin>>N;
  status.resize(N);
  thread_id.resize(N);
  for(int i=0;i<N;i++)
  {
    status[i]=0;
    pq.push(i);
  }
  // terminated.resize(N);
  // cout<<status.size()<<"\t"<<thread_id.size()<<endl;
  for(int i=0;i<N;i++)
  {
    pthread_create(&thread_id[i],NULL,worker,(void *)i);
  }
  // pthread_t reporter,scheduler;
  pthread_create(&reporter,NULL,Reporter,NULL);
  pthread_create(&scheduler,NULL,sched,(void *)reporter);
  pthread_join(scheduler,NULL);
  // pthread_exit(0);
  // while(count(status.begin(),status.end(),0)!=0){}
  // cout<<count(status.begin(),status.end(),0)<<endl;
  // cout<<171<<endl;
  // while(count_term<N){}
  // pthread_kill(scheduler,SIGKILL);
  return 0;
}
