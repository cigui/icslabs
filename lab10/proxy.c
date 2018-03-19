/*
 * proxy.c - CS:APP Web proxy
 *
 * description: a web proxy program with prethreading
 * 
 * name: Zhou Xin
 * student number: 515030910073
 * email: cgcg96@sjtu.edu.cn
 * 
 */ 

#include "csapp.h"
#define LOG_FILE "proxy.log"
#define NTHREADS 16
#define SBUFSIZE 256

/*
 * conn_t - A structure used when creating a new thread, containing
 *          necessary information for function doit().
 */
typedef struct {
    int fd;
    struct sockaddr_in addr;
} conn_t;

/* 
 * sbuf_t - A structure used to implement prethreading
 */
typedef struct {
    conn_t *buf;        /* Buffer array */
    int n;              /* Maximum number of slots */
    int front;          /* buf[(front+1)%n] is first item */
    int rear;           /* buf[rear%n] is last item */
    sem_t mutex;        /* Protects accesses to buf */
    sem_t slots;        /* Counts available slots */
    sem_t items;        /* Counts available items */
} sbuf_t;


/* Global Variables */
sem_t openmutex, logmutex;
static FILE *logfile;
sbuf_t sbuf;

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

ssize_t Rio_readn_w(int fd, void *usrbuf, size_t nbytes);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_writen_w(int fd, void *usrbuf, size_t n);
int open_clientfd_ts(char *hostname, int port);
static void *doit(conn_t *vargp);
static void *thread(void *vargp);
static void handler(int sig);
static void sbuf_init(sbuf_t *sp, int n);
static void sbuf_deinit(sbuf_t *sp);
static void sbuf_insert(sbuf_t *sp, conn_t item);
static conn_t sbuf_remove(sbuf_t *sp);

/* sbuf functions */
/* Create an empty, bounded, shared FIFO buffer with n slots */
static void sbuf_init(sbuf_t *sp, int n) {
    sp->buf = Calloc(n, sizeof(conn_t));
    sp->n = n;                          /* Buffer holds max of n items */
    sp->front = sp->rear = 0;           /* Empty buffer iff front == rear */
    Sem_init(&sp->mutex, 0, 1);         /* Binary semaphore for locking */
    Sem_init(&sp->slots, 0, n);         /* Initially, buf has n empty slots */
    Sem_init(&sp->items, 0, 0);         /* Initially, buf has zero data items */
}

/* Clean up buffer sp */
static void sbuf_deinit(sbuf_t *sp) {
    Free(sp->buf);
}

/* Insert item onto the rear of shared buffer sp */
static void sbuf_insert(sbuf_t *sp, conn_t item) {
    P(&sp->slots);                          /* Wait for available slot */
    P(&sp->mutex);                          /* Lock the buffer */
    sp->buf[(++sp->rear)%(sp->n)] = item;   /* Insert the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->items);                          /* Announce available item */
}

/* Remove and return the first item from buffer sp */
static conn_t sbuf_remove(sbuf_t *sp) {
    conn_t item;
    P(&sp->items);                          /* Wait for available item */
    P(&sp->mutex);                          /* Lock the buffer */
    item = sp->buf[(++sp->front)%(sp->n)];  /* Remove the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->slots);                          /* Announce available slot */
    return item;
}


/* 
 * Rio_readn_w - A wrapper for rio_readn that doesn't terminate the process
 *               when it encounters an error
 */
ssize_t Rio_readn_w(int fd, void *ptr, size_t nbytes) { 
    ssize_t n;
  
    if ((n = rio_readn(fd, ptr, nbytes)) < 0) {
        return 0;
    }
    return n;
}

/* 
 * Rio_readlineb_w - A wrapper for rio_readlineb that doesn't terminate the 
 *                   process when it encounters an error
 */
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen) {
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0) {
        return 0;
    }
    return rc;
}

/* 
 * Rio_writen_w - A wrapper for rio_writen that doesn't terminate the process
 *                when it encounters an error
 */
void Rio_writen_w(int fd, void *usrbuf, size_t n) {
    if (rio_writen(fd, usrbuf, n) != n){
        return;
    }
}

