#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <fcntl.h>
#include <time.h>
#include "structures.c" 
#define PORT 5000


void sendReadToClient(int desc, char *msg){
	struct message communication;
	strcpy(communication.buffer, msg);
	communication.read = 0;
	write(desc, &communication, sizeof(communication));
	
}


void sendCloseToClient(int desc, char *msg){
	struct message communication;
	strcpy(communication.buffer, msg);
	communication.read = 2;
	write(desc, &communication, sizeof(communication));
}

void sendWriteToClient(int desc, char *msg, char *output){
	struct message communication;
	strcpy(communication.buffer, msg);
	communication.read = 1;
	write(desc, &communication, sizeof(communication));
	read(desc, output, 1024);
}

void initialSetup(){
	int fd = open("customer_db", O_CREAT|O_EXCL|O_RDWR, 0755);
	if(fd!=-1){
		struct Customer admin;
		strcpy(admin.username, "admin");
		strcpy(admin.password, "admin");
		admin.del_flag = 0;
		admin.type = 1;
		admin.account_number=5000;
		write(fd, &admin, sizeof(admin));
	}
	close(fd);
	
	int account_count=5000;
	fd=open("account_db", O_CREAT|O_EXCL|O_RDWR, 0755);
	if(fd!=-1){
		struct Account account_admin;
		account_admin.account_number=account_count;
		account_admin.balance=0;
		account_admin.del_flag=0;

	write(fd,&account_count,sizeof(account_count));
	write(fd,&account_admin,sizeof(account_admin));

	close(fd);

	}
	open("transaction_db", O_CREAT|O_EXCL, 0755);
}
int check_unique_user(char* uname)
{
	int fd=open("customer_db",O_RDONLY);
	struct Customer admin;
	struct flock lock;
	lock.l_type=F_RDLCK; 		
  	lock.l_whence=SEEK_SET; 
  	lock.l_start=0; 			
  	lock.l_len=0; 			
  	lock.l_pid=getpid();
  	 	
  	fcntl(fd, F_SETLKW, &lock);
	while(read(fd,&admin,sizeof(admin)))
	{
		if(strcmp(uname,admin.username)==0 && admin.del_flag==0)
		{
			close(fd);
			return 1;
		}
	}
	
	lock.l_type=F_UNLCK;
  	fcntl(fd,F_SETLK,&lock);
	close(fd);
	return 0;
}
void add_JointAccount(int socket_desc,char *username, char *password, char *username2, char *password2)
{

    struct Customer cust;
    struct Customer cust2;
    struct Account acc;
    struct flock lock;
    
    //char temp[1024];

    int fd = open("account_db", O_RDWR);
    int count;
    
    lock.l_type=F_WRLCK; 		
    lock.l_whence=SEEK_SET; 	
    lock.l_start=0; 			
    lock.l_len=sizeof(int); 			
    lock.l_pid=getpid(); 
    fcntl(fd, F_SETLKW, &lock);
    
    //sendWriteToClient(socket_desc,"Checking the lock mechanism",temp);

    read(fd, &count, sizeof(count));
    count++;
    lseek(fd, 0, SEEK_SET);
    write(fd, &count, sizeof(count));
    
    lock.l_type=F_UNLCK;
    fcntl(fd,F_SETLK,&lock);
 	
    lock.l_type=F_WRLCK; 		
    lock.l_whence=SEEK_END;
    lock.l_len=sizeof(acc); 			
    fcntl(fd, F_SETLKW, &lock);
    
    lseek(fd, 0, SEEK_END);
    acc.account_number = count;
    acc.balance = 0;
    acc.del_flag = 0;
    write(fd, &acc, sizeof(acc));
    
    lock.l_type=F_UNLCK;
    fcntl(fd,F_SETLK,&lock);

    close(fd);
    
    fd = open("customer_db", O_WRONLY | O_APPEND);

    strcpy(cust.password, password);
    strcpy(cust.username, username);
    cust.type = 3;
    cust.del_flag = 0;
    cust.account_number = count;
    
    lock.l_type=F_WRLCK; 		
    lock.l_whence=SEEK_END;
    lock.l_len=sizeof(cust)*2; 			
    fcntl(fd, F_SETLKW, &lock);
    
    write(fd, &cust, sizeof(cust));

    strcpy(cust2.password, password2);
    strcpy(cust2.username, username2);
    cust2.type = 3;
    cust2.del_flag = 0;
    cust2.account_number = count;
    write(fd, &cust2, sizeof(cust2));
    
    lock.l_type=F_UNLCK;
    fcntl(fd,F_SETLK,&lock);
    close(fd);
    
    sendReadToClient(socket_desc, "Users Added Succesfully\n");
}

