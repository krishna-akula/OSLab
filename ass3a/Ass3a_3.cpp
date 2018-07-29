#include<bits/stdc++.h>

using namespace std;

//creating the struct for each process

struct process
{
  int arrival_time;
  int finish_time;
  int cpu_burst_time;
  int next_cpu;
  int time_spent;
  int id;
  process()
  {}
  process(int arv,int burst_time,int pid)
  {
    arrival_time=arv;
    cpu_burst_time=burst_time;
    next_cpu=burst_time;
    finish_time=0;
    id=pid;
    time_spent=0;
  }

};

int get_inter_arrival(double R)
{
  double temp ;
  temp=(-1.0/0.35)*(log(R));
  return ((int)temp)%11;

}

double caluculate_atn(vector<process> &sys_proc)
{
  int N=sys_proc.size();
  int to_tn=0;
  for(int i=0;i<N;i++)
  {
    to_tn+=(sys_proc[i].finish_time-sys_proc[i].arrival_time);
  }
  return ((double)to_tn)/N;

}
void print_finish_times(vector<process>&sys_proc)
{
  int N=sys_proc.size();
  cout<<"process_no\t\tstarttime\t\tfinishtime\t\ttime_spent\n";
  for(int i=0;i<N;i++)
  {
    cout<<sys_proc[i].id<<"\t\t\t"<<sys_proc[i].arrival_time<<"\t\t\t"<<sys_proc[i].finish_time<<"\t\t\t"<<sys_proc[i].time_spent<<endl;
  }

}

int get_FCFS(vector<process>sys_proc)
{
  int N=sys_proc.size();
  int last_finish_time=0;
  last_finish_time=sys_proc[0].cpu_burst_time;
  sys_proc[0].finish_time=sys_proc[0].cpu_burst_time;
  for(int i=1;i<N;i++)
  {
    if(last_finish_time<sys_proc[i].arrival_time)
      last_finish_time=sys_proc[i].arrival_time;
    sys_proc[i].finish_time=last_finish_time+sys_proc[i].cpu_burst_time;
    last_finish_time=sys_proc[i].finish_time;
  }
  cout<<"After running FcFs\n";
  // print_finish_times(sys_proc);
  cout<<"Average turn around in FCFS:\t\t"<<caluculate_atn(sys_proc)<<endl;
  return caluculate_atn(sys_proc);

}

bool comparator( const process &a, const process &b)
{
  if(a.next_cpu!=b.next_cpu)
      return a.next_cpu<b.next_cpu;
  return a.arrival_time<b.arrival_time;
}

void print_info(const process &a)
{
  cout<<a.id<<"\t"<<a.next_cpu<<"\t"<<a.time_spent<<endl;
}

int premp_sjf(vector<process>sys_proc)
{
  // cout<<"premptive-sjf:\n";
  int N=sys_proc.size();
  // cout<<N<<endl;
  vector<process>fp;
  vector<process>pq;
  int i=0;
  pq.push_back(sys_proc[i]);
  int current_time=0;
  i++;
  while(i<N)
  {
    sort(pq.begin(),pq.end(),comparator);
    if(pq.size()>0)
    {

      if(pq[0].time_spent==pq[0].cpu_burst_time)
      {
        pq[0].finish_time=current_time;
        fp.push_back(pq[0]);
        pq.erase(pq.begin());
        continue;
      }
      int last_finish_time=current_time+pq[0].next_cpu;
      if(sys_proc[i].arrival_time>=last_finish_time)
      {
        pq[0].finish_time=last_finish_time;
        current_time=last_finish_time;
        pq[0].time_spent=pq[0].cpu_burst_time;
        fp.push_back(pq[0]);
        pq.erase(pq.begin());
        continue;
      }
      else
      {
        // cout<<127<<endl;
        // print_info(pq[0]);
        // print_info(sys_proc[i]);
        pq[0].time_spent+=sys_proc[i].arrival_time-current_time;
        pq[0].next_cpu=pq[0].cpu_burst_time-pq[0].time_spent;
        // print_info(pq[0]);
        current_time=sys_proc[i].arrival_time;
        // cout<<current_time<<endl;
      }
    }
    else
    {
      current_time=sys_proc[i].arrival_time;
    }
    pq.push_back(sys_proc[i]);
    i++;
  }
  // cout<<146<<endl;
  // print_finish_times(pq);
  while(pq.size()!=0)
  {
    // cout<<147<<endl;
    // print_info(pq[0]);
    int last_finish_time=current_time+pq[0].next_cpu;
    pq[0].finish_time=last_finish_time;
    current_time=last_finish_time;
    pq[0].time_spent+=pq[0].next_cpu;
    fp.push_back(pq[0]);
    pq.erase(pq.begin());
  }
  // print_finish_times(fp);
  cout<<"Average turn around in premptive sjf:\t\t"<<caluculate_atn(fp)<<endl;
  return caluculate_atn(fp);

}

void print_queue(queue<process>q)
{
  cout<<"process_no\t\tstarttime\t\tfinishtime\t\ttime_spent\n";
  while(!q.empty())
  {
    process temp=q.front();
    q.pop();
    cout<<temp.id<<"\t\t\t"<<temp.arrival_time<<"\t\t\t"<<temp.finish_time<<"\t\t\t"<<temp.time_spent<<endl;

  }
}

