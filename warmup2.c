/* Warmup 2
Author: harshit
*/

#include "warmup2.h"



struct timeval start_time;
pthread_t packet_thread,token_thread,s1_thread,s2_thread,sighand_thread;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
sigset_t set;

int B=10, P=3, num=20;
double r=1.5,lambda=1,mu=0.35;
int count=0; //tokens in bucket
int num_completed=0;

My402List Q1,Q2;
char* tsfile;
FILE* file;
int mode;
int flag=1;
///int flag_windup=0;
//mode=0 deterministic
//mode=1 non-deterministic
double sum_tarrival=0,sum_tservice=0;
double sum_tinQ1=0,sum_tinQ2=0;
double sum_servicetime1=0,sum_servicetime2=0;
double sumx=0,sumxsq=0;
int count_dropped=0;
int num_dropped=0;
int tokenID=0;
/*packets to go into Q1 & Q2*/
int yet_2pass_q1=0;
int yet_2pass_q2=0;

/***************************************************************/


//*function to calculate difference between timestamps*//
void time_diff(double* result, struct timeval* end, struct timeval* start)
{
 double x= (start->tv_sec)*1000000 + (start->tv_usec);
 double y= (end->tv_sec)*1000000 + (end->tv_usec);
 *result = (y-x)/1000; //milliseconds
}


//*function to move packets from q1 to q2, always call with mutex locked*//
void q1toq2()  
{
	double timestamp=0, tinQ1=0;
	struct timeval time;
	My402ListElem* felem = My402ListFirst(&Q1);
	if (felem!=NULL)
	{
		packet* p=(packet*)felem->obj;
		if(p->tokenReq <= count)
		{
			count = count - p->tokenReq;
			
			gettimeofday(&time,NULL);
			time_diff(&timestamp,&time,&start_time);
			My402ListUnlink(&Q1,felem);
			time_diff(&tinQ1,&time,&(p->tq1));
			sum_tinQ1=sum_tinQ1+tinQ1;
			printf("%012.3lfms: p%d leaves Q1, time in Q1 = %.6gms, token bucket now has %d tokens\n",timestamp,p->packetID,tinQ1,count);

			
			gettimeofday(&time,NULL);
			p->tq2 = time;
			time_diff(&timestamp,&time,&start_time);
			My402ListAppend(&Q2,(void*)p);
			printf("%012.3lfms: p%d enters Q2\n",timestamp,p->packetID);
			if(My402ListLength(&Q2)==1)
				pthread_cond_broadcast(&cv); //wakeup servers if q2 is empty
			
		}

	}

}