void add_account(int socket_desc){
	
	char username[1024], password[1024];
	
	char type[1024]; 
	char msg_username[1024] = "Enter username: ";
	char msg_password[1024] = "Enter password: ";
	char msg_type[1024]="Enter type\n\tPress 1 for Admin\n\tPress 2 for Normal User\n\tPress 3 for Joint  User\n";
	
	sendWriteToClient(socket_desc, msg_username, username);
	sendWriteToClient(socket_desc, msg_password, password);
	
	if(check_unique_user(username)==1){	
		sendReadToClient(socket_desc,"\nError: Username is not unique.\n");
		return;
	}
	
	sendWriteToClient(socket_desc,msg_type,type);
	int int_type=atoi(type);
	
	
	if(int_type==3)//joint user
	{
	   char username2[1024], password2[1024];
        char msg_username2[1024] = "Enter second username: ";
        char msg_password2[1024] = "Enter second password: ";
        sendWriteToClient(socket_desc, msg_username2, username2);
        sendWriteToClient(socket_desc, msg_password2, password2);
        if (check_unique_user(username2) == 1)
        {
            sendReadToClient(socket_desc, "\nError: Username2 is not unique\n");
            return;
        }
        else if(strcmp(username,username2)==0)
        {
        	sendReadToClient(socket_desc, "\nError: Username and Username 2 can not be same\n");
        	return;
        }
        else
        {
            add_JointAccount(socket_desc,username, password, username2, password2);
        }
	
	return;
	}
	
	struct Customer cust;
	struct Account acc;
	struct flock lock;

	int fd=open("account_db", O_RDWR);
	int count;
	//char temp[1024];
	
	lock.l_type=F_WRLCK; 		
  	lock.l_whence=SEEK_SET; 	
  	lock.l_start=0; 			
  	lock.l_len=sizeof(int); 			
  	lock.l_pid=getpid(); 
  	fcntl(fd, F_SETLKW, &lock);
	//sendWriteToClient(socket_desc,"Checking the mechanism",temp);
	read(fd,&count,sizeof(count));
	count++;
	lseek(fd,0,SEEK_SET);
	write(fd,&count,sizeof(count));

	lock.l_type=F_UNLCK;
 	fcntl(fd,F_SETLK,&lock);
 	
 	lock.l_type=F_WRLCK; 		
  	lock.l_whence=SEEK_END;
  	lock.l_len=sizeof(acc); 			
  	fcntl(fd, F_SETLKW, &lock);
  
	lseek(fd,0,SEEK_END);
	acc.account_number=count;
	acc.balance=0;
	acc.del_flag=0;
	write(fd,&acc,sizeof(acc));
	
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
	
	fd=open("customer_db", O_WRONLY|O_APPEND);

	strcpy(cust.password,password);
	strcpy(cust.username,username);
	
	int intType=atoi(type); 
	cust.type=intType;
	cust.del_flag=0;
	cust.account_number=count;
	
	lock.l_type=F_WRLCK; 		
  	lock.l_whence=SEEK_END;
  	lock.l_len=sizeof(cust); 			
  	fcntl(fd, F_SETLKW, &lock);
	
	write(fd,&cust,sizeof(cust));
	close(fd);
	
	lock.l_type=F_UNLCK;
 	fcntl(fd,F_SETLK,&lock);
	
	sendReadToClient(socket_desc,"User Added Succesfully\n");
	
}
int delete_account(int socket_desc)
{
	int fd,fd2, found=0;
	
	struct Customer cust;
	struct Account ac;
	struct flock lock;
	char confirm[1024];
	char accountno[1024];
	char sbalance[1024];
	
	sendWriteToClient(socket_desc,"Enter Account number:",accountno);
	int accountnumber=atoi(accountno);
	
	fd2=open("account_db",O_RDWR);
	lseek(fd2,sizeof(int),SEEK_SET);
	
	lock.l_type=F_WRLCK; 		
  	lock.l_whence=SEEK_SET; 	
  	lock.l_start=sizeof(int); 			
  	lock.l_len=0; 			
  	lock.l_pid=getpid(); 
  	fcntl(fd2, F_SETLKW, &lock);
  	
	while(read(fd2,&ac,sizeof(ac)))
	{
		if(found==0 && accountnumber==ac.account_number &&ac.del_flag==0 )
		{
			found=1;
			if(ac.balance>0)
			{
			
				sendReadToClient(socket_desc,"\nBalance Remaining: ");
				sprintf(sbalance,"%.2f",ac.balance);
				sendReadToClient(socket_desc,sbalance);
				sendWriteToClient(socket_desc,"\nAre you sure you want to delete account?(Y/N):",confirm);
				if(confirm[0]=='Y'||confirm[0]=='y'){	
					sendReadToClient(socket_desc,"Deleting account\n");
				}
				else if (confirm[0]=='N'||confirm[0]=='n'){
					sendReadToClient(socket_desc,"Stopping delete operation\n");
					lock.l_type=F_UNLCK;
					fcntl(fd2,F_SETLK,&lock);		
					return -1;			
				}
			}
			ac.del_flag=1;
			lseek(fd2,-sizeof(ac),SEEK_CUR);
			write(fd2,&ac,sizeof(ac));	
		}
	}
	if(found==0)
	{
		sendReadToClient(socket_desc,"Account does not exist\n");
		lock.l_type=F_UNLCK;
		fcntl(fd2,F_SETLK,&lock);
		return -1;
	}
	
	lock.l_type=F_UNLCK;
	fcntl(fd2,F_SETLK,&lock);
	
	fd=open("customer_db",O_RDWR);
	lock.l_type=F_WRLCK; 		
  	lock.l_whence=SEEK_SET; 	
  	lock.l_start=0; 			
  	lock.l_len=0; 			
  	lock.l_pid=getpid(); 
  	fcntl(fd2, F_SETLKW, &lock);
	
	while(read(fd,&cust,sizeof(cust)))
	{
		if(accountnumber==cust.account_number && cust.del_flag==0)
		{
			cust.del_flag=1;
			lseek(fd,-sizeof(cust),SEEK_CUR);
			write(fd,&cust,sizeof(cust));
				
		}
	}
	
	lock.l_type=F_UNLCK;
	fcntl(fd2,F_SETLK,&lock);
	
		
	sendReadToClient(socket_desc,"Successfully deleted the account\n");
		
	close(fd);
	close(fd2);
	return 0;

}