int round_robin(vector<process>sys_proc,int delta)
{
   int N=sys_proc.size();
   queue<process>qu;
   vector<process>fp;
   int i=0;
   qu.push(sys_proc[0]);
   int current_time=0;
   // print_finish_times(sys_proc);
   i++;
   while(i<N)
   {
     if(qu.size()>0)
     {
       process temp=qu.front();
       qu.pop();
       int last_finish_time=current_time+temp.next_cpu;;
       int time_span=current_time+delta;
       int next_arrival=sys_proc[i].arrival_time;
       while(sys_proc[i].arrival_time<=time_span and i<N)
       {

         qu.push(sys_proc[i]);
         i++;
       }
       // cout<<197<<endl;
       // print_queue(qu);
       if(time_span>=last_finish_time)
       {
         temp.time_spent+=temp.next_cpu;
         temp.next_cpu=0;
         temp.finish_time=last_finish_time;
         current_time=last_finish_time;
         fp.push_back(temp);
         // cout<<"pushing info\t"<<current_time<<endl;
         // print_info(temp);
         // continue;
       }
       if(time_span<last_finish_time)
       {
         temp.time_spent+=delta;
         temp.next_cpu=temp.cpu_burst_time-temp.time_spent;
         qu.push(temp);
         current_time=time_span;
         // cout<<"running info\t"<<current_time<<endl;
         // print_info(temp);
       }

     }
     else
     {
       qu.push(sys_proc[i]);
       i++;
       while(sys_proc[i-1].arrival_time==sys_proc[i].arrival_time and i<N)
       {

         qu.push(sys_proc[i]);
         i++;
       }
       current_time=sys_proc[i].arrival_time;
   }
  }
   // print_finish_times(fp);
   // cout<<"current time\n"<<current_time<<endl;
   // print_queue(qu);
   while(!qu.empty())
   {
     process temp=qu.front();
     int last_finish_time=current_time+temp.next_cpu;;
     int time_span=current_time+delta;
     qu.pop();
     if(time_span>=last_finish_time)
     {
       temp.time_spent+=temp.next_cpu;
       temp.next_cpu=0;
       temp.finish_time=last_finish_time;
       current_time=last_finish_time;
       fp.push_back(temp);
       // cout<<"pushing info\t"<<current_time<<endl;
       // cout<<"pushing info\n";
       // print_info(temp);
       // continue;
     }
     if(time_span<last_finish_time)
     {
       temp.time_spent+=delta;
       temp.next_cpu=temp.cpu_burst_time-temp.time_spent;
       qu.push(temp);
       current_time=time_span;
       // cout<<"running info\t"<<current_time<<endl;
       // cout<<"running info\n";
       // print_info(temp);
     }
   }
   // print_finish_times(fp);
   cout<<"Average turn around in round robin :\t\tdelata\t\t"<<delta<<"\t\t"<<caluculate_atn(fp)<<endl;
   return caluculate_atn(fp);
}

int main()
{
  int N;
  cout<<"enter the no of process\n";
  cin>>N;
  //Doing Fcfs for 10 times
  //for one iteration
  // int a[]={10,50,100};
  // int avg_atn[3][5];
  // for(int i=0;i<3;i++)
  // {
  // 	for(int j=0;j<5;j++)
  // 	{
  // 		avg_atn[i][j]=0;
  // 	}
  // }
  // for(int n=0;n<3;n++)
  {
  	  // N=a[n];
	  // cout<<"n:\t"<<N<<endl;
	  // for(int k=0;k<10;k++)
	  {
	      // cout<<"iteration :"<<k<<endl;
	      srand(9);
	      vector<process>sys_proc;
	      int last_arrival=0;
	      sys_proc.push_back(process(last_arrival,(rand()%20)+1,0));
	      for(int i=1;i<N;i++)
	      {
	        int int_arv=get_inter_arrival(rand()/(double)RAND_MAX);
	        sys_proc.push_back(process((last_arrival+int_arv),(rand()%20)+1,i));
	        last_arrival+=int_arv;
	      }
	      cout<<"Created processes with start time\n";
	      cout<<"process_no\tstarttime\tcpu_burst_time\n";
	      for(int i=0;i<N;i++)
	      {
	        cout<<i<<"\t\t\t"<<sys_proc[i].arrival_time<<"\t\t\t"<<sys_proc[i].cpu_burst_time<<endl;
	      }
	      // avg_atn[n][0]+=;
	      get_FCFS(sys_proc);
	      // avg_atn[n][1]+=;
	      premp_sjf(sys_proc);
	      // avg_atn[n][2]+=;
	      round_robin(sys_proc,1);
	      // avg_atn[n][3]+=;
	      round_robin(sys_proc,2);
	      // avg_atn[n][4]+=;
	      round_robin(sys_proc,5);
	      // print_finish_times(sys_proc);
		}
  }
  // ofstream myfile;
  // myfile.open("data.csv");
  // // cout<<"total caluculated avg for ten iterations for each n\n";
  //  myfile<<"n,fcfs,premp_sjf,rr_1,rr_2,rr_5"<<endl;
  // for(int i=0;i<3;i++)
  // {
  // 	myfile<<a[i];
  // 	for(int j=0;j<5;j++)
  // 	{
  // 		myfile<<",\t"<<avg_atn[i][j]/10.0;
  // 	}
  // 	myfile<<endl;
  // }
  // myfile.close();
}
