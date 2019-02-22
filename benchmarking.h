/**
 * Name: Jia, Yichen
 * Project : PA-1 (Simplebench)
 * Instructor: Dr. Feng Chen
 * Class: cs7700-sp18
 * LogonID: cs770095
 * This file is the header file that define all the data structure used in the main file.
 * */


#ifndef _BENCKMARKING_H_
#define _BENCKMARKING_H_
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include <cstdio>
#include <string>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <sys/mman.h>
#include "assert.h"
#include "util.h"
#include <pthread.h>
#include <deque>
struct Settings {
    char *device;
    uint16_t duration;
    size_t range;
    size_t request_size;
    char *type;
    char *pattern;
    uint16_t iodepth;
    bool direct;
    char *outputfile;
};

struct Request {
   double timestamp;
   int thread_ID;
   char *pattern;
   char *type;
   bool direct;
   size_t size;
   off_t offset;
   double latency; 
};

struct Record {
  float timestamp;  
  int thread_ID;
  char *type;
  off_t offset;
  size_t size;
  float latency;
};
int simplebench_run(struct Settings settings);
void print_stats(void);
void log_trace(struct Record record);
#endif