void modify_account(int socket_desc)
{
	char choice[1024];
	char username[1024];
	char password[1024];
	struct flock lock;
	char temp[1024];
	
	sendWriteToClient(socket_desc,"\nPress 1 to change password\nPress 2 to change account to joint account\n",choice);
	if(choice[0]=='1')
	{
		sendWriteToClient(socket_desc,"Enter username:",username);
		int fd=open("customer_db",O_RDWR);
		int found=0;
		struct Customer cust;
		
		lock.l_type=F_RDLCK; 		
  		lock.l_whence=SEEK_SET; 	
  		lock.l_start=0; 			
  		lock.l_len=0; 			
  		lock.l_pid=getpid(); 
  		fcntl(fd, F_SETLKW, &lock);	
  
		
		while(found==0 && read(fd,&cust,sizeof(cust)))
		{
			if(strcmp(cust.username,username)==0 && cust.del_flag==0){
				found=1;
			}
		}
		lock.l_type=F_UNLCK;
  		fcntl(fd,F_SETLK,&lock);
  		
  		lock.l_type=F_WRLCK;
  		lock.l_whence=SEEK_CUR; 	
  		lock.l_start=-sizeof(cust); 			
  		lock.l_len=sizeof(cust); 
  		fcntl(fd, F_SETLKW, &lock);
  		
		
		if(found==1)
		{
			lseek(fd,-sizeof(cust),SEEK_CUR);
			sendWriteToClient(socket_desc,"Enter new password:",password);
			strcpy(cust.password,password);
			write(fd,&cust,sizeof(cust));
			close(fd);
			sendReadToClient(socket_desc,"Password Succesfully Changed\n");
		}
		else
		{
			sendReadToClient(socket_desc,"User not Found\n");
		
		}
		lock.l_type=F_UNLCK;
  		fcntl(fd,F_SETLK,&lock);
	
	}
	else if(choice[0]=='2')
	{
		sendWriteToClient(socket_desc,"Enter username:",username);
		int fd=open("customer_db",O_RDWR);
		int found=0;
		char username2[1024];
		struct Customer cust,cust2;
		lock.l_type=F_RDLCK; 		
  		lock.l_whence=SEEK_SET; 	
  		lock.l_start=0; 			
  		lock.l_len=0; 			
  		lock.l_pid=getpid(); 
  		fcntl(fd, F_SETLKW, &lock);
  		
		while(found==0 && read(fd,&cust,sizeof(cust)))
		{
			if(strcmp(cust.username,username)==0 && cust.del_flag==0){
				found=1;
			}
		}
		lock.l_type=F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		
		
		
		if(found==1 && cust.type==2)
		{
			lock.l_type=F_WRLCK; 		
  			lock.l_whence=SEEK_CUR; 	
  			lock.l_start=-sizeof(cust); 			
  			lock.l_len=sizeof(cust); 
  			fcntl(fd, F_SETLKW, &lock);
  				
			sendWriteToClient(socket_desc,"Enter new Username:",username2);
			sendWriteToClient(socket_desc,"Enter new Password:",password);
			lseek(fd,-sizeof(cust),SEEK_CUR);
			cust.type=3; //joint user
			write(fd,&cust,sizeof(cust));
			
			lock.l_type=F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			
			lseek(fd,0,SEEK_END);
			
			strcpy(cust2.username,username2);
			strcpy(cust2.password,password);
			cust2.account_number=cust.account_number;
			cust2.del_flag=0;
			cust2.type=3;
			
			lock.l_type=F_WRLCK; 		
  			lock.l_whence=SEEK_END; 	
  			lock.l_start=0; 			
  			lock.l_len=sizeof(cust); 
  			fcntl(fd, F_SETLKW, &lock);
			
			write(fd,&cust2,sizeof(cust2));
			lock.l_type=F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			
			
			close(fd);
			sendReadToClient(socket_desc,"Joint account successfully created\n");
		}
		else if(found==0)
		{
			sendReadToClient(socket_desc,"User not Found\n");
		
		}
		else
		{
			sendReadToClient(socket_desc,"User already has a joint account\n");
		}
	
	
	
	}
	else{
		sendReadToClient(socket_desc,"Invalid Choice\n");
	}
}

