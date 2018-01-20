#ifndef _PITHINGS_H
#define _PITHINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <error.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
//#include <pigpio.h>

#define NUM_SPACES 32
#define CLASS_VAR 1
#define CLASS_CLASS 2
#define CLASS_TYPE 3

#define SPACE_ROOT 0
#define SPACE_FLOAT 1
#define SPACE_INT 2
#define SPACE_STR 3


#define NUM_SOCKS 16
#define NUM_CMDS 16
#define NUM_HAND 16
#define INSIZE 1024
#define OUTSIZE 1024

#define STATE_IN_NORM 0
#define STATE_IN_REP 1
#define STATE_IN_CMD 2
#define STATE_IN_HTTP 3

struct iosock;

struct node {
  struct node *prev;
  struct node *next;
  char *addr;
  int port;
  int fd;
  struct iosock *in;
};

// OK the space becomes the real thing
// A space can have kids and attributes
// all of which are also "spaces"  
struct space {
  char *name;
  char *desc;
  int idx;        // local index
  int ridx;       // remote index
  struct space *next;
  struct space *prev;
  struct space *parent;
  struct space *child;  // multispace
  struct space *attr;  // here are things about this item 
  struct space *class; // attrs can be given to classes
  struct space *clone; // here are copies of this item
  char *value;
  int ival;
  float fval;
  char *cval;
  int type;
  struct node *node;
  int (*onset)(struct space *this, int idx, char *name, char * value);
  int (*onget)(struct space *this, int idx, char *name);

};

struct iobuf;

struct iobuf
{
  struct iobuf *prev;
  struct iobuf *next;
  char *outbuf;
  int outsize;
  int outlen;
  int outptr;
};


struct iosock
{
  int fd;
  unsigned int inbptr;
  unsigned int inblen;
  unsigned int outbptr; // num sent
  unsigned int outblen; // num to send
  struct iobuf *iobuf;  // output buffers
  struct iobuf *inbuf;  // input buffers
  int cmdlen;   // number of bytes left for curent command
  int cmdbytes;   // number of bytes expected curent command
  int hlen;
  int hidx;
  char *cmdid;    // current command id
  int tlen;       // term
  int nosend;       // term
  int instate;
};

struct cmds
{
  char *key;
  char *cmd;
  char *desc;
  struct space *(*new_hand)(struct space **b, char *name, struct iosock *in);
  int (*handler)(void *key, int n, char *cmd, int speed, int time);
};

struct node;
struct hands
{
  char *key;
  char *cmd;
  char *desc;
  int(*new_hand)(int fd, char *id, char *buf, int len);
  int (*handler)(void *key, int n, char *cmd, int speed, int time);
};

int accept_socket(int sockfd);
int add_socket(int sockfd);
int connect_socket(int portno, char *addr);

int init_iosock(struct iosock *in);
char *get_space(struct space * base, char *name);
struct space *get_space_in(struct space ** base, char *name, struct iosock *in);
struct space *show_space_in(struct space ** base, char *name, struct iosock *in);
struct space *decode_cmd_in(struct space ** base, char *name, struct iosock *in);
struct space *decode_rep_in(struct space ** base, char *name, struct iosock *in);
struct space *help_new_gcmds(struct space ** base, char *name, struct iosock *in);
struct space *help_new_cmds(struct cmds * cmds, int n,struct space ** base, char *name, struct iosock *in);
struct space *cmd_quit(struct space **base, char *name, struct iosock *in);

int init_new_cmd(struct cmds *cmds, int n, char *key, char *desc, struct space *(*hand)
		 (struct space ** base, char *name, struct iosock *in));
int init_new_gcmd(char *key, char *desc, struct space *(*hand)
		 (struct space ** base, char *name, struct iosock *in));
int init_new_hcmd(char *key, char *desc, struct space *(*hand)
		 (struct space ** base, char *name, struct iosock *in));

int run_new_cmd(struct cmds *cmd, int n,char *key, struct space **base, char *name, struct iosock *in);

int run_new_gcmd(char *key, struct space **base, char *nam, struct iosock *in);
int run_new_hcmd(char *key, struct space **base, char *nam, struct iosock *in);

