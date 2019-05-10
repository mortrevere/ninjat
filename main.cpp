#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> /* close */
#include <thread>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

#define PORT 1337
#define PRIVATE false
#define PASSWORD "chaussette"
#define MAX_ATTEMPT 3
#define MOTD_PATH "./motd"

int init(const std::string& version) {
  std::cout << "ninjat server - v" << version;
#ifdef WIN32
  WSADATA WSAData;
  std::cout << " (Win32)" << std::endl;
  return WSAStartup(MAKEWORD(2,2), &WSAData);
#else
  std::cout << " (GNU/Linux)" << std::endl;
  return 0;
#endif
}

void die(const std::string& err) {
  std::cout << "[FATAL ERROR] : " << err << std::endl;
#ifdef WIN32
  WSACleanup();
#endif
  std::exit(EXIT_FAILURE);
}

void info(const std::string& info) {
  std::cout << "[INFO] : " << info << std::endl;
}

void err(const std::string& err) {
  std::cout << "[ERR] : " << err << std::endl;
}

std::string fileRead(const std::string& path) {
  std::ifstream infile(path, std::ios::in);
  std::string out("");
  char chr;
  if(infile){while(infile.get(chr)){out += chr;}return out;}else {return "";}
}

bool fileExists(const std::string& path) {
  std::ifstream infile(path);
  return infile.good();
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

bool s(std::string toSend, int sock, bool format = true) {
  if(sock <= 0)
    return false;
  if(format)
    toSend = "\t\t\t\t\t\t" + toSend;

  int error = send(sock,toSend.c_str(),toSend.size(),0);
  return !(error == SOCKET_ERROR);
}

void sAll(const std::string& in, std::vector<int>& clients) {
  std::cout << "[SENT:*] : " << in << std::endl;
  for (std::size_t i = 0; i < clients.size(); i++) {
    s(in, clients[i]);
  }
}

std::string buildList(std::vector<std::string>& nicks) {
  std::string list = std::to_string(nicks.size()) + " users :";
  for(unsigned int i = 0; i < nicks.size(); i++)
    list += " " + (nicks[i].length() ? nicks[i] : "<no_nickname_yet>");
  return list;
}


bool setNick(int csock, std::vector<int>& clients, std::vector<std::string>& nicks, std::string nick) {
  for(unsigned int i = 0; i < clients.size(); i++) {
    if(clients[i] == csock) {
      nicks[i] = nick;
      return true;
    }
  }
  return false;
}

int getIndex(int csock, std::vector<int>& clients) {
  for (std::size_t i = 0; i < clients.size(); i++) {
    if(clients[i] == csock) {
      return i;
    }
  }
  return -1;
}

int getIndexByNick(std::string nick, std::vector<std::string>& nicks) {
  for (std::size_t i = 0; i < nicks.size(); i++) {
    if(nicks[i] == nick) {
      return i;
    }
  }
  return -1;
}

int clientCare(int csock, std::vector<int>& clients, std::vector<std::string>& nicks) {
  char buffer[65535];
  std::string in, nick, lastPrivate, to, password;
  int error = 0, attempt = 0; //will be used to handle all networking errors
  bool authenticated = !PRIVATE;
  info("Taken care of client !");
  if(csock != INVALID_SOCKET) {

    if(fileExists(MOTD_PATH)) {
      std::string motd = "\n" + fileRead(MOTD_PATH);
      size_t pos = 0;
      while((pos = motd.find("\n", pos)) != std::string::npos) {
        motd.replace(pos, 1, "\n\t\t\t\t\t\t");
        pos += 7;
      }
      s(motd + "\n", csock);
    }

    while(1) {
      info("Waiting for data ...");
      error = recv(csock,buffer,65535,0);
      if (error <= 0) {
        info("Peer disconnected.");
        nicks.erase(nicks.begin() + getIndex(csock, clients));
        clients.erase(clients.begin() + getIndex(csock, clients));
        closesocket(csock);
        if(nick != "")
          sAll(" *** [QUIT] : " + nick + "\n", clients);
        return 0;
      }
      else {
        buffer[error] = '\0';
        in = buffer;

        if(in.substr(0,5) == "/quit")
        {
          clients.erase(clients.begin() + getIndex(csock, clients));
          closesocket(csock);
          return 0;
        }

        if(!authenticated) {
          if(in.substr(0,6) == "/auth ") {
            password = in.substr(6);
            password = password.substr(0,password.length() - 1);
            if(password == PASSWORD) {
              authenticated = true;
              s("OK. Use /nick to get a nickname and talk now.\n", csock);
            } else {
              attempt++;
              s("WRONG PASSWORD. (" + std::to_string(attempt) + "/" + std::to_string(MAX_ATTEMPT)  + ")\n", csock);
              if(attempt == MAX_ATTEMPT) {
                clients.erase(clients.begin() + getIndex(csock, clients));
                closesocket(csock);
                return 0;
              }
            }
          } else {
            s("THIS SERVER IS PASSWORD-PROTECTED : use /auth <pw> to log in\n", csock);
          }
        } else {
          if(in.substr(0,6) == "/nick ") {
            nick = in.substr(6);
            nick = nick.substr(0,nick.length() - 1);
            if(nick.length() > 16) {
              s(" *** [ERROR] : Nicknames are 16-char-long at most.\n", csock);
            } else {
              setNick(csock,clients,nicks,nick);
              sAll(" *** [INFO] changed nick to : <" + nick + ">\n", clients);
              s(buildList(nicks) + "\n", csock);
            }
          }
          else if(in.substr(0,5) == "/list") {
            s(buildList(nicks) + "\n", csock);
          }
          else if(in.substr(0,3) == "/p ") {
            to = in.substr(3,in.substr(3).find(" "));

            if(to == ".") {
              if(lastPrivate != "") {
                s(" *** [PRIVATE] " + nick + " : " + in.substr(4 + in.substr(3).find(" ")), clients[getIndexByNick(lastPrivate, nicks)]);
                s(" >>> [PRIVATE] " + nick + " : " + in.substr(4 + in.substr(3).find(" ")), csock);
              }
              else {
                s(" >>> [PRIVATE] [ERR] SERVER : How can you use /p . <message> if you haven't spoke to anyone before ? Dumbass.\n", csock);
              }
            }
            else {
              if(to != nick) {
                if(getIndexByNick(to, nicks) != -1) {
                  lastPrivate = to;
                  s(" *** [PRIVATE] " + nick + " : " + in.substr(4 + in.substr(3).find(" ")), clients[getIndexByNick(to, nicks)]);
                  s(" >>> [PRIVATE] " + nick + " : " + in.substr(4 + in.substr(3).find(" ")), csock);
                }
                else {
                  s(" >>> [PRIVATE] [ERR] SERVER : It seems like " + to + " is not connected. Sorry.\n", csock);
                }
              }
              else {
                s(" >>> [PRIVATE] [ERR] SERVER : DO NOT TALK TO YOURSELF.\n", csock);
              }
            }
          }
          else if(in != "\n") {
            if(nick != "")
              sAll("<" + nick + "> " + in, clients);
            else
              s(" *** [ERROR] : Please identify first, using /nick <nickname>\n", csock);
          }
        }
        std::cout << " *** [RECV] : " << in;
      }
    }
  }
  return 0;
}

int main() {
  int error = init("0.0.2"); //will be used to handle all networking errors
  int sock;
  int port = PORT;
  struct sockaddr_in sin;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  std::string tmp, key, value;
  tmp = std::to_string(port);

  int csock;
  struct sockaddr_in csin;
  socklen_t tempo = sizeof(csin);

  std::vector<std::string> lines;
  std::ofstream fout;

  std::vector<std::thread> threads;
  std::vector<int> clients;
  std::vector<std::string> nicks;

  if(error != 0)
    die("Couldn't initialize network.");

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == INVALID_SOCKET)
    die("Couldn't create socket.");
  else
    info("Main socket created.");

  while(bind(sock, (sockaddr *)&sin, sizeof(sin)) != 0) {
    err("Can't listen to port " + std::to_string(port) + " ... check if the port is busy.");
    port++;
    info("Retrying on port " + std::to_string(port) + " ...");
    sin.sin_port = htons(port);
  }
  info("bind() successful on port " + std::to_string(port));

  listen(sock, 99);
  info("Listening on port " + std::to_string(port));

  while(1) {
    info("Waiting for peers ...");
    csock = accept(sock,(struct sockaddr*)&csin,&tempo);
    clients.push_back(csock);
    nicks.push_back("");
    threads.push_back(std::thread(clientCare,csock,std::ref(clients),std::ref(nicks)));
    sAll("Someone (#" + std::to_string(csock) + ") joined !\n", clients);

    s("/nick               : get a name and talk to people.\n", csock);
    s("/list               : list people on the chat\n", csock);
    s("/p <dest> <message> : pm someone based on nickname\n", csock);
    s("/p . <message>      : pm the last person you pm'd\n", csock);

    if(PRIVATE) {
      s("THIS SERVER IS PASSWORD-PROTECTED : use /auth <pw> to log in\n", csock);
    }
    info("NCC !");
  }

  for (auto& th : threads) th.join();
  info("End.");
  return 0;
}
