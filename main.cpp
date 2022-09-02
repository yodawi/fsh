#include <iostream>
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

const int pathsSize = 3;
string paths[pathsSize] = {"./", "/bin/", "/usr/bin/"};
string currentPath;
string v_currentPath = "~";
string homeDir;

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

int jump = 0;

void signitHandler(int s) {
  jump = 1;
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

  signal(SIGINT, &signitHandler);

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
