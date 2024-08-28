#include <bits/stdc++.h>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <csignal>
#include <termios.h>

#define int long long
using namespace std;
using namespace filesystem;

const char* exec(string s){
    const char* command = s.c_str();
    return command;
}

void disableEcho() {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);        
    termios newt = oldt;                
    newt.c_lflag &= ~ECHO;                  
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); 
}

void enableEcho() {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);        
    oldt.c_lflag |= ECHO;               
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 
}


string getExec(string comm){
    char buffer[128];
    string result = "";

    FILE* pipe = popen(comm.c_str(), "r");
    while(!feof(pipe)){
        if(fgets(buffer, 128, pipe) != NULL){
            result += buffer;
        }
    }
    pclose(pipe);
    return result;
}

pid_t strToPid(const string& s) {
    char* end;
    errno = 0;
    long pid_long = strtol(s.c_str(), &end, 10);

    if (*end != '\0' || end == s.c_str() || errno == ERANGE || pid_long < 0) {
        std::cerr << "Error: Invalid PID string." << std::endl;
        return -1;
    }

    return static_cast<pid_t>(pid_long);
}

void sendSignals(string s){
    s += 'x';
    string cobj = "";
    for(auto i : s){
        if(i == 'x') break;
        if(isdigit(i)) cobj += i;
        else{
            if (!cobj.empty()) {
                string command = "kill -10 " + cobj;
                system(command.c_str());
                cobj = "";
            }
        }
    }
}


void help(){
    cout<<"Neolock is an open source lockscreen made for Arch Hyprland. It is highly advised not to use it outside of Arch Hyprland\nFlags:\n   -h or --help: displays this\n   -c or --config: requires a password after it (in quotation marks), sets that as the phrase which exits out of the program\n    no flags: launches the lock normally";
}

const int mod = 1e9+7;
int sum(int a,int b){return (a%mod + b%mod) %mod;}
int mult(int a,int b){return (a%mod * b%mod) %mod;}

int hash1(string s){
    int p = 29;
    int sl = s.length();
    int r = 0;
    for(int i=0;i<sl;i++){
        r=sum(r,mult((s[i]+1),p));
        p=mult(p,29);
    }
    return r;
}
int hash2(string s){
    int p = 31;
    int sl = s.length();
    int r = 0;
    for(int i=0;i<sl;i++){
        r=sum(r,mult((s[i]+1),p));
        p=mult(p,31);
    }
    return r;
}
pair<int,int>hashf(string s){
    return {hash1(s), hash2(s)};
}

bool hasFragment(string s, string f){
    return s.find(f) != string::npos;
}

bool fileHasFragment(string s, string f){
    bool has = false;
    ifstream inf(s);
    string cobj="";
    while(getline(inf,cobj)){
        if(hasFragment(cobj, f)){
            has=true;
            return true;
        }
    }
    return false;
}

