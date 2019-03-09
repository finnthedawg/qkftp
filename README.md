# 🗄️ qkftp 🗄️
A implementation of the File Transfer Protocol in C

**Collaborators:**
* <a href="https://github.com/finnthedog223">finnthedog223</a>
* <a href="https://github.com/kokostos1673">kokostos1673</a>

---

To run the project with default `localhost` and port `9999`
```
git clone
make client
make server
```

To change the port and host:
```
//Server side, set
int port = [portNumber];
./server

//Client side
./client [hostname] [port]
```


## Features:

* Multiple users and concurrent file Upload/Download
* User authentication and password
* Multiple working directories
* `USER username`
* `PASS password` Entered by the client to login.
* `PUT [filename]` Uploads a file to the server.
* `GET [filename]` Downloads a file from the server.
* `PWD` Prints the working direcotry of the client on the server
* `LS` List all the files under the current server directory
* `CD` Change the directory of the client on the server
* `![CD | LS | PWD]` Calls these commands locally on the client.

## Testing:

---------------

To test the client and the server, you can use netcat. You can also simply run the server and then then client.

`nc -l 127.0.0.1 9999` To test the client, simulating server

`nc 127.0.0.1 9999` To test the simulating client.

-------------

### Test cases

* Incorrect command
  * Input `UESR name`
  * Expected output: `Invalid FTP command entered`
  * Output `Invalid FTP command entered`

* Authenticating a user.
  * Expected: Authentication complete.
  * Result:
    ```
    ftp >> USER koko
    Username OK, password required
    ftp >> PASS 123
    Authentication complete
    ```

* Authenticating multiple users at the same time.
  * Expected: Authentication completed
  * Output: Multiple users can be authenticated.

* Downloading a file from server `GET`.
  * Goal: Move `ts.txt` from server to client
  * Expected: File successfully moved.
  * Output: Valid
    ```
    ftp >> !LS //Client files
    tc.txt
    ftp >> LS //Server files
    ts.txt

    ftp >> GET ts.txt //Get the file from server
    ftp >> !LS //Client files
    tc.txt	ts.txt
    ```

* Uploading a file to server `PUT`.
  * Goal: Upload `tc.txt` from client to server
  * Expected: File successfully moved.
  * Output: Valid
    ```
    ftp >> !LS //Client files
    tc.txt

    ftp >> LS //Server files
    ts.txt

    ftp >> PUT tc.txt //Upload tc.txt
    ftp >> LS //Server files
    tc.txt
    ts.txt
    ```

* `!PWD | PWD` Print working directory, on client and server.
  * Expected: Directory on server or client.
  * `!PWD` Output: `/Users/Jdawg223/Documents/Networking/qkftp`
  * `PWD` Output: `/Users/Jdawg223/Documents/Networking/qkftp/test_server`

* `CD | !CD` change directory, on client and server.
  * Expected: changed directory.
  * Result: Sucessfully changes client and server directories.
  * Sucessfully tested with two clients and seperate working directories:

* `LS | !LS` change directory, on client and server.
  * Expected: Listed directory
  * Result:
    ```
    ftp >> LS
    FTPserver.c
    README.md
    client
    client.c
    makefile
    server
    test_client
    test_server
    users.txt
    ```

* Executing commands before authentication
  * Expected: Do not execute commands
  * Result:
    ```
    ftp >> LS
    Server requires you to be authenticated
    ```
