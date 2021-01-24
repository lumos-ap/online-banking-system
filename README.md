# online-banking-system
### Title: 
Design and Development of online banking management system
### Description:
The project aims to develop a banking system that is user-friendly and multifunctional. The project should have the following functionalities:

a. All account transactional details and customer information are stored in files.

b. Account holders have to pass through a login system to enter their accounts.

c. The application should possess password-protected administrative access; thus
preventing the whole management system from unauthorized access.

d. Three types of login should be created: normal user, joint account user; administrator;

e. Once you login as administrator, you should be able to add, delete, modify, search for a
specific account details.

f. Once the customer connect to the server, you should get login and password prompt.

After successful login, you should get menu for example:
Do you want to:
* Deposit
* Withdraw
* Balance Enquiry
* Password Change
* View details
* Exit

g. If you login as a joint account holder, proper file locking should be implemented. If you
want to view the account details then read lock is to be set else if you want to withdraw
or deposit then write lock should be used to protect the critical data section.

h. Use socket programming – Server maintains the data base and service multiple clients
concurrently. Client program can connect to the server and access their specific account
details.

i. Use system calls instead of Library functions wherever it is possible in the project:
Process Management, File Management, File Locking, Multithreading and Inter Process
Communication Mechanisms. 

### Instrustions to compile:
Instructions to compile:
1. Compile the Server.c file
2. Compile the Client.c file

To log in the system, the initial credentials are:

username :admin

password :admin
