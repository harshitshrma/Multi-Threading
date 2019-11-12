
#ifndef WARMUP2_H
#define WARMUP2_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>
#include <unistd.h> //for sleep() 
#include <errno.h>
#include <sys/stat.h> //for fileexists
#include "my402list.h"


typedef struct tagPacket
{
	int packetID, tokenReq;
	double tservice,tarrival; //times as per the trace file
	//tarrival = interarrival time
	struct timeval tq1, tq2, time_arr; //time when entered in q1/q2/system
}packet;

extern void time_diff(double* result, struct timeval* end, struct timeval* start);
extern void q1toq2();
extern void* token_dep(void* arg);
extern void* packet_arr(void* arg);
extern void* server(void* arg);
extern void ccl();
extern int fileexists(const char * filename);
extern void* sighand(void* arg);
extern My402List Q1,Q2;
extern void removepackets(My402List* list,int arg);


#endif /*_WARMUP2_H_*/