//* Main function: Parent thread *//
int main(int argc, char *argv[])
{
	//block SIGINI signals
	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	sigprocmask(SIG_BLOCK,&set,0);
	/*Initialising Queues Q1 and Q2 */
	My402ListInit(&Q1);
	My402ListInit(&Q2);
	//lambda=1;
	//mu=0.35;
	//r=1.5;
	if(argc % 2 == 0)
	{
	    printf("Bad Parameter\n");
		ccl();
		exit(1);
	}
	mode=0; //deterministic
	int i;
	for(i=1;i<argc;i++)
	{
		if(strcmp("-lambda",argv[i])==0)
		{
			lambda=strtod(argv[i+1],NULL);
			i++;
		}
		else if(strcmp("-mu",argv[i])==0)
		{
			mu=strtod(argv[i+1],NULL);
			i++;
		}
		else if(strcmp("-r",argv[i])==0)
		{
			r=strtod(argv[i+1],NULL);
			i++;
		}
		else if(strcmp("-B",argv[i])==0)
		{
			B=atoi(argv[i+1]);
			i++;
		}
		else if(strcmp("-P",argv[i])==0)
		{
			P=atoi(argv[i+1]);
			i++;
		}
		else if(strcmp("-n",argv[i])==0)
		{
			num=atoi(argv[i+1]);
			i++;
		}
		else if(strcmp("-t",argv[i])==0)
		{
			tsfile=argv[i+1];
			mode=1; //non-deterministic
			i++;
		}
		else
		{
			printf("Bad Parameter\n");
			ccl();
			exit(1);
		}

	}

	if(mode==1)
	{
		int exist=fileexists(tsfile);
		if(exist==0)
		{
			printf("Input File doesn't exist or error during file read\n");
			exit(1);
		}
		/*if(errno!=0)
		{
			printf("File cannot be opened (access denied)\n");
			exit(1);
		}*/
				
		file = fopen(tsfile,"r");
		if(file==NULL)
		{
		printf("File cannot be opened (access denied)\n");
		exit(1);
		}
		
		char buff[1025]; //to read 1024 characters and add '/o'to it. 
	// 1023+\n+\o
		if(fgets(buff,1024,file)==NULL)
		{
		printf("Error in Line 1: Not in correct format");
		exit(1);
		}
		int len = strlen(buff);
		if(buff[0]==' '||buff[0]=='\t'||buff[len-1]==' '||buff[len-1]=='\t')
		{
		printf("Error in Line 1: Not in correct format");
		exit(1);
		}
		else{
		num=atoi(buff);
		if(num<=0){
		printf("malformed input - line 1 is not just a number\n");
		exit(1);
		}
		}
    }

	printf("Emulation Parameters:\n");
	printf("	number to arrive = %d\n",num);


	if(mode!=1) //deterministic
	{
	printf("	lambda = %.6g\n",lambda);
	printf("	mu = %.6g\n", mu);
	printf("	r = %.6g\n",r);
	printf("	B = %d\n",B);
	printf("	P = %d\n",P);
	}
	else
		{
			printf("	r = %.6g\n",r);
			printf("	B = %d\n",B);
			printf("	tsfile = %s\n",tsfile);
		}


	yet_2pass_q1=num;
	yet_2pass_q2=num;

	gettimeofday(&start_time,NULL);
	printf("\n00000000.000ms: emulation begins\n");

	pthread_create(&packet_thread,0,packet_arr,0);
	pthread_create(&token_thread,0,token_dep,0);
	
	
	pthread_create(&s1_thread,0,server,(void*)1);
	pthread_create(&s2_thread,0,server,(void*)2);
	pthread_create(&sighand_thread,0,sighand,0);


	pthread_join(packet_thread,0);
	pthread_join(token_thread,0);
	pthread_cond_broadcast(&cv);
	//to wakeup servers since packet and token thread have returned 
	//and there is no one to wake them up if they are sleeping
	pthread_join(s1_thread,0);
	pthread_join(s2_thread,0);


	struct timeval time;
	double timestamp=0;
	gettimeofday(&time,NULL);
	time_diff(&timestamp,&time,&start_time);
	printf("%012.3lfms: emulation ends\n\n",timestamp);

	pthread_cancel(sighand_thread);
	// to cancel signal catching thread

	/*Stats*/

	printf("Statistics:\n\n");
	if(num==0)
	printf("\taverage packet inter-arrival time = N/A (no packet generated)\n");	
	else
	printf("\taverage packet inter-arrival time = %.6g\n",(sum_tarrival/(double)num));

	if(num_completed==0)
		printf("\taverage packet service time = N/A (no packet completed service)\n");
	else
	printf("\taverage packet service time = %.6g\n\n",(sum_tservice/(double)num_completed));

	
	printf("\taverage number of packets in Q1 = %.6g\n",(sum_tinQ1/timestamp));
	printf("\taverage number of packets in Q2 = %.6g\n",(sum_tinQ2/timestamp));

	printf("\taverage number of packets in S1 = %.6g\n",(sum_servicetime1/timestamp));
	printf("\taverage number of packets in S2 = %.6g\n\n",(sum_servicetime2/timestamp));

	if(num_completed==0)
		printf("\taverage time a packet spent in system = N/A (no packet completed service)\n");
	else
	printf("\taverage time a packet spent in system = %.6g\n",(sumx/(double)num_completed));
	

	double expxsq= sumxsq/(double)num_completed;
	double expx = sumx/(double)num_completed;
	double var = expxsq - expx*expx;
	double sd=0;
	sd = sqrt(var);

	if(num_completed==0)
		printf("\tstandard deviation for time spent in system = N/A (no packet completed service)\n");
	else
	printf("\tstandard deviation for time spend in system = %.6g\n\n",sd);

	if(tokenID==0)
		printf("\ttoken drop probability = N/A (no token generated)\n");
	else
	printf("\ttoken drop probability = %.6g\n",(double)count_dropped/tokenID);

	if(num==0)
		printf("\tpacket drop probability = N/A (no packet completed service)\n");
	else
	printf("\tpacket drop probability = %.6g\n",(double)num_dropped/num);
	///(count+count_dropped)
	//count_dropped/total_count

	return 0;
}


