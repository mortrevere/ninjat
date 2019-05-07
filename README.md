# ninjat

ninjat is a single-file, client-free chat server.

To compile and launch the server : 

```
g++ main.cpp -pthread -std=c++11 -o ninjat-serv
./ninjat-serv
```

Clients can join the chat using any TCP client like `netcat` or `telnet`.

```
nc <server_ip_address> <port>
```

Default port is 1337.

## Usage

Available commands are : 

```
/nick            : set your name and talk to people (auth).
/list            : list people on the chat
/p <dest> <msg>  : pm someone based on nickname
/p . <msg>       : pm the last person you pm'd
/auth <password> : input a password to authorize your connection
```

## Configuration

All configuration parameters are contained in `#define` statements at the top of the file.

```cpp
#define PORT 1337
#define PRIVATE false
#define PASSWORD "chaussette"
#define MAX_ATTEMPT 3
```

## TODO

- Custom port
- Custom motd (started)
- ~~Password protection~~ **DONE**
- Nick reservation 