void search_account(int socket_desc){
	
	char username[1024],balance[1024],acno[1024];
	int found=0;
	char username2[1024];
	struct Customer cust;
	struct Account ac;
	struct flock lock;
	
	
	int fd=open("customer_db",O_RDONLY);
	lock.l_type=F_RDLCK; 		
 	lock.l_whence=SEEK_SET; 	
  	lock.l_start=0; 			
  	lock.l_len=0; 			
  	lock.l_pid=getpid(); 
  	fcntl(fd, F_SETLKW, &lock);	
	
	
	sendWriteToClient(socket_desc,"Enter username:",username);
	
	while(found==0 && read(fd,&cust,sizeof(cust))){
		if(strcmp(cust.username,username)==0 && cust.del_flag==0){
			found=1;
		}
	}
	lock.l_type=F_UNLCK;
 	fcntl(fd,F_SETLK,&lock);
	close(fd);
	
	if(found==0)
	{
		sendReadToClient(socket_desc,"User not found\n");
	}
	else
	{	
		fd=open("account_db",O_RDONLY);
		
		lock.l_type=F_RDLCK; 		
 		lock.l_whence=SEEK_SET; 	
  		lock.l_start=0; 			
  		lock.l_len=0; 			
  		lock.l_pid=getpid(); 
  		fcntl(fd, F_SETLKW, &lock);
		
		lseek(fd,sizeof(int),SEEK_SET);		
		lseek(fd,sizeof(ac)*(cust.account_number-5000),SEEK_CUR);
		read(fd,&ac,sizeof(ac));
		
		lock.l_type=F_UNLCK;
 		fcntl(fd,F_SETLK,&lock);
		close(fd);
		

		sendReadToClient(socket_desc,"\nUsername: ");
		sendReadToClient(socket_desc,cust.username);
		sendReadToClient(socket_desc,"\nPassword: ");
		sendReadToClient(socket_desc,cust.password);
		sendReadToClient(socket_desc,"\nAccountType: ");
		if(cust.type==1)
			sendReadToClient(socket_desc,"Admin");
		else if(cust.type==2)
			sendReadToClient(socket_desc,"Normal Account");
		else
			sendReadToClient(socket_desc,"Joint Account");
		
		
		sprintf(acno,"%ld",cust.account_number);
		sendReadToClient(socket_desc,"\nAccount number: ");
		sendReadToClient(socket_desc,acno);
		
		sprintf(balance,"%.2f",ac.balance);
		sendReadToClient(socket_desc,"\nAccount Balance:");
		sendReadToClient(socket_desc,balance);		
	}	
}