//** token depositing function: child thread **//

void* token_dep(void* arg)   //function to deposit tokens
{
//printf("1111");
double ttoken= (r<=0.1) ? 10000000 : 1000000/r; //in microseconds
//double ttoken= 1000000/r;
struct timeval time;
double timestamp=0;
//token id initialized
while(My402ListEmpty(&Q1)!=TRUE || yet_2pass_q1){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
	usleep(ttoken);
	tokenID++;
    
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);	
	//no cancelation after this stage
	pthread_mutex_lock(&m);
	gettimeofday(&time,NULL);
	time_diff(&timestamp,&time,&start_time);
	if(count<=B)
	{
		count++;
		printf("%012.3lfms: token t%d arrives, token bucket now has %d tokens\n",timestamp,tokenID,count);

	}
	else
		{
		printf("%012.3lfms: token t%d arrives, dropped\n",timestamp,tokenID);
		count_dropped++;
		}
	q1toq2(); 
	pthread_mutex_unlock(&m);
}
return 0;
}



//** packet generating function: child thread **//
void* packet_arr(void* arg)
{	struct timeval time;
	int packettag=0;
	 
	double tpacket= (lambda<=0.1) ? 10000000 : 1000000/lambda; //in microseconds
	double timestamp=0;
	double inter_arrival=0,service_time=0;
	int tokens=0;
	while(packettag<num && yet_2pass_q1)
	{
	yet_2pass_q1--;
		
	packet* p = (packet*)malloc(sizeof(packet));
	//packet* p=NULL;
	packettag++;
	
	p->packetID=packettag;
	//deter
	if(mode==0)
	{
	p->tokenReq=P;
	p->tservice = (mu<=0.1) ? 10000 : 1000/mu; // in milliseconds
	p->tarrival= tpacket/1000; //in milliseconds
    }
    else if (mode==1) //non-deterministic
    {

    //char* element[3];
    	char buff[1024];
    	fgets(buff,1024,file);
    	char* elem[3]; // to store all tokens
    	char* token;
    	token = strtok(buff," \t");
    	int i=0;
    	while(token!=NULL)
    	{
    		//case of more than three tokens
    		elem[i]= token;
    		token=strtok(NULL," \t");
    		i++;
    	}
    	inter_arrival=strtod(elem[0],NULL);
    	service_time=strtod(elem[2],NULL);
    	tokens=atoi(elem[1]);
    	p->tarrival=inter_arrival;
    	p->tservice=service_time;
    	p->tokenReq=tokens;


    	///errno errors
    }

    sum_tservice = sum_tservice + p->tservice;
    sum_tarrival = sum_tarrival + p->tarrival;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
	usleep(p->tarrival*1000);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);

	gettimeofday(&time,NULL);
	time_diff(&timestamp,&time,&start_time);
	p->time_arr=time;
    
	if (p->tokenReq>B)
	{
	printf("%012.3lfms: packet p%d arrives, needs %d tokens, inter-arrival time = %.6gms, dropped\n",timestamp,p->packetID,p->tokenReq,p->tarrival);
	num_dropped++;
	yet_2pass_q2--;
	continue;
    }
	else
	printf("%012.3lfms: packet p%d arrives, needs %d tokens, inter-arrival time = %.6gms\n",timestamp,p->packetID,p->tokenReq,p->tarrival);

	pthread_mutex_lock(&m);

	My402ListAppend(&Q1,(void*)p);

	gettimeofday(&time,NULL);
	p->tq1=time; //time when entered q1
	time_diff(&timestamp,&time,&start_time);
	printf("%012.3lfms: p%d enters Q1\n",timestamp,p->packetID);

	q1toq2();
	pthread_mutex_unlock(&m);

    }
    //flag=0;
    return 0;
}


