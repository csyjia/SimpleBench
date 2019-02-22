/**
 * Name: Jia, Yichen
 * Project: PA-1 (Simplebench)
 * Instructor: Dr. Feng Chen
 * Class: cs7700-sp18
 * LogonID: cs770095
 * This is the main file that is used to initialize all the settings and start the corresponding working threads.
 ***/
#include "benchmarking.h"
#define KB 1024
#define MB 1048576 
struct Settings settings;

	
static void 
simplebench_set_default_options(void)
{
   settings.device = NULL;	
   settings.duration=60; // the duration si 60 seconds. 
   settings.range=100*MB; // the scan range is 100MB
   settings.request_size=4*KB; // the request size is 4KBs
   settings.type="R";// 0 means read, 1 means write.
   settings.pattern="S"; // 0 means sequential, 1 means random.
   settings.iodepth=1; // the number of threads.
   settings.direct=false; // use the direct_io
   settings.outputfile = NULL ; 
}

static void
simplebench_show_usage(void)
{
    printf(
	"Usage: simplebench [-T duration]\n" \
	" [-f device or file] [-r range (MBs)]\n" \
	" [-s request size (KBs)] [-t Type(R or W)]\n" \
	" [-p pattern (S or R)] [-q number of threads]\n" \
	" [-d direct IO (T or F)] [-o output trace]\n" \
	" ");
}

static char short_options[]=
	"T:"
	"f:"
	"r:"
	"s:"
	"t:"
	"p:"
	"q:"
	"d:"
	"o:"
	;

static int
simplebench_get_options(int argc, char **argv)
{
     int c, value;
     char *pos;
     opterr=0;
     for (;;){
         c=getopt(argc, argv, short_options);
	 if(c==-1){
	    /* no more options */
	    break;
	 }
	 switch(c){
		case 'T':
			value = atoi(optarg);
			if (value<=0){
			   printf("simplebench: option -T requires a non zero number");
			   return 0;
			}
			settings.duration=value;
			break;
		case 'f':
			settings.device=optarg;
			break;
		case 'r':
			value = atoi(optarg);
			if(value<=0){
			   printf("simplebench: option -r requires a non zero number");
			   return 0;
			}
			settings.range=value*MB;
			break;
		case 's':
			value = atoi(optarg);
			if(value<=0){
			   printf("simplebench: option -s requires a non zero number");
			   return 0;
			}
			if(value>settings.range){
			   printf("simplebench: option -s requires the request size smaller than the scan range");
			   return 0;
			}
			settings.request_size=value*KB;
			break;
		case 't':
			if((!strcmp(optarg,"W")) || (!strcmp(optarg,"R"))){
			    settings.type = optarg;
			} else {
			    return 0;
			}
			break;
		case 'p':
			if((!strcmp(optarg,"S")) || (!strcmp(optarg,"R"))){
	   		    settings.pattern = optarg;
			} else {
			    return 0;
			}
			break;
		case 'q':
			value = atoi(optarg);
			if(value<=0){
			   printf("simplebench: option -q requires a positive integer number");
			   return 0;
			}
			settings.iodepth=value;
			break;
		case 'd':
			if(!strcmp(optarg,"T")){
                             settings.direct=true;
                        } 
                        if(!strcmp(optarg,"F")){
                	     settings.direct=false;
                        }
			break;
		case 'o':
			settings.outputfile=optarg;
			break;
		default:
			printf("simplebench: invalid options -- '%c'", optopt);
			return 0;
	 }
     }
     return 1;
}

int main(int argc, char *argv[], char *envp[])
{
      int status = 1;

      simplebench_set_default_options();
      status = simplebench_get_options(argc, argv);
      if (status != 1){
            simplebench_show_usage();
	    exit(1);
      }	    

      /**
       * Line buffer stdout to avoid mixed output from multiple threads.
       * **/
      setvbuf(stdout, NULL, _IOLBF, 0);
      //simplebench_time_init();// To initialize the time.

      if(status) {
              //set_genesis_time();

	      status = simplebench_run(settings);
	      if(status!=1){
	              exit(1);
	      }
      } else 
	      //ret=simplebench_backend(NULL);

      //deinitialize_simplebench();
      return status;
}

