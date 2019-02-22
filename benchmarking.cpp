/**
 * Name : Jia, Yichen
 * Project : PA-1 (Simplebench)
 * Instructor : Dr. Feng Chen
 * Class : cs7700-sp18
 * LogonID : cs770095
 * This file is used to issue the req.ests with multiple threads, either sequential or random. 
 * **/

#include "benchmarking.h"

using namespace std;
//#define queue_size 256 // It means this piece of buffer can store 256 req.ests.
#define bench_memcpy(_d, _c, _n)  memcpy(_d, _c, (size_t)(_n))
/*
 * Return 'x' rounded up to the nearest multiple of 'step'. Only valid
 * for x >= 0, step >= 1.
 */
#define ROUND_UP(x, step)   (((x) + (step) - 1) / (step) * (step))

/*
 * Return 'x' rounded down to the nearest multiple of step. Only valid
 * for x >= 0, step >= 1.
 */
#define ROUND_DOWN(x, step) ((x) / (step) * (step))

pthread_spinlock_t spin_mut;
pthread_spinlock_t log_mut;
struct timeval prog_start, prog_finish, prog_diff;
struct timeval issue_start, issue_diff;
struct timeval timestamp;

struct Settings configuration;
int request_counter=0;
int current_offset=0;
int fd;                         /* disk file descriptor */
FILE *outputfile;
size_t request_size;
double total_latency=0.0;
double avg_latency=0.0;
char READ[]="R";
char WRITE[]="W";
char Seq[]="S";
char Random[]="R";
//std::queue<Request> pending_queue;
//std::queue<Request> working_queue;
//std::queue<Request> paused_queue;
std::deque<struct Request> log_deque;
int generateRandom(int lower, int upper)
{
     int num = (rand() + lower) % (upper + 1);
     return num;
}

off_t generate_offset()
{
     off_t offset;
     if (!strcmp(configuration.pattern,Seq))
     {
        //pthread_spin_lock(&spin_mut);
        off_t new_off = current_offset + configuration.request_size; 
	if(new_off > configuration.range)
	{
	    new_off = 0; // To keep the current_offset within the range
	}
        offset = ROUND_DOWN(new_off, getpagesize());
        current_offset=offset; //To current_offset has to be locked before its increment.  
	//printf("the current offset is %zu\n", current_offset);
        request_counter+=1;
	//pthread_spin_unlock(&spin_mut);
     }
     if (!strcmp(configuration.pattern,Random))
     {
        int val = generateRandom(0, configuration.range);
        offset = ROUND_DOWN(val, getpagesize());
	//pthread_spin_lock(&spin_mut);
	request_counter+=1;
	//pthread_spin_unlock(&spin_mut);
     }
     //pthread_spin_lock(&spin_mut);
     //pending_queue.push(req);
     //pthread_spin_unlock(&spin_mut);
     return offset;
     
}

/*****************************************************
 * *This is the function to get the time interval
 * ******************************************************/
int timeval_subtract(struct timeval* result, struct timeval* start, struct timeval* stop)
{
       if (start->tv_sec > stop->tv_sec)
       {
                return -1;
       }
       if ((start->tv_sec == stop->tv_sec) && (start->tv_usec > stop->tv_usec))
       {
                return -1;
       }
       result->tv_sec = (stop->tv_sec - start->tv_sec);
       result->tv_usec = (stop->tv_usec - start->tv_usec);
       if (result->tv_usec < 0)
       {
                result->tv_sec--;
                result->tv_usec = result->tv_usec + 1000000;
       }
       return 0;
}
void log_trace(double lat, size_t offset)
{
     pthread_spin_lock(&log_mut);
     std::deque<struct Request>::iterator it = log_deque.begin();
     while (it->offset!=offset)
     {
         *it++;
     }
     it->latency = lat;
     while(log_deque.front().latency!=0){
	  struct Request req=log_deque.front(); 
          fprintf(outputfile, "%f  %u  %s  %zu  %d  %f\n", req.timestamp, req.thread_ID, req.type, req.offset, req.size, req.latency);
          log_deque.pop_front();
          if(log_deque.empty())	{
	     break;
	  }  
     }
     pthread_spin_unlock(&log_mut);
}
struct Request init_request(void){
        struct timeval io_start;	
	struct Request req;
        gettimeofday(&io_start,0);
        req.size=configuration.request_size;
	req.thread_ID = (unsigned int) pthread_self();
	req.latency=0;
        if (configuration.direct == true)            
	{
	      req.direct=true;
        } else {
              req.direct=false;
        }
	if(request_counter==0){
	        gettimeofday(&issue_start,0);
	        timeval_subtract(&issue_diff, &issue_start, &issue_start);
       	} else {
	        timeval_subtract(&issue_diff, &issue_start, &io_start);
	}
	if (!strcmp(configuration.type,READ))
	{
	    req.type="R";   
	} else if (!strcmp(configuration.type,WRITE))
	{
	    req.type="W";
	}
        pthread_spin_lock(&spin_mut);
       	req.offset=generate_offset();       
	req.timestamp=(double)issue_diff.tv_sec+(double)issue_diff.tv_usec/1000000.0;   	     
	log_deque.push_back(req);
	pthread_spin_unlock(&spin_mut);
	return req;
}
void *IO_processing_thread(void *)
{
    void *memory_buffer;
    struct Request req;
    struct timeval io_start, io_finish, io_diff;
    struct timeval thread_start, thread_finish, thread_diff;
    //memory_buffer = new uint8_t[request_size];    
    gettimeofday(&thread_start,0);
    gettimeofday(&thread_finish,0);
    timeval_subtract(&thread_diff, &thread_start, &thread_finish);
    size_t n;
    off_t offset;
    while((double)thread_diff.tv_sec < configuration.duration)
    {	
        gettimeofday(&io_start,0);
	req=init_request();
	if (!strcmp(configuration.type,READ))
        {
            if(!posix_memalign(&memory_buffer, getpagesize(),req.size)){
                n = pread(fd, memory_buffer, req.size, req.offset);
            }
	    if (n<req.size){
	        printf("READ failed!\n");
	    }
	    gettimeofday(&io_finish,0);
	    timeval_subtract(&io_diff, &io_start, &io_finish);
            double lat=(double)io_diff.tv_sec*1000+ (double)io_diff.tv_usec/1000;	    
	    log_trace(lat, req.offset);
	    total_latency+=io_diff.tv_usec;
	    gettimeofday(&thread_finish,0);
	    timeval_subtract(&thread_diff, &thread_start, &thread_finish);
        }
        if (!strcmp(configuration.type,WRITE))
        {
            if(!posix_memalign(&memory_buffer, getpagesize(), req.size)){
                 n = pwrite(fd, memory_buffer, req.size, req.offset);
            }
	    if(n<req.size){
	        printf("WRITE failed!\n");
	    }
	    gettimeofday(&io_finish,0);
	    timeval_subtract(&io_diff, &io_start, &io_finish);
            double lat=(double)io_diff.tv_sec*1000+ (double)io_diff.tv_usec/1000;
	    log_trace(lat, req.offset);
	    total_latency+=io_diff.tv_usec;
	    gettimeofday(&thread_finish,0);
	    timeval_subtract(&thread_diff, &thread_start, &thread_finish);
        }
    }	
}

