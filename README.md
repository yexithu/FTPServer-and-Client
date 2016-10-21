#FTP Server and Client 

Simple FTP Server and Client implemented using C.  
Assignment of Architecture of Computer and Network(1). 

##Server
###Accepted Arguements
Run server using  
`sudo ./server [-port PORT] [-root ROOT]`  
If you want, you can specify the port and root for server using the optional [-port] and [-root] arguements. And the defualt setting is
  
* port: 21 
* root: /tmp

###Supported Verbs
**USER & PASS:** Use USER [username] and PASS [password] to log in the FTP server. USER anonymous allows guests loging in. User verification is implemented and user-table is stored in `server/src/user.config`.   
**PORT & PASV:** PORT [IP and PORT] or PASV are used to set the mode of FTP server. If PORT is set, the client opens a socket and waits the server to connect. If PASV is set, the server sends IP and PORT to the client and waits for the client.  PORT and PASV is one-time used.  
**RETR:** RETR [file] retrieval [file] from FTP server.    
**STOR:** STOR [file] upload [file] to FTP server.  
**SYST:** on receiving a SYST command, return the string “215 UNIX Type: L8” to the client.  
**TYPE:** on receiving a “TYPE I” command, return “200 Type set to I.” to the client. On receiving some other TPYE command, return an appropriate error code.   
**CWD:** CWD [path] asks the server to set the name prefix to this pathname, or to another pathname that will have the same effect as this pathname if the filesystem does not change.  
**CDUP:** CDUP asks the server to set the name prefix to upper directory.  
**DELE:** DELE [file] asks server to delete the [file].   
**LIST:** LIST [path/file] asks server to list the file or directories in target path or show information of file. If empty paremeters are passed, the server show the list of the current name prefix. Usually, there could be a number of files or directories in a directory. So, the server use a file-transfer style to response to the LIST verb. And before List, PORT or PASV should be set.
**MKD:** MDK [dir] create a new directory of server.  
**PWD:** PWD show the name prefix of the server.  
**RMD:** RMD [dir] removes an empty directory of server. Notice that [dir] must be an empty directory.  
**RNFR & RNTO:** RNFR [src] and RNTO [dst] should be used as a union.  

##Client
###Accepted Arguements
Run server using  
`sudo ./server [-ip IP] [-port PORT]`  
If you want, you can specify the host ip and host port for client using the optional [-ip] and [-port] arguements. And the defualt setting is  
   
* ip:127.0.0.1  
* port: 21

###Surported Commands
After sending your username and password, you enter the FTP Command Prompt, where the following commands are suported. Use **help** command to view all the supported commands. Here we only select some important commands.  
**help:** show the supported commands.  
**upload & download:** upload [src] [dst] upload the [src] file to the server and save the file with name [dst] (example: `upload a.dat b.dat`). 
download [src] [dst] download [src] file from server and save the file with name [dst] (example: `download b.dat a.dat`)  
**setport & setpasv:** set the transfer style to PASV & PORT.  
**list:** list [dir] display the file lists of server.  
**others:** please refer to the HELP command  