void deposit(int socket_desc, struct Customer record)
{
	struct Account ac;
	struct transaction tran;
	char amount[1024];
	float dep_amount;
	
	int fd=open("account_db",O_RDWR);
	int fd2=open("transaction_db",O_WRONLY|O_APPEND);
	struct flock lock, lock2;
	
	lock.l_type=F_WRLCK; 		
  	lock.l_whence=SEEK_SET; 	
  	lock.l_start=sizeof(ac)*(record.account_number-5000)+sizeof(int); 			
  	lock.l_len=sizeof(ac); 			
  	lock.l_pid=getpid(); 
  	fcntl(fd, F_SETLKW, &lock);
  	
	
	sendWriteToClient(socket_desc,"Enter the deposit amount:",amount);
	dep_amount=atof(amount);
	
	if(dep_amount <= 0) {
         sendReadToClient(socket_desc, "Deposit Amount should be greater than zero\n");
         return;
     }

	
	lseek(fd,sizeof(int),SEEK_SET);
	lseek(fd,sizeof(ac)*(record.account_number-5000),SEEK_CUR);
	read(fd,&ac,sizeof(ac));
	ac.balance=ac.balance+dep_amount;
	lseek(fd,-sizeof(ac),SEEK_CUR);
	write(fd,&ac,sizeof(ac));
	
	lock.l_type=F_UNLCK;
 	fcntl(fd,F_SETLK,&lock);
	
	tran.account_number=ac.account_number;
	tran.amount=dep_amount;
	tran.balance_remaining=ac.balance;
	
	char text[12];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(text, sizeof(text)-1, "%d/%m/%Y", t);
	
	lock2.l_type=F_WRLCK; 		
  	lock2.l_whence=SEEK_END; 	
 	lock2.l_start=0; 			
  	lock2.l_len=sizeof(tran); 			
  	lock2.l_pid=getpid(); 
  	fcntl(fd2, F_SETLKW, &lock2);
	
	strcpy(tran.date,text);
	write(fd2,&tran,sizeof(tran));
	
	lock2.l_type=F_UNLCK;
  	fcntl(fd2,F_SETLK,&lock2);
	
	close(fd);
	close(fd2);
	sendReadToClient(socket_desc,"Successfully Deposited the amount\n");

}