//** server threads: child threads **//
void* server(void* arg)
{
	double timestamp=0, tinQ2=0,timeinsys=0,servicetime=0;
	struct timeval time,time_prev;
while(yet_2pass_q2)
{
	/*if(flag_windup==1)
	{ 
		return 0;
	}*/
	pthread_mutex_lock(&m);
	while(My402ListEmpty(&Q2)==TRUE && yet_2pass_q2)
				pthread_cond_wait(&cv, &m);  //sleep if q2 is empty

	
	if(yet_2pass_q2==0){
		pthread_mutex_unlock(&m);
		break;
	}
	My402ListElem* felem = My402ListFirst(&Q2);
	yet_2pass_q2--;

	if (felem!=NULL)
	{
			packet* p=(packet*)felem->obj;
			//int packetnum = p->packetID;

			gettimeofday(&time,NULL);
			time_diff(&timestamp,&time,&start_time);
			My402ListUnlink(&Q2,felem);
			time_diff(&tinQ2,&time,&p->tq2);
			sum_tinQ2=sum_tinQ2+tinQ2;
			printf("%012.3lfms: p%d leaves Q2, time in Q2 = %.6gms\n",timestamp,p->packetID,tinQ2);

			pthread_mutex_unlock(&m);
			//My402ListAppend(&Q2,(void*)felem);
			gettimeofday(&time,NULL);
			time_diff(&timestamp,&time,&start_time);
			time_prev=time;
			printf("%012.3lfms: p%d begins service at S%d, requesting %.6gms of service \n",timestamp,p->packetID,(int)arg,p->tservice);

			usleep(p->tservice*1000);

			gettimeofday(&time,NULL);
			time_diff(&timestamp,&time,&start_time);
			time_diff(&timeinsys,&time,&p->time_arr);
			time_diff(&servicetime,&time,&time_prev);
			printf("%012.3lfms: p%d departs from S%d, service time = %.6gms, time in system = %.6gms\n",timestamp,p->packetID,(int)arg,servicetime,timeinsys);
			
			free(p);
			/*if(packetnum == num)
			{
				flagserver=0;
				return 0;
			}
			else if(flagserver==0)
				return 0;*/
			num_completed++;
			if((int)arg==1)
				sum_servicetime1 += servicetime;
			else
				sum_servicetime2 += servicetime;
			sumx = sumx + timeinsys;
			sumxsq = sumxsq + timeinsys*timeinsys;
	}
	else 
		pthread_mutex_unlock(&m);
}
return 0;
}


//** correct input format **//
void ccl()
{
	printf( "Input format:\nwarmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n\tSquare bracketed items are optional\n");
}


//** function to check if file exists **//
int fileexists(const char* filename)
{
	/*FILE *file;
	file=fopen(filename,"r");
	if(file!=NULL);//opening file for reading, must exist
	{
		fclose(file);
		return 1;
	}
	return 0;*/


	struct stat buffer;
	int exist = stat(filename,&buffer);
	if(exist==0)
		return 1; //file exists
	else
		return 0;
}




//** function to catch signal Ctrl+C : child thread **//
void* sighand(void* arg)
{
struct timeval time;
double timestamp;
int sig;
sigwait(&set,&sig);
gettimeofday(&time,NULL);
time_diff(&timestamp,&time,&start_time);
printf("\n%012.3lfms: SIGINT caught, no new packets or tokens will be allowed\n",timestamp);
//start here
pthread_cancel(packet_thread);
pthread_cancel(token_thread);


//when sigwait returns
pthread_mutex_lock(&m);
yet_2pass_q1=0;
yet_2pass_q2=0;
//flag_windup=1;
pthread_cond_broadcast(&cv);
removepackets(&Q1,1);
removepackets(&Q2,2);
pthread_mutex_unlock(&m);



return 0;
}

void removepackets(My402List* list,int arg)
{
struct timeval time;
double timestamp;
while(My402ListEmpty(list)==FALSE)
{
My402ListElem* felem= My402ListFirst(list);

packet* p=(packet*)felem->obj;
My402ListUnlink(list, felem);
gettimeofday(&time,NULL);
time_diff(&timestamp,&time,&start_time);
printf("%012.3lfms: p%d removed from Q%d\n",timestamp,p->packetID,arg);
free(p); 
}

}
