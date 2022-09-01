#include <iostream>
#include <functional>
#include <map>
#include <filesystem>
#include <unistd.h>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <signal.h>

namespace fs = std::filesystem;
using namespace std;

typedef int (*FnPtr)(int, int);

vector<string> command;

const int pathsSize = 3;
const string homeDir = "/home/fox";
string paths[pathsSize] = {"./", "/bin/", "/usr/bin/"};
string currentPath = homeDir;

void readCommands() {
  string inp;
  getline(cin, inp);

  if(cin.eof()) {
    cout<<endl;
    exit(0);
  }

  int pi=0;
  for(int i=0; i<inp.size(); i++) {
    if(inp[i]==' ') {
      command.push_back(inp.substr(pi, i-pi));
      pi = i+1;
    }
  }
  command.push_back(inp.substr(pi, inp.size()-pi));
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

int jump = 0;

void signitHandler(int s) {
  jump = 1;
}

int main(void) {
  currentPath = fs::current_path();

  signal(SIGINT, &signitHandler);

  int ok=1;
  while(1) {
    command.clear();
    cout<<currentPath<<" SNOWDEN"<<endl<<"$ ";
    readCommands();
    string buf;

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