void withdraw(int socket_desc,struct Customer record)
{
	struct Account ac;
	struct transaction tran;
	char amount[1024];
	float withdraw_amount;
	
	struct flock lock, lock2;
	
	int fd=open("account_db",O_RDWR);
	lock.l_type=F_WRLCK; 		
  	lock.l_whence=SEEK_SET; 	
  	lock.l_start=sizeof(ac)*(record.account_number-5000)+sizeof(int); 			
  	lock.l_len=sizeof(ac); 			
  	lock.l_pid=getpid(); 
  	fcntl(fd, F_SETLKW, &lock);
	
	sendWriteToClient(socket_desc,"Enter the amount to be withdrawn:",amount);
	withdraw_amount=atof(amount);

	lseek(fd,sizeof(int),SEEK_SET);
	lseek(fd,sizeof(ac)*(record.account_number-5000),SEEK_CUR);
	read(fd,&ac,sizeof(ac));
	
	if(ac.balance<withdraw_amount)
	{
		sendReadToClient(socket_desc,"Insufficient balance\n");
		return;
	}
	ac.balance=ac.balance-withdraw_amount;
	lseek(fd,-sizeof(ac),SEEK_CUR);
	write(fd,&ac,sizeof(ac));
	
	lock.l_type=F_UNLCK;
  	fcntl(fd,F_SETLK,&lock);
	close(fd);
	
	
	tran.account_number=ac.account_number;
	tran.amount=-withdraw_amount;
	tran.balance_remaining=ac.balance;

	char text[12];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(text, sizeof(text)-1, "%d/%m/%Y", t);
	strcpy(tran.date,text);
		
	int fd2=open("transaction_db",O_WRONLY|O_APPEND);
	
	lock2.l_type=F_WRLCK; 		
  	lock2.l_whence=SEEK_END; 	
 	lock2.l_start=0; 			
  	lock2.l_len=sizeof(tran); 			
  	lock2.l_pid=getpid(); 
  	fcntl(fd2, F_SETLKW, &lock2);
  	
	write(fd2,&tran,sizeof(tran));
	lock.l_type=F_UNLCK;
  	fcntl(fd,F_SETLK,&lock);
	
	close(fd2);
	sendReadToClient(socket_desc,"Successfully withdrawn the amount\n");

}
void change_password(int socket_desc,struct Customer record)
{
	struct Customer cust;
	struct flock lock;
	char newpass[1024],oldpass[1024];
	
	
	sendWriteToClient(socket_desc,"Enter old password:",oldpass);
	
	
	int found=0;
	int fd=open("customer_db",O_RDWR);
	
	lock.l_type=F_RDLCK; 		
  	lock.l_whence=SEEK_SET; 	
  	lock.l_start=0; 			
  	lock.l_len=0; 			
  	lock.l_pid=getpid(); 
  	fcntl(fd, F_SETLKW, &lock);
	
	while(found==0 && read(fd,&cust,sizeof(cust)))
	{
		if(strcmp(cust.username,record.username)==0 && cust.del_flag==0)
		{
			
			lock.l_type=F_UNLCK;
  			fcntl(fd,F_SETLK,&lock);
  			
  			lock.l_type=F_WRLCK; 		
  			lock.l_whence=SEEK_CUR; 	
  			lock.l_start=-sizeof(cust); 			
  			lock.l_len=sizeof(cust); 
  			fcntl(fd, F_SETLKW, &lock);
  			
  			if(strcmp(oldpass,cust.password)!=0)
			{
				sendReadToClient(socket_desc,"Incorrect Password\n");
				lock.l_type=F_UNLCK;
  				fcntl(fd,F_SETLK,&lock);
				return;
			}
			sendWriteToClient(socket_desc,"Enter new password:",newpass);
  			
			found=1;
			lseek(fd,-sizeof(cust),SEEK_CUR);
			strcpy(cust.password,newpass);
			write(fd,&cust,sizeof(cust));
			
			lock.l_type=F_UNLCK;
  			fcntl(fd,F_SETLK,&lock);	
			
		}
	}
	close(fd);
	sendReadToClient(socket_desc,"Password successfully Changed\n");
}
void check_balance(int socket_desc, struct Customer record)
{
	struct Account ac;
	struct flock lock;
	char balance[1024];
	int fd=open("account_db",O_RDWR);
	
	lock.l_type=F_RDLCK; 		
  	lock.l_whence=SEEK_SET; 	
  	lock.l_start=sizeof(int)+sizeof(ac)*(record.account_number-5000); 			
  	lock.l_len=sizeof(ac); 			
  	lock.l_pid=getpid(); 
  	fcntl(fd, F_SETLKW, &lock);
  	
	lseek(fd,sizeof(int),SEEK_SET);
	lseek(fd,sizeof(ac)*(record.account_number-5000),SEEK_CUR);
	read(fd,&ac,sizeof(ac));
	
	lock.l_type=F_UNLCK;
  	fcntl(fd,F_SETLK,&lock);
  	close(fd);
	
	
	sendReadToClient(socket_desc,"Your current balance is: Rs");
	sprintf(balance,"%.2f\n",ac.balance);
	sendReadToClient(socket_desc,balance);

}

