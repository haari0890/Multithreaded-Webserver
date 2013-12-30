#include<iostream>
#include<thread>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string>
using namespace std;
using namespace this_thread;
class some
{
public:
int a;
string b;
some()
{
a = 1;
b = "hi";
}
some(int x,string y)
{
a = x;
b = y;
}
};
void fn1(int);
void fn(int a)
{
cout << a << endl;
signal(SIGILL,fn1);
raise(SIGILL);

cout << endl << get_id() << endl;
}
void fn1(int b)
{
cout << "something";
//exit(0);
}
int main()
{
thread threads[3];
some ob(10,"hihari");
vector<some> somevec;
somevec.push_back(ob);
cout << somevec.back().b << "*****";
some ob1;
somevec.push_back(ob1);
cout << somevec.back().b << "*****";
for (int i=0; i<3; i++)
{
threads[i] = thread(fn,i+i);
}
for(auto& th:threads)
{
th.join();
//th.detach();
}
cout << true;
return 1;
}
