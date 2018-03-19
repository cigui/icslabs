/*
 * name: Zhou Xin
 * loginID: ics515030910073
 */
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

/* global variables */
static int hit_count = 0, miss_count = 0, eviction_count = 0;
static int s = 0, E = 0, b = 0;
static int S = 0;               /* S = 2 ^ s */
static char verbose = 0;        /* -v flag */
static char *tracefile = NULL;

/* macros */

/* check if the input from command line is wrong */
#define WRONGINPUT  (s<=0 || E<=0 || b<=0 || tracefile==NULL)

/* the max number of chars input from tracefile */
#define MAXINPUT 100

/* check if this line is valid using the char c */
#define VALIDLINE(c) (c == 'L' || c == 'S' || c == 'M')

/* Cache struct */
/* the struct "line" consists of its valid flag, its tag, and
 * an additional unsigned int "cnt" as a record of the last 
 * time that this line was accessed, which helps to implement
 * the LRU strategy.
 */
typedef struct {
    char valid;
    unsigned long long int tag;
    unsigned int cnt;
}line;
typedef line* set;
typedef set* cache;

/* Cache simulator */
cache CSIM;

/* print usage */
void printUsage() {
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}
    
/* 3 major functions used in main() */
void init_cache();
void run_trace(char *tracefile);
void free_cache();

/* initialize the cache with the s, E, b set. */
void init_cache() {
    int i, j;
    S = pow(2, s);
    CSIM = (set*)malloc(sizeof(set) * S);
    for (i = 0; i < S; i++) {
        CSIM[i] = (line*)malloc(sizeof(line) * E);
        for (j = 0; j < E; j++) {
            CSIM[i][j].valid = 0;
            CSIM[i][j].tag = 0;
        }
    }
}

/* free the cache */
void free_cache() {
    int i;
    for (i = 0; i < S; i++) {
        free((void *)CSIM[i]);
    }
    free((void *)CSIM);
} 

/* access the cache at the address addr */
void accessCache(unsigned long long int addr) {
    int t = (64 - b - s);             /* tag bit */
    unsigned long long int setNo = 0; /* set number */
    unsigned long long int tag = 0;
    int i;
    unsigned int maxcnt = 0;          /* the max cnt of the lines in this set */
    unsigned int mincnt = 0;
    set cur;                          /* the current set */
    int LRUline = 0;                  /* the last recently used line */
    setNo = (addr << t) >> (b + t);
    tag = addr >> (b + s);
    cur = CSIM[setNo];
    /* find the max cnt of the lines in this set */
    for (i = 0; i < E; i++) {
        if (cur[i].cnt > maxcnt) maxcnt = cur[i].cnt;
    }
    /* try to find the line needed and update its cnt */
    for (i = 0; i < E; i++) {
        if (cur[i].valid && cur[i].tag == tag) {
            hit_count++;
            cur[i].cnt = maxcnt + 1;
            if (verbose) printf("hit ");
            return;
        }
    }
    /* if didn't find the line, try to find a line not used */
    miss_count++;
    if (verbose) printf("miss ");
    for (i = 0; i < E; i++) {
        if (!cur[i].valid) {
            cur[i].valid = 1;
            cur[i].tag = tag;
            cur[i].cnt = maxcnt + 1;
            return;
        }
    }
    /* if didn't find a free line, find the LRU line and evict it */
    eviction_count++;
    if (verbose) printf("eviction ");
    mincnt = cur[0].cnt;
    for (i = 1; i < E; i++) {
        if (cur[i].cnt < mincnt) {
            mincnt = cur[i].cnt;
            LRUline = i;
        }
    }
    cur[LRUline].tag = tag;
    cur[LRUline].cnt = maxcnt + 1;
    return;
}

/* run the trace with the cache simulator */
void run_trace(char *tracefile) {
    char op = 0;                        /* the operation of the line */
    char buf[MAXINPUT];
    unsigned long long int addr = 0;    /* the address to access */
    unsigned int len = 0;
    FILE *trace = fopen(tracefile, "r");

    if (!trace) {
        printf("%s: No such file or directory\n", tracefile);
        exit(1);
    }

    while (fgets(buf, MAXINPUT, trace) != NULL) { /* read line from the file */
        op = buf[1];
        if (VALIDLINE(op)) {
            /* read the address and the length from the line */
            sscanf(buf+3, "%llx,%u", &addr, &len);
            if (verbose) printf("\n%c %llx,%u ", buf[1], addr, len);
            /* if it's a "load" or "store", access the cache once */
            if (op == 'L' || op == 'S') {
                accessCache(addr);
            }
            /* if it's a "modify", access the cache twice */
            else if (op == 'M') {
                accessCache(addr);
                accessCache(addr);
            }
        }
    }

    fclose(trace);
}

int main(int argc, char** argv) {
    /* parse command line */
    int ch;  
    opterr = 0;  
    while ((ch = getopt(argc, argv, "hvs:E:b:t:"))!=-1) {  
        switch(ch){  
        case 'h':
            printUsage();
            return 0;
        case 'v': /* verbose flag */
            verbose = 1;
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            tracefile = optarg;
            break;
        default:  
            printUsage();
            return 1;
        }
    }
    if (WRONGINPUT) {
        printf("./csim: Missing required command line argument\n");
        printUsage();
        return 1;
    }
    /* initialize the cache */
    init_cache();
    /* run the trace with the cache */
    run_trace(tracefile);
    /* free the cache */
    free_cache();
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
