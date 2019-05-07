
# ninjat

ninjat is a single-file, client-free chat server.

To compile and launch the server : 

```
g++ main.cpp -pthread -std=c++11 -o ninjat-serv
./ninjat-serv
```

Clients can join any TCP client like `netcat` or `telnet`.

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

## TODO

- Custom port
- Custom motd (started)
- ~~Password protection~~ **DONE**
- nick reservation 
