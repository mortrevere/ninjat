# ninjat

ninjat is a single-file, client-free chat server.

To compile and launch the server : 

```
make
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
/auth <password> : input a password to authorize your connection (for private servers ony)
/quit            : disconnect from and leave the chat
```

## Configuration

All configuration parameters are contained in `#define` statements at the top of the file.

```cpp
#define PORT 1337
#define PRIVATE false
#define PASSWORD "chaussette"
#define MAX_ATTEMPT 3
#define MOTD_PATH "./motd"
```

If the file containing the motd for the server is missing, it will simply skip that step.

## TODO

- ~~Custom port~~ todo on CLI
- ~~Custom motd~~ **DONE**
- ~~Password protection~~ **DONE**
- nick reservation 