/*
 * open_clientfd_ts - A thread-safe version of open_clientfd
 */
int open_clientfd_ts(char *hostname, int port) {
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    return -1; /* check errno for cause of error */

    /* Fill in the server's IP address and port */
    P(&openmutex);
    if ((hp = gethostbyname(hostname)) == NULL)
	    return -2; /* check h_errno for cause of error */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0], 
	        (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);
    V(&openmutex);

    /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
	    return -1;
    return clientfd;
}

/* 
 * handler - signal handler for SIGINT
 */
static void handler (int sig) {
    sbuf_deinit(&sbuf);
    exit(0);
}

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv) {
    /* To keep the proxy from crashing */
    Signal(SIGPIPE, SIG_IGN);
    /* Variables */
    int listenfd, port, i;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    pthread_t tid;
    conn_t *conn;
    logfile = fopen(LOG_FILE, "a");
    Sem_init(&logmutex, 0, 1);
    Sem_init(&openmutex, 0, 1);
    /* Check arguments */
    if (argc != 2) {
	    fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	    exit(0);
    }
    port = atoi(argv[1]);
    sbuf_init(&sbuf, SBUFSIZE);
    signal(SIGINT, handler);
    listenfd = Open_listenfd(port);

    for (i = 0; i < NTHREADS; i++)
        Pthread_create(&tid, NULL, thread, NULL);
    while (1) {
        conn = Malloc(sizeof(conn_t));
        conn->fd = Accept(listenfd, (SA *)&(conn->addr), &clientlen);
        sbuf_insert(&sbuf, *conn);
    }
    exit(0);
}

/*
 * thread - thread function for the proxy program
 */
static void *thread(void *vargp) {
    Pthread_detach(pthread_self());
    while (1) {
        conn_t conn = sbuf_remove(&sbuf);
        doit(&conn);
        close(conn.fd);
    }
    return NULL;
}

/* 
 * doit - main function for the thread
 */
static void *doit(conn_t *vargp) {
    /* Variables */
    conn_t *conn = vargp;
    int fd = conn->fd;
    struct sockaddr_in *sockaddr = &(conn->addr);
    int port, clientfd, size = 0;
    char buf[MAXLINE], host[MAXLINE], path[MAXLINE];
    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char reqstring[MAXLINE], logstring[MAXLINE];
    ssize_t nbytes;
    rio_t rioc, rios; // rioc for client, rios for server

    /* Read the first line of the request */
    Rio_readinitb(&rioc, fd);
    Rio_readlineb_w(&rioc, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    parse_uri(uri, host, path, &port);
    strcpy(reqstring, buf);
    /* Read the remaining lines of the request */
    while (strcmp(buf, "\r\n")) {
        if (buf[17] == 'g' && buf[18] == 'z' && buf[19] == 'i'
                && buf [20] == 'p') continue;
        nbytes = Rio_readlineb_w(&rioc, buf, MAXLINE);
        /* store the request in a string instead of writing it instantly */
        strcat(reqstring, buf);
    }
 // fprintf(logfile, "%s", reqstring); // print the request string for debug
 // fflush(logfile);

    /* Send request */
    clientfd = open_clientfd_ts(host, port);
    Rio_writen_w(clientfd, reqstring, sizeof(reqstring));

    /* Read response & Write back to client */
    Rio_readinitb(&rios, clientfd);
    while ((nbytes = Rio_readlineb_w(&rios, buf, MAXLINE)) != 0) {
        Rio_writen_w(fd, buf, strlen(buf));
        size += nbytes;
    }

    /* Logging */
    format_log_entry(logstring, sockaddr, uri, size);
    P(&logmutex);
    fprintf(logfile, "%s", logstring);
    fflush(logfile);
    V(&logmutex);

    close(clientfd);

    return NULL;
}

/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port) {
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
	    hostname[0] = '\0';
	    return -1;
    } 
       
    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';
    
    /* Extract the port number */
    *port = 80; /* default */
    if (*hostend == ':')   
	*port = atoi(hostend + 1);
    
    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
	pathname[0] = '\0';
    }
    else {
	pathbegin++;	
	strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size) {
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d\n", time_str, a, b, c, d, uri, size);
}