void view_details(int socket_desc, struct Customer record)
{
	struct flock lock;
	struct transaction tran;
	char balance[1024],amount[1024],acno[1024];
	int fd=open("transaction_db",O_RDONLY);
	int found=0;
	
	lock.l_type=F_RDLCK; 		
  	lock.l_whence=SEEK_SET; 	
  	lock.l_start=0; 			
  	lock.l_len=0; 			
  	lock.l_pid=getpid(); 		 
  	fcntl(fd, F_SETLKW, &lock);
	while(read(fd,&tran,sizeof(tran)))
	{
		if(tran.account_number==record.account_number)
		{
			found=1;
			sendReadToClient(socket_desc,"\nAccount number: ");
			sprintf(acno,"%ld",tran.account_number);
			sendReadToClient(socket_desc,acno);
			sendReadToClient(socket_desc,"\nDate: ");
			sendReadToClient(socket_desc,tran.date);
			sendReadToClient(socket_desc,"\nAmount transferred: ");
			sprintf(amount,"%.2f",tran.amount);
			sendReadToClient(socket_desc,amount);
			sendReadToClient(socket_desc,"\nRemaining Balance: ");
			sprintf(balance,"%.2f",tran.balance_remaining);
			sendReadToClient(socket_desc,balance);
			sendReadToClient(socket_desc,"\n----------------------\n");
			
		
		}
	}
	
	lock.l_type=F_UNLCK;
  	fcntl(fd,F_SETLK,&lock);
	if(found==0)
		sendReadToClient(socket_desc,"\n\nNo Transactions made yet\n\n");



}