void print_stats(){
        printf("Total duration is: %f.\n", (double)prog_diff.tv_sec);
	printf("Total number of requests is: %d.\n", request_counter);
	double data_volumn=request_counter*request_size/(1024*1024);
	printf("Total volumn of data transferred is %f MBs.\n", data_volumn);
	avg_latency=total_latency/request_counter/1000;
	printf("Average latency is %f milliseconds\n",avg_latency);
	printf("Throughput is %f IOPS.\n", request_counter/(double)prog_diff.tv_sec);
	double bandwidth = data_volumn/(double) prog_diff.tv_sec;
	printf("The bandwidth is %f MB/s \n", bandwidth);	
}

int simplebench_run(struct Settings settings)
{
	configuration=settings;
	if(pthread_spin_init(&spin_mut, 0) != 0)
	{
		printf("Spin lock init error!\n");
	}
       	if(pthread_spin_init(&log_mut, 0) != 0)
	{
		printf("Log lock init error!\n");
	}

	request_size=configuration.request_size;
	int create_client_res[configuration.iodepth];
	pthread_t client_thread[configuration.iodepth];
	void *client_thread_result[configuration.iodepth];

	int thread_index = 0;
	int res;
        fd = -1;
        
	gettimeofday(&prog_start, 0);
	if (configuration.direct == true){
	    fd = open(configuration.device, O_CREAT | O_RDWR | O_DIRECT, 0644);
	    printf("fd is %d\n", fd);
	    if (fd < 0) 
	    {
	          printf("open '%s' failed: %s\n", configuration.device);
	          return 0;
	    }
        } else if(configuration.direct == false) {
	    fd = open(configuration.device, O_CREAT | O_RDWR, 0644);
	    if (fd < 0)
	    {
	    	  printf("open '%s' failed: %s\n", configuration.device);
		  return 0;
	    }		
	}

	if (configuration.outputfile!=NULL)
	{
	    outputfile=fopen(configuration.outputfile,"w");
	    if (fd<0)
	    {
	          printf("open '%s' failed: %s\n", outputfile);
		  return 0;
	    }
	
	}
        /* start the threads to process the req.ests*/
	for (thread_index = 0; thread_index < configuration.iodepth; thread_index++)
	{
	        create_client_res[thread_index] = pthread_create(&client_thread[thread_index], NULL, IO_processing_thread, NULL);
	        if (create_client_res[thread_index] != 0)
	        {
	                printf("create client thread error\n");
	        }
	}
	/*stop the client threads*/
        for (thread_index = 0; thread_index < configuration.iodepth; thread_index++)
	{
	        res = pthread_join(client_thread[thread_index], &client_thread_result[thread_index]);
     	        if (res != 0)
	        {
		        perror("Client threads join failed!\n");
		        exit(EXIT_FAILURE);
		}
	}

	gettimeofday(&prog_finish, 0);
	timeval_subtract(&prog_diff, &prog_start, &prog_finish);
        print_stats();	
}