int32_t main(int32_t argc, char* argv[]){

// checking flags
    bool flagNone = false;
    
    if(argc == 1) flagNone = true;
    if(!flagNone){
        string flag = argv[1];
        if(flag == "-c" || flag == "--config"){
            string pass="";
            if(argc > 2) pass = argv[2];
            else{
                cout<<"Config flag requires password";
                return 0;
            }
            string path = string(getenv("HOME")) + "/.config/neolock";
            if(!exists(path)) system(exec("mkdir "+path));
            path += "/neolock.conf";
        
            ofstream outf(path);
            pair<int,int>hash = hashf(pass);
            outf<<hash.first<<'\n'<<hash.second;
            outf.close();

            string kittypath = string(getenv("HOME")) + "/.config/kitty/kitty.conf";
            if(!fileHasFragment(kittypath, "allow_remote_access")){
                ifstream inf(kittypath);
                stringstream nc;
                string line = "";
                while(getline(inf,line)){
                    nc << line << '\n';
                }
                nc << "allow_remote_access  yes";
                inf.close();
                ofstream outf(kittypath);
                outf << nc.str();
                outf.close();
            }
            

            return 0;
        }
        else if(flag == "-h" || flag == "--help"){
            help();
            return 0;
        }
        else{
            cout<<"Incorrect flag syntax, try -h or --help";
            return 0;
        }
    }

// do this if launching without flags
    bool isPasswordSet = false;
    string path = string(getenv("HOME")) + "/.config/neolock/neolock.conf";
    if(exists(path)) isPasswordSet = true;
    else{
        cout<<"No password set! Try -h or --help";
        return 0;
    }

// move to workspace 11
    system(exec("hyprctl dispatch movetoworkspace 11"));

// disable ctrl+c in kitty
    string kittypath = string(getenv("HOME")) + "/.config/kitty/kitty.conf";
    ifstream inf(kittypath);
    string line = "";
    bool lineExists = false;
    stringstream newContentT;
    while(getline(inf,line)){
        if(line == "#map ctrl+c discard_event" || line == "map ctrl+c discard_event"){
            lineExists = true;
        }
        newContentT << line << '\n';
    }
    inf.close();
    ofstream outf(kittypath);
    outf << newContentT.str();
    outf.close();


    ifstream a2inf(kittypath);
    stringstream newContent;
    if(lineExists){
        while(getline(a2inf,line)){
            if(line == "#map ctrl+c discard_event"){
                newContent << "map ctrl+c discard_event" << '\n';
            } else {
                newContent << line << '\n';
            }
        }
    } else {
        while(getline(a2inf,line)){
            newContent << line << '\n';
        }
        newContent << "map ctrl+c discard_event" << '\n';
    }
    a2inf.close();
    ofstream a2outf(kittypath);
    a2outf << newContent.str();
    a2outf.close();

// reload kitty config (send signal)
    string pids = getExec("pgrep -x kitty");
    sendSignals(pids);

// disable workspace switching and SUPER_V
    string hyperconfpath = string(getenv("HOME")) + "/.config/hypr/hyprland.conf";
    ifstream hci(hyperconfpath);
    stringstream nhc;
    line = "";
    while(getline(hci,line)){
        if(hasFragment(line, "workspace")){
            nhc << '#' << line << '\n';
        } else {
            nhc << line << '\n';
        }
    }
    hci.close();
    ofstream hco(hyperconfpath);
    hco << nhc.str();
    hco.close();


// grab password hashes from config file (also cout neofetch)
    system(exec("clear"));
    cout<<'\n';
    system(exec("neofetch"));


    pair<int,int>setpass={1,1};
    ifstream inf2(path);
    int counter = 1;
    line = "";
    while(getline(inf2,line)){
        if(counter==1) setpass.first = stoll(line);
        if(counter==2) setpass.second = stoll(line);
        counter++;
    }
    inf2.close();
    
    pair<int,int>input = {0,0};
    
    disableEcho();
    while(input != setpass){
        const char* user = getenv("USER");
        cout<<"Password for user "<<user<<": ";

        string inpass;
        cin>>inpass;
        input = hashf(inpass);
        if(input != setpass)cout<<"\nWrong password, try again.\n";
    }
    enableEcho();

// enable ctrl+c
    string filename = string(getenv("HOME")) + "/.config/kitty/kitty.conf";
    ifstream inputFile(filename);
    line = "";
    stringstream modifiedContent;

    while (getline(inputFile, line)) {
        if (line.find("ctrl+c") != string::npos) {
            if(line[0] != '#') modifiedContent << '#';
            modifiedContent << line << "\n";
        } else {
            modifiedContent << line << "\n";
        }
    }
    inputFile.close();
    ofstream outputFile(filename);
    outputFile << modifiedContent.str();
    outputFile.close();
    
    pids = getExec("pgrep -x kitty");
    sendSignals(pids);

// enable workspace switching and SUPER_V
    ifstream hci2(hyperconfpath);
    stringstream nhc2;
    line = "";
    while(getline(hci2,line)){
        if(hasFragment(line, "workspace") && line[0] == '#'){
            for(int i=1;i<line.length();i++){
                nhc2 << line[i];
            }
            nhc2 << '\n';
        } else {
            nhc2 << line << '\n';
        }
    }
    hci2.close();
    ofstream hco2(hyperconfpath);
    hco2 << nhc2.str();
    hco2.close();

// kill the terminal:
    system(exec("kitty @ close-window"));

}