void display_menu(int socket_desc,struct Customer record){
	char choice[10];
	while(1)
	{if(record.type==1){
		
		char welcome_msg[1024]="\n\n-----------------WELCOME ADMIN----------------------\n\n";
		sendReadToClient(socket_desc,welcome_msg);
		char menu[1024]="Press 1 to add an account\nPress 2 to delete an account\nPress 3 to modify an account\nPress 4 to search an account\nPress 5 to Exit\n";
		sendWriteToClient(socket_desc,menu,choice);
		switch(choice[0])
		{
			case '1': sendReadToClient(socket_desc,"\n---------------Add Account---------------\n\n");
				add_account(socket_desc);
				break;
			case '2': sendReadToClient(socket_desc,"\n---------------Delete Account-------------\n\n");
					//sendWriteToClient(socket_desc,"Press 1 to delete user\nPress 2 to delete account\n");
					delete_account(socket_desc);
				break;
			case '3': sendReadToClient(socket_desc,"\n---------------Modify Account-------------\n\n");
					modify_account(socket_desc);
				break;
			case '4': sendReadToClient(socket_desc,"\n---------------Search account--------------\n\n");
					search_account(socket_desc);
				break;
			case '5' : return;
			default: sendReadToClient(socket_desc,"Invalid option\n");
		}
	}
	else
	{
		char welcome_msg[1024]="\n\n---------------WELCOME USER-------------------\n\n";
		sendReadToClient(socket_desc,welcome_msg);
		char menu[1024]="Press 1 to deposit\nPress 2 to withdraw\nPress 3 to check balance\nPress 4 to change password\nPress 5 to view details\nPress 6 to Exit\n";
		sendWriteToClient(socket_desc,menu,choice);
		switch (choice[0])
		{
			case '1': sendReadToClient(socket_desc,"\n------------------Deposit---------------\n\n");
					deposit(socket_desc,record);
				break;
			case '2': sendReadToClient(socket_desc,"\n-----------------Withdraw----------------\n\n");
					withdraw(socket_desc,record);
				break;
			case '3':sendReadToClient(socket_desc,"\n------------------Check Balance-------------\n\n");
					check_balance(socket_desc,record);
				break;
			case '4':sendReadToClient(socket_desc,"\n------------------Change Password-------------\n\n");
					change_password(socket_desc,record);
				break;
			case '5':sendReadToClient(socket_desc,"\n------------------View Details------------\n\n");
					view_details(socket_desc,record);
				break;
			case '6': return;
		}
	
	
	}}
}

void validate_credentials(int socket_desc){
	char welcome_msg[1024] = "\n------------------WELCOME TO ONLINE BANKING SYSTEM--------------------\n\n";
	sendReadToClient(socket_desc, welcome_msg); 
	char username[1024], password[1024];
	char msg_username[1024] = "Enter username: ";
	char msg_password[1024] = "Enter password: ";
	sendWriteToClient(socket_desc, msg_username, username);
	sendWriteToClient(socket_desc, msg_password, password);
	struct Customer record;
	int fd = open("customer_db", O_RDONLY);
	bool flag = false;
	while(read(fd, &record, sizeof(record))){
		if(strcmp(record.username, username) == 0 && strcmp(record.password, password) == 0 && !record.del_flag){

			display_menu(socket_desc,record);
			flag = true;
			break;
		}
	}
	if(!flag){
		sendReadToClient(socket_desc, "Invalid username/password. Logging out.\n");
		return;
	}
}

void main()
{
	struct sockaddr_in serverad,clientad;
	int sfd,nsd,clientLen, client_socket;
	char buff[50];

	sfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);  
	initialSetup();

	serverad.sin_family=AF_INET;
	serverad.sin_addr.s_addr=INADDR_ANY;
	serverad.sin_port=htons(PORT);  						//convers 5555 from host byte order to network byte order
	
	bind(sfd,(struct sockaddr *)&serverad,sizeof(serverad));    //binds a name to socket
	
	listen(sfd,2); 									//coverts socket to passive, used to accept connections

	printf("Waiting for client...\n");
	while(1)
	{		
		clientLen=sizeof(clientad);
		client_socket=accept(sfd,(struct sockaddr *)&clientad,&clientLen);  //accepting connection
		printf("\n");
		write(1,"Connected to client...\n",sizeof("Connected to client...\n"));
		if(!fork()){
			validate_credentials(client_socket);
			char close_message[1024] = "\nShutting down";
			sendCloseToClient(client_socket, close_message);
		}
		else
		{
			close(nsd);		
		}
	}
	close(sfd);

}