int run_new_hand (char *key, int fd, char *buf, int len);
int init_new_hand(char *key, char *desc, int(*hand)
		  (int fd, char *id,char *buf, int len));


int set_space(struct space * base, char *name);
struct space *set_space_in(struct space **base, char *name, struct iosock *in);

int insert_space(struct space *parent, struct space *space);
struct space *add_space(struct space **root, char *name);
int add_child(struct space *base, struct space *child);

int show_spaces(struct iosock *in, struct space *base, char *desc, int indent);
int parse_stuff(char delim, int num, char **vals, char *stuff, char stop);

int parse_name(int *idx, char **valx, int *idy , char **valy, int size, char *name);
int free_stuff(int num, char **vals);

struct iobuf *new_iobuf(int len);
int in_snprintf(struct iosock *in, struct iobuf *iob,const char *fmt, ...);
int iob_snprintf(struct iobuf *iob, const char *fmt, ...);
int find_parents(struct space* node, struct space **list, int num, int max);


typedef unsigned char uint8_t;

#define BIT(bit) (1 << (bit))

/* assign gpios to drive the shield pins */

/*      Shield      Pi */

#define MOTORLATCH  14
#define MOTORCLK    24
#define MOTORENABLE 25
#define MOTORDATA   15

#define MOTOR_3_PWM  7
#define MOTOR_4_PWM  8

/*
   The only other connection needed between the Pi and the shield
   is ground to ground. I used Pi P1-6 to shield gnd (next to D13).
*/

/* assignment of motor states to latch */

#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR4_A 0
#define MOTOR4_B 6
#define MOTOR3_A 5
#define MOTOR3_B 7

#define FORWARD  1
#define BACKWARD 2
#define BRAKE    3
#define RELEASE  4

#define NUM_IDX 1024

struct iobuf *pull_iob(struct iobuf **inp, char **bufp, int *lenp);
struct iobuf *pull_ciob(struct iobuf **inp, struct iobuf *ciob, char **bufp, int *lenp);
int push_ciob(struct iobuf **iobp, struct iobuf *ciob, struct iobuf *iob);
int init_g_spaces(void);
int init_iosocks(void);
int init_cmds(struct cmds *cmds, int n);
int init_hands(int num);
int in_new_cmds(struct cmds * cmds, int n, char * name);
int set_up_new_cmds(void);
int test_find_parents(void);
//  base = find_space_new(base, name);
struct space *copy_space_new(struct space *base, char *new_name);
int poll_sock(int lsock);
int listen_socket(int portno);
int motor_onset(struct space *this, int idx, char *name, char *value);
int motor_onget(struct space *this, int idx, char *name);
int speed_onset(struct space *this, int idx, char *name, char *value);
int speed_onget(struct space *this, int idx, char *name);
int dummy_handler(int fd, char *id, char *buf, int len);
int test_iob(void);
int test_iob_out(void);
int show_stuff(int rc, char **vals);

struct space *find_space_new(struct space *base, char *name);

int show_spaces_new(struct iosock *in, struct space **basep, char *desc, int len, char *bdesc);
struct space *find_space(struct space**parent, char *name);
struct space *new_space(char *name , struct space *parent, struct space **root_space, struct node *node);

extern int g_debug;
extern int g_debug_term;

int handle_output(struct iosock *in);
int print_iob(struct iobuf *iob);
int store_iob(struct iobuf **iobp ,  struct iobuf *iob);
int find_cmd_term(struct iosock *in, int len, int last);
int run_str_in(struct iosock *in, char *stuff, char *cmd);
int count_buf_bytes(struct iobuf *oubuf);


// node list interface
int test_nodes(void);
int remove_nodes(struct node **root);
int print_nodes(struct node *item);
int print_node(struct node *item);
int push_node(struct node **root ,  struct node *item);
int push_cnode(struct node **root, struct node *citem, struct node *item);
struct node *get_node(struct node**root, char *addr, int port);
struct node *seek_node(struct node **root, char *addr, int port);

#endif












