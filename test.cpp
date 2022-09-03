#include <cstdio>
#include <iostream>
using namespace std;

int main() {
  system ("/bin/stty raw -echo");
  for(int i=0; i<20; i++) {
    cout<<(getchar())<<endl;
  }
  system ("/bin/stty cooked");
}
