#include <iostream>
#include <stack>
#include <chrono>
#include <ctime>
#include <functional>
#include <map>
#include <filesystem>
#include <unistd.h>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>

namespace fs = std::filesystem;
using namespace std;

typedef int (*FnPtr)(int, int);

vector<string> command;
vector<vector<string> > history;

const int pathsSize = 3;
string paths[pathsSize] = {"./", "/bin/", "/usr/bin/"};
string currentPath;
string v_currentPath = "~";
string homeDir;

int readCommands() {
  string inp;

  char c=100, pc=100;
  int xPos=0;
  system ("/bin/stty raw -echo");
  string line = "";
  string aec = "";
  while(1) {
    c = getchar();

    // cout<<" = "<<(int)c<<" "<<(getchar())<<" "<<(getchar())<<" "<<endl;
    if(c==4) {
      system ("/bin/stty cooked");
      exit(0);
    }

    else if(c==3)
      return(0);

    else if(c==13) {
      break;
      puts("breaking...");
    }

    else if(c==27 && getchar()==79) {
      int cc = getchar();

      if(cc==65) {
        puts("up");
      }
      else if(cc==66) {
        puts("down");
      }
      else if(cc==67 && line.size()-xPos>0) {
        xPos++;
      }
      else if(cc=68 && xPos>0) {
        xPos--;
      }
    }


    else if(c==127 && line.size()>=1) {
      xPos--;
      line.erase(line.begin()+xPos, line.begin()+xPos+1);
    }

    else {
      line.insert(xPos, string(1, c));
      xPos++;
    }

    cout<<"\33[2K\r";
    cout<<"$ "<<line;

    cout<<"\33[1000000;"<<xPos+3<<"H"; // TODO FIX THIS SHIT
    aec="";

    pc = c;
  }
  system ("/bin/stty cooked");

  //  getline(cin, inp);
  //
  //  if(cin.eof()) {
  //    cout<<endl;
  //    exit(0);
  //  }
  //
  //  int pi=0;
  //  for(int i=0; i<inp.size(); i++) {
  //    if(inp[i]==' ') {
  //      command.push_back(inp.substr(pi, i-pi));
  //      pi = i+1;
  //    }
  //  }
  //
  //  command.push_back(inp.substr(pi, inp.size()-pi));

  return(1);
}

void execute() {
  std::vector<const char*> argv;

  for(const auto& s: command) {
    argv.push_back(s.data());
  }
  argv.push_back(NULL);
  argv.shrink_to_fit();

  if(!fork()) {
    execv(command[0].c_str(), const_cast<char* const *>(argv.data()));
  } 

  wait(NULL);

}

int cd() {
  string path;
  if(command.size()<2) {
    path.append(homeDir);
  }

  else {
    path.append(currentPath);
    path.append("/");
    path.append(command[1]);
  }

  int s = chdir(path.c_str());

  currentPath = fs::current_path();
  if(currentPath.substr(0, homeDir.size()) == homeDir){
    v_currentPath = "";
    v_currentPath.append("~");
    v_currentPath.append(currentPath.substr(homeDir.size(), currentPath.size()-homeDir.size()));

  }
  else
    v_currentPath = currentPath;

  if(s<0)
    return 0;

  return 1;
}

map<string, function<int()> > shellMap = {
  {"cd", cd}
};

int checkShell() {
  if(shellMap.find(command[0]) != shellMap.end()) {  
    shellMap["cd"]();
    return 1;
  }
  return 0;
}


const string currentTime() {
  time_t     now = time(0);
  struct tm  tstruct;
  char       buf[80];
  tstruct = *localtime(&now);

  strftime(buf, sizeof(buf), "%X", &tstruct);

  return buf;
}

int main(void) {
  currentPath = fs::current_path();


  struct passwd *pw = getpwuid(getuid());

  homeDir = pw->pw_dir;
  string userName = pw->pw_name;

  char hostName[253];
  gethostname(hostName, 253);

  currentPath = homeDir;


  int ok=1;
  while(1) {
    command.clear();
    cout<<"\033[1;32m"<<v_currentPath<<"\033[0m ";
    cout<<"\033[1;34m"<<currentTime()<<"\033[0m ";
    cout<<"\033[1;33m"<<userName<<"@"<<hostName<<"\033[0m"<<endl;
    cout<<"$ ";

    string buf;

    int clr = readCommands();
    if(!clr)
      goto done; 


    if(history.size()==0 || history[history.size()-1] != command) {
      history.push_back(command);
    }

    if(checkShell())
      goto done;

    for(int i=0; i<pathsSize; i++) {
      string buf = paths[i];
      buf.append(command[0]);


      if(fs::exists(buf)) {
        command[0] = buf;
        goto cont; 
      }
    }
    cout<<"fsh: command not found: "<<command[0]<<endl;
    goto done;

cont:
    execute();

done:
    cout<<endl;
    ok=0;
  }

exit:
  return 0;
}
