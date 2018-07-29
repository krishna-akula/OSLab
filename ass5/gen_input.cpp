#include<bits/stdc++.h>
using namespace std;

int no_of_pages;
void gen_instructions(int no_references,int prob,int work_sets)
{
    ofstream myfile;

    int rw;
    int page;
    int genprob;
    deque<int> ws;//working set
    myfile.open("input.txt");
    /* initially creating present working set */
    for(int i=0;i<work_sets;i++)
    {
        rw=rand()%2;
        page=rand()%no_of_pages;
        ws.push_back(page);
        myfile<<rw<<"  "<<page<<"\n";

    }
    for(int i=0;i<no_references-work_sets;i++)
    {
        rw=rand()%2;
        genprob=rand()%101;
        if(genprob<prob)
        {
            page =rand()%ws.size();
            page = *(ws.begin()+page);
            
            // outfile<<rw<<endl;

        }
        else
        {
            page=rand()%no_of_pages;
            while(find(ws.begin(),ws.end(),page)!=ws.end())
            {
                page=rand()%no_of_pages;            
            }
            // outfile<<rw<<endl;
        }
        ws.pop_front();
        ws.push_back(page);
        myfile<<rw<<"  "<<page<<"\n";        
    }
     
    myfile.close();
}

int main()
{
    int work_sets;
    int no_references;
    int prob;    
    cout<<"no of working sets\n";
    cin>>work_sets;
    cout<<"no_pages\n";
    cin>> no_of_pages;
    cout<<"no_references\n";
    cin>>no_references;
    cout<<"prob\n";
    cin>>prob;
    gen_instructions(no_references,prob,work_sets);
}