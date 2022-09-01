#include <iostream>
using namespace std;

int main()
{
  int i;
  cin>> i;

  if(!cin.eof())
  {
    cout<< "\n Format Error \n";
  }
  else
  {
    cout<< i;
  }

  getchar();
  return 0;
}
