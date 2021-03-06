/* from
http://abyz.me.uk/rpi/pigpio/ex_motor_shield.html
cc -o pimotor pimotor.c -lpigpio -lpthread -lrt
*/

/*
 for now forget the motor stuff
 we aim to build up a repository of spaces
 a space has a name, 

 has some text value int value or float value
 things can be classes, lists or types
 ADD foo data float 2.3456
 ADD foo1 data string  "this is foo 1"
 SHOW foo
 SHOW data
 
 well we need something called spaces
 spaces contain lists of things
 a space can live on a node !!
 a space can be copied or cloned
 a cloned space will sync with its parent
 a parent will keep track of its clones
 we'll make the number of spaces dynamic
 
  Big jump now  make multi/space/possible 


API thoughts
basic 
lreg /me/
greg /me/
add foo/boo/fum
find foo/boo/fum
show foo/boo/fum
set foo/boo/fum value
onget foo/boo/fum <function>
onset foo/boo/fum <function>
show foo/boo/fum
showall foo/boo/fum

notes
   in all cases the address foo/ refers to the local node
   and  the /node/foo will refer the request to a remote node

lreg /me/
   this is my root am on the local node
   just for this app
   i can talk to ant other apps

greg /me/ <node>  
   this is who I am on the system based on node re report in to the server

add foo/boo/fum
   add this item to my local list

find foo/boo/fum
   find this item on my local list

get foo/boo/fum
   get the value of foo/boo/fum locally

set foo/boo/fum value
   set the value of foo/boo/fum locally

onget foo/boo/fum <function>
   run this function when getting the value

onset foo/boo/fum <function>
   run this function when setting the value

show foo/boo/fum
   shows details of the variable

showall foo/boo/fum
   shows details of all the child variables


message details
ascii messages are terminated with two cr's
CMD xxx  yyy0xa0xa
This message will set up a fixed length message
The contents will be read until we get yyy chars from the socket
A process will register with the id yyyy and when we get a response for that message it will be preceeded with :

REP xxx zzzz0xa0xa
This means that we will get a reply of zzzz bytes and send that to the process
with an id of xxx
look at handle_input
Sending a command to a remte
See the "send" thing
First send [CMD RAW lll\n\n]
Then send the command specified in the command line
The remote will detect the [CMD RAW lll\n\n] request and then wait for lll bytes.
The run_str_in will put stuff in the iobuf to send as usual but we'll turn off
the actual output is prevented by setting the nosend flag to 1;
Once run_str_in has completed we can calculate the output bytes and then send the 
[RSP xxx lll\n\n] with the buffer length and then simply turn off the nosend flag
*/

#if 0

http decode
we get this

accept ret = 4 
find_cmd_term checking G 47 rc 1 lend 88
find_cmd_term checking E 45 rc 2 lend 87
find_cmd_term checking T 54 rc 3 lend 86
find_cmd_term checking   20 rc 4 lend 85
find_cmd_term checking / 2f rc 5 lend 84
find_cmd_term checking u 75 rc 6 lend 83
find_cmd_term checking a 61 rc 7 lend 82
find_cmd_term checking v 76 rc 8 lend 81
find_cmd_term checking 1 31 rc 9 lend 80
find_cmd_term checking ? 3f rc 10 lend 79
find_cmd_term checking v 76 rc 11 lend 78
find_cmd_term checking e 65 rc 12 lend 77
find_cmd_term checking r 72 rc 13 lend 76
find_cmd_term checking = 3d rc 14 lend 75
find_cmd_term checking 1 31 rc 15 lend 74
find_cmd_term checking   20 rc 16 lend 73
find_cmd_term checking H 48 rc 17 lend 72
find_cmd_term checking T 54 rc 18 lend 71
find_cmd_term checking T 54 rc 19 lend 70
find_cmd_term checking P 50 rc 20 lend 69
find_cmd_term checking / 2f rc 21 lend 68
find_cmd_term checking 1 31 rc 22 lend 67
find_cmd_term checking . 2e rc 23 lend 66
find_cmd_term checking 1 31 rc 24 lend 65
 d rc 25 lend 64ecking 
find_cmd_term checking 
 a rc 26 lend 63
 handle_input_norm message received [GET /uav1?ver=1 HTTP/1.1
Host: 127.0.0.1:5432
User-Agent: curl/7.47.0
Accept: */*

] -> n 3 cmd [GET] uri [/uav1?ver=1] vers [HTTP/1.1]
 run_str_http >>>> cmd [GET] sp [GET /uav1?ver=1 HTTP/1.1
Host: 127.0.0.1:5432
User-Agent: curl/7.47.0
Accept: */*

]
 parse_name 1 name [/uav1?ver=1] *idx 1 valx[0/1] 0x41d010/0x7fdd26df60
 parse_name 2 name [/uav1?ver=1] *idy 1 valy[0/1] 0x41d080/0x7f9cf816f0 [/uav1]-[none]
 looking for [/uav1] [uav1] found [uav1] i 0 idx/v 1/1
 find_space_new we found it [uav1]

find_cmd_term checking 1 31 rc 15 lend 74
find_cmd_term checking : 3a rc 16 lend 73
find_cmd_term checking 5 35 rc 17 lend 72
find_cmd_term checking 4 34 rc 18 lend 71
find_cmd_term checking 3 33 rc 19 lend 70
find_cmd_term checking 2 32 rc 20 lend 69
 d rc 21 lend 68ecking 
find_cmd_term checking 
 a rc 22 lend 67
 handle_input_norm message received [Host: 127.0.0.1:5432
User-Agent: curl/7.47.0
Accept: */*

] -> n 3 cmd [Host:] uri [127.0.0.1:5432] vers [User-Agent:]
 run_str_http >>>> cmd [Host:] sp [Host: 127.0.0.1:5432
User-Agent: curl/7.47.0
Accept: */*

]
[...]
  
find_cmd_term checking 7 37 rc 21 lend 68
find_cmd_term checking . 2e rc 22 lend 67
find_cmd_term checking 0 30 rc 23 lend 66
 d rc 24 lend 65ecking 
find_cmd_term checking 
 a rc 25 lend 64
 handle_input_norm message received [User-Agent: curl/7.47.0
Accept: */*

] -> n 3 cmd [User-Agent:] uri [curl/7.47.0] vers [Accept:]
 run_str_http >>>> cmd [User-Agent:] sp [User-Agent: curl/7.47.0
Accept: */*

]

find_cmd_term checking : 3a rc 7 lend 82
find_cmd_term checking   20 rc 8 lend 81
find_cmd_term checking * 2a rc 9 lend 80
find_cmd_term checking / 2f rc 10 lend 79
find_cmd_term checking * 2a rc 11 lend 78
 d rc 12 lend 77ecking 
find_cmd_term checking 
 a rc 13 lend 76
 handle_input_norm message received [Accept: */*

] -> n 2 cmd [Accept:] uri [*/*] vers []
 run_str_http >>>> cmd [Accept:] sp [Accept: */*

]*/
 d rc 1 lend 88hecking 
find_cmd_term checking 
 a rc 2 lend 87
 handle_input_norm message received [
] -> n -1 cmd [] uri [] vers []
 run_str_http >>>> cmd [] sp [
]
 handle_input_norm reset buffers tlen = 2 ptr/len 0/0
					     
				     
or

run_str_http >>>> cmd [Accept:] sp [Accept:
				    
Content-Length: 4
Content-Type: application/x-www-form-urlencoded

1234]

1234] -> n 3 cmd [Content-Length:] uri [4] vers [Content-Type:]
 run_str_http >>>> cmd [Content-Length:] sp [Content-Length: 4
Content-Type: application/x-www-form-urlencoded

1234]
#endif
  

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
  int idx;
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


//int num_socks=0;
int g_num_socks=0;
int g_debug = 0;
int g_lsock = 0;
int g_quit_one = 0;

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

struct iosock g_iosock[NUM_SOCKS];
struct cmds g_cmds[NUM_CMDS];
struct cmds h_cmds[NUM_CMDS];
struct hands hand[NUM_HAND];
#define NUM_IDX 1024
struct space *space=NULL;
struct space *g_space=NULL;
int g_space_idx = 1;
struct space *g_spaces[NUM_IDX];
struct iobuf *g_iob_store = NULL;
int g_debug_term = 1;

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

int test_find_parents(void)
{
  int num=0;
  int rc;
  int i;
  struct space *slist[64];
  struct space *sp1 = add_space(&g_space, "ADD uav1/motor1/speed");
  for (i = 0 ; i < 64; i++)
    slist[i] = NULL;
  num = find_parents(sp1,slist,num,64);
  printf("%s sp1 [%s] parent %p rc %d num %d\n"
	 , __FUNCTION__
	 , sp1->name
	 , sp1->parent
	 , rc
	 , num
	 );
  if (num > 0)
    {
      for ( i = num-1; i >=0; i--)
	{
	  printf("%d [%s] ", i, slist[i]?slist[i]->name?slist[i]->name:"NONAME":"NONODE");
	}
      printf("\n");
      
    }
  return rc;
}

int find_parents(struct space* node, struct space **list, int idx, int max)
{
  int rc = 1;
  list[idx++] = node;
  if (node->parent)
    {
      rc += find_parents(node->parent, list, idx, max);
    }
  return rc ;
}


struct space *setup_space(char *name, struct space*parent)
{
  struct space *space = calloc(sizeof(struct space), 1);

  space->idx = g_space_idx++;
  space->parent = parent;

  space->name = strdup(name);

  space->attr = NULL;
  space->class = NULL;
  space->clone = NULL;
  space->child = NULL;
  space->desc = NULL;
  space->value = NULL;
  space->node = NULL;

  space->prev = space;
  space->next = space;
  space->onset = NULL;
  space->onget = NULL;

  space->type = SPACE_ROOT;
  if(space->idx < NUM_IDX)
    {
      g_spaces[space->idx] = space;
    }
  return space;
}

struct space *new_space(char *name , struct space *parent, struct space **root_space, struct node *node)
{
  struct space *space;
  struct space *root = NULL;
  struct space *last_space;
  printf(" new space [%s] root %p\n"
	 , name
	 , root_space
	 );
  space = setup_space(name, parent);

  if(node)
    {
      space->node = node;
    }

  // insert in parent list
  if(parent)
    {
	  last_space = parent->prev;
	  last_space->next = space;
	  space->prev =  parent->prev;
	  space->next =  parent;
	  parent->prev = space;
    }
  else
    {
#if 0
      if(root_space)
	{
	  root = *root_space;
	  if(root)
	    {
	      last_space = root->prev;
	      last_space->next = space;
	      space->prev =  root->prev;
	      space->next =  root;
	      root->prev = space;
	    }
	  else
	    {
	      *root_space = space;
	      space->next = space;
	      space->prev = space;
	    }
	}
      else
	{
	  g_space = space;
	  space->next = space;
	  space->prev = space;
	}
#endif
    }
  return space;
}  

struct space *new_space_class(char *name , struct space *parent)
{
  int i;
  struct space *space;
  struct space *class;
  struct space *last_space;
  printf(" new space class [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );
  class = parent->class;
  space = setup_space(name, parent);

  if(class)
    {
	  last_space = class->prev;
	  last_space->next = space;
	  space->prev =  class->prev;
	  space->next =  class;
	  class->prev = space;
    }
  else
    {
      parent->class = space;
    }
  return space;
}  

struct space *new_space_attr(char *name , struct space *parent)
{
  int i;
  struct space *space;
  struct space *attr;
  struct space *last_space;
  printf(" new space attr [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );
  attr = parent->attr;
  space = setup_space(name, parent);


  if(attr)
    {
	  last_space = attr->prev;
	  last_space->next = space;
	  space->prev =  attr->prev;
	  space->next =  attr;
	  attr->prev = space;
    }
  else
    {
      parent->attr = space;
    }
  return space;
}  

struct space *new_space_attr_str(char *name, struct space *parent, char *val)
{

  struct space *space = NULL;
  printf(" new space attr_str [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );

  space = new_space_attr(name, parent);
  if(space)
    {
      space->cval = val;
      space->type = SPACE_STR;

    }
  return space;
}

struct space *new_space_attr_float(char *name, struct space *parent, float val)
{
  struct space *space = NULL;
  printf(" new space attr_float [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );
  space = new_space_attr(name, parent);
  if(space)
    {
      space->fval = val;
      space->type = SPACE_FLOAT;

    }
  return space;
}

struct space *new_space_attr_int(char *name, struct space *parent, int val)
{

  struct space *space = NULL;
  printf(" new space attr_int [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );
  space = new_space_attr(name, parent);
  if(space)
    {
      space->ival = val;
      space->type = SPACE_INT;

    }

}

int show_space_class(struct iosock *in,struct space *base, int indent)
{
  char atname[128];
  int len;
  snprintf(atname, sizeof(atname),"  %s Class", base->name); 
  if(base->class)
  {
    len = show_spaces(in, base->class, atname, indent);
  }
  return len;
}

int show_space_attr(struct iosock *in,struct space *base, int indent)
{
  char atname[128];
  int len=0;
  snprintf(atname, sizeof(atname),"  %s Attr", base->name); 
  if(base->attr)
  {
    len = show_spaces(in,base->attr, atname, indent);
  }
  return len;
}

int show_space(struct iosock *in, struct space *base, int indent)
{
  int rc=-1;

  if(!base)return rc;
  if(!in)return rc;
 
  rc =  indent;
  while(rc--)
    {
	printf(" ");
    }
  printf(" %p space %03d name [%s] node [%p] next name [%s] prev name [%s]\n"
	 , base
	 , base->idx
	 , base->name
	 , base->node
	 , base->next->name
	 , base->prev->name
	 );
  show_space_attr(in,base, indent+3);

  show_space_class(in,base, indent+5);
  return rc;
}


int show_spaces(struct iosock *in, struct space *base, char *desc, int indent)
{
  struct space *start=base;
  int rc = 0;
  in_snprintf(in, NULL, "spaces ... %s\n", desc ? desc:" ");

  while (base)
    {
      rc = show_space(in, base, indent);
      if(base->child) 
	{
	  rc =  show_spaces(in, base->child,"child",indent+2);
	}
      if(base->next != start)
	base=base->next;
      else
	base =  NULL;
    }
  return rc;
}

int show_space_new(struct iosock *in, struct space *base, char *desc, int len, char *bdesc)
{
  struct space *start=base;
  struct space *child=NULL;
  int ret;
  int rc=-1;
  char *sp = desc;
  int slen = len;

  if(!base)return rc;
  if(!desc)return rc;
  if(len == 0)return rc;
  //  sp += strlen(desc);

  child = base->child;
  if(child == NULL)
    {
      snprintf(sp, slen,
	       "/%s => %d"
	       , base->name
	       , base->idx
	       );
    }
  else
    {
      snprintf(sp, slen,
	       "/%s"
	       , base->name
	       );
    }
  if(child == NULL)
    {
      in_snprintf(in, NULL," >> [%s]\n", bdesc);
      //desc[0]=0;
      //printf(
      return 0;
    }
  // foreach child do the same
  slen -= strlen(sp);
  sp += strlen(sp);
  //  sp += strlen(sp);
  while (child != NULL)
    {

      slen += show_space_new(in,child, sp, slen, bdesc);
      child = child->next;
      if (child == base->child) 
	child= NULL;
    }
  //  ret =  strlen(buf);
  ret =  slen;
  return ret;
}

int show_spaces_new(struct iosock *in, struct space **basep, char *desc, int len, char *bdesc)
{
  int rc  = -1;
  struct space *base=*basep;
  struct space *start=base;
  struct space *xstart=NULL;
  while (start)
    {
      rc = show_space_new(in, start, desc, len, bdesc);
      if(g_debug)
	printf(" >> [%s]\n", desc);
      if(in)(in, NULL, ">>[%s]\n", desc);
      xstart = start->next;
      start = start->next;

      if(start == base) 
	start =  NULL;
      //start =  NULL;
    }
  //printf(" base [%s] %d @ %p \n", base->name, base->idx, base);
  //if(xstart)
  //printf(" next [%s] %d @ %p \n", xstart->name, xstart->idx, xstart);

 return rc;
}

struct space *find_space_new(struct space *base, char *name)
{
  struct space *start;
  char *sp = name;
  char *spv = NULL;
  int i;

  int rc;
  int idx = 0;
  int idv = 0;
  char *valv[64];
  char *valx[64];
  rc = parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);
  // now find the space at each step
  i = 0;
  start = base;
  while (base)
    {
      spv = valv[i];
      if(*spv == '/')spv++;
      if(1)
	printf(" looking for [%s] [%s] found [%s] i %d idx/v %d/%d\n"
	       , valv[i], spv, base->name
	       , i
	       , idx
	       , idv
		  );
      if(strcmp(base->name, spv)==0)
	{

          if(i < idv) i++;
	  if(i == idv)
	    {
	      free_stuff(idv, valv);
	      free_stuff(idx, valx);
	      printf(" %s we found it [%s]\n", __FUNCTION__, base->name);
	      return base;
	    }
	  base =  base->child;
	  start = base;
	  //break;
	}
      else
	{
	  if(base->next != start)
	    base=base->next;
	  else
	    {
	      if(i < idv) i++;
	      if(i == idv)
		{
		  free_stuff(idv, valv);
		  free_stuff(idx, valx);
		  return NULL;
		}
	      base =  base->child;
	      start = base;
	    }
	}
    }
  return base;
}


int add_child(struct space *parent, struct space *child)
{
  if(parent->child == NULL)
    parent->child = child;
  else
    insert_space(parent->child, child);
  child->parent = parent;
  return 0;
}

// create a copy of the space if any at name
//  base = find_space_new(base, name);
struct space *copy_space_new(struct space *base, char *new_name)
{
  struct space *start;
  struct space *sp1;
  struct space *sp2;
  struct space *space= NULL;

  space = setup_space(new_name, NULL);

  start = base->child;
  sp1 = base->child;
  while(sp1)
    {
      sp2 = copy_space_new(sp1, sp1->name);
      add_child(space, sp2);
      sp1=sp1->next;
      if (sp1 == start)
	sp1 = NULL;
    }
  return space;
}

struct space *find_space(struct space**parent, char *name)
{
  struct space *base;
  struct space *start;

  start = g_space;
  base = g_space;
  if(parent)
    {
      start = *parent;
      base = *parent;
      if(g_debug)
	printf(" >>> find space [%s] parent name [%s] ... \n", name
	       ,start?start->name ? start->name:"No name":"No start");
    }
  else
    {
      if(g_debug)
	printf(" >>> find space name [%s] ... \n", name);
    }
  while (base)
    {
      if(strcmp(base->name, name)==0)
	{
	  break;
	}
      
      if(base->next != start)
	base=base->next;
      else
	base =  NULL;
    }
  
  if(g_debug)
    printf(" >>> find space name [%s] space %p... \n", name, base);
  return base;
}

/*
before any s0->p = s0 s0->n=s0
after add s1 s0->p = s1 s0->n = s1 s1->p = s0 s1->n=s0
after add s2 s0->p = s2 s0->n = s1 s1->p = s0 s1->n=s2 s2->p = s1 s2->n=s0
 */
int insert_space(struct space *parent, struct space *space)
{
  struct space *last_space;

  last_space = parent->prev;

  if(0)printf( " add_space  parent [%s] pprev [%s] next [%s]\n"
	       , parent->name
	       , last_space->name
	       , space->name
	       );


  if(parent->next == parent) 
    {
      parent->next = space;
      parent->prev = space;
      space->next = parent;
      space->prev = parent;
    }
  else
    {
      last_space->next = space;
      space->prev = last_space;
      space->next =  parent;
      parent->prev = space;
    }
  space->parent = parent;
  return 0;
}

//   idx = parse_stuff(' ', 64, (char **)valx, name);
//   rc = parse_name(&idx (char **)valx, &idy (char **)valy, 64, name);
int parse_name(int *idx, char **valx, int *idy , char **valy, int size, char *name)
{
  int  rc = 1;
  char *sp;
  *idx = parse_stuff(' ', size, (char **)valx, name, 0);
  printf(" parse_name 1 name [%s] *idx %d valx[0/1] %p/%p\n", name, *idx
	 , valx[0], valx[1]); 

  sp = valx[0];
  if(*idx >= 2)sp = valx[1];

  *idy = parse_stuff('/', size, (char **)valy, sp,'?');
  printf(" parse_name 2 name [%s] *idy %d valy[0/1] %p/%p [%s]-[%s]\n"
	 , name, *idy
	 , valy[0], valy[1]
	 , valy[0], (*idy>1)?valy[1]:"none"
	 ); 
  return rc;
}

struct space *cmd_html_len(struct space **root, char *name,
			    struct iosock *in)
{
  int rc;
  int i;
  int idx = 0;
  char *valx[64];

  idx = parse_stuff(' ', 64, (char **)valx, name,'\n');
  in->hlen = 0;
  
  for (i = 0; i<idx; i++)
    {
      printf(" >> String %d [%s]\n", i, valx[i]);
    }
  
  if(idx>1)
    {
      rc = atoi(valx[1]);
      in->hlen = rc;
      printf(" %s setting hlen to %d\n"
	     , __FUNCTION__
	     , in->hlen
	     );
      
    }

  return NULL;
}

struct space *cmd_html_dummy(struct space **root, char *name,
			    struct iosock *in)
{
  int rc;
  int i;
  int idx = 0;
  char *valx[64];
  
  if((name[0] == 0xd) || (name[0] == 0xd))
    printf(" %s start of data from %x hlen %d\n"
	   , __FUNCTION__
	   , name[0], in->hlen);
  

  idx = parse_stuff(' ', 64, (char **)valx, name,'\n');
  //rc = parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);
  for (i = 0; i<idx; i++)
    {
      printf(" >> String %d [%s]\n", i, valx[i]);
    }

  return NULL;
}
// split up multi/space/name
// look for children of the same name 
// return found name or new space object
//    sp1 = add_space(&g_space, "ADD uav1/motor1/speed");

struct space *add_space_in(struct space **root, char *name,
			    struct iosock *in)
{
  struct space *parent=NULL;
  struct space *space=NULL;
  struct iobuf * iob;
  int cidx = 0;
  char *sp = name;
  int new = 0;
  int i;
  int rc;
  int idx = 0;
  int idv = 0;
  char *valv[64];
  char *valx[64];

  rc = parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);

  for (i = 0; i<idv; i++)
    {
      printf(" >> Space %d [%s]\n", i, valv[i]);
    }
  for (i=0 ; i<idv; i++)
    {
      space = NULL;
      space = find_space(parent?&parent->child:root, valv[i]);
      printf(" Space %d [%s] %p\n", i, valv[i], space);
      if(0)in_snprintf(in, NULL, "Seeking [%s]  %p\n", valv[i], space);
      if (!space)
	{
	  new = 1;
	  if(parent)
	    {
	      if(g_debug)
		printf(" New Space for [%s] parent->name [%s]\n", valv[i], parent->name);
	      space = new_space(valv[i], parent->child, &parent->child, NULL); 
	      add_child(parent, space);

	    }
	  else
	    {
	      if(g_debug)
		printf(" New Space for [%s] at root\n", valv[i]);
	      space = new_space(valv[i], NULL, &g_space, NULL); 
	      printf(" New Space for [%s] at root %p\n", valv[i], space);

	    }
	  if(i == 0)
	    {
	      if (*root == NULL)
		{
		  *root = space;
		}
	      else
		{
		  insert_space(*root, space);
		}
	    }
	}
      else
	{
	  if(g_debug)
	    printf(" Space found [%s]\n", valv[i]);
	}
      parent = space;
    }
  //iob_snprintf(iob1, "more stuff  the name [%s] value is %d ", "some_name", 23);

  if(new)
    in_snprintf(in, NULL, "Added [%s] added  space [%s] %d\n",name, space->name, idx);
  else
    in_snprintf(in, NULL, "Found [%s] found  space [%s] %d\n",name, space->name, idx);
  
  free_stuff(idv, valv);
  free_stuff(idx, valx);
  return space;
}

struct node *new_node(char *aport, char *addr)
{
  struct node * node;
  node = (struct node *) calloc(sizeof (struct node), 1);
  node->addr = strdup(addr);
  node->port = atoi(aport);
  node->fd = connect_socket(node->port, node->addr);;
  node->in = NULL;
  return node;
}

// struct node
// "NODE name/n/n addr port 
struct space *add_node_in(struct space **root, char *name,
			    struct iosock *in)
{
  struct space *space=NULL;
  int idv = 0;
  int idx = 0;
  char *valv[64];
  char *valx[64];
  int rc = 0;
  int i = 0;
  int fd = 0;
  struct node *node = NULL;

  rc = parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);

  for (i = 0; i<idx; i++)
    {
      printf(" >> Arg %d [%s]\n", i, valx[i]);
    }
  // connect
  space = add_space_in(root, name, in);

  if(idx >= 3)
    {
      node =  new_node(valx[3], valx[2]);
    }
  space->node =  node;
  printf(" %s space name %s idx %d fd %d @%s:%s\n"
	 , __FUNCTION__
	 , space->name
	 , idx
	 , fd
	 , valx[2]
	 , valx[3]
	 );

  return space;  
}


struct space *add_space(struct space **root, char *name)
{
  return add_space_in(root, name, NULL);
}
  
int free_stuff(int num, char **vals)
{
  int i;
  for(i = 0 ; i< num; i++)
    {
      if(vals[i])
	free(vals[i]);
      vals[i] = 0;
    }
  return 0;
}

// This will do for now
// TODO dynamic sizing
//      allow escape
//
int parse_stuff(char delim, int num, char **vals, char *stuff, char cstop)
{
  int idx = 0;
  char *sp = stuff;
  char *sp1 = stuff;
  int rc = 1;
  char *val;
  char  *spv;
  int val_size;
  int skip = 0;
  int done = 0;
  val_size = 64;
  val = malloc(64);  //TODO fix this
  val[0]=0;
  if(g_debug)
    printf("%s start stuff[%s] \n", __FUNCTION__, stuff);
  //vals[idx] = strdup(sp);
  //idx++;
  // TODO special case where *sp == delim at the start
  if(*sp && *sp == delim )
    {
      skip = 1;
    }
  while(*sp && (rc>0) && (idx < num) && !done)
    {
      rc = 0;
      spv = val;
      while (*sp && (*sp!= cstop) && ((skip == 1) || (*sp != delim)) && (rc < (val_size-1)))
	{
	  skip = 0;
	  if ((*sp != 0xa)&&(*sp != 0xd) &&(*sp != cstop))
	    {
	      rc++;
	      *spv++ = *sp++;
	    }
	  else
	    sp++;
	  if(*sp && *sp == cstop) done = 1;
  
	}
      if(*sp)sp++;
      *spv = 0;
	
      if(rc>0)
	{
	  vals[idx] = strdup(val);
	  if(g_debug)
	    printf("rc %d val [%s] val[%d] [%s] %x %x"
		   , rc
		   , val
		   , idx
		   , vals[idx]
		   , vals[idx][0]
		   , vals[idx][1]
		   );
	  if(g_debug)
	    printf("sp [%s] \n", sp);
	  //if(!skip)
	  idx++;
	  //skip = 0;
	}
    }
  if(g_debug)
    printf("%s done idx %d\n", __FUNCTION__, idx);
  free(val);
  return idx;
}

// use cmds
// HELP
// ADD       add_space_in(&g_space, stuff, in);
// SET       set_space_in(&g_space, stuff, in);
// GET       get_space_in(&g_space, stuff, in);
// SHOW      show_space_in(&g_space, stuff, in);

int set_up_new_cmds(void)
{
  int rc = 0;
  init_new_gcmd("HELP", "Print this help",             help_new_gcmds);
  init_new_gcmd("ADD",  "Create a new space",          add_space_in);
  init_new_gcmd("SET",  "Set a (string) value",	       set_space_in);
  init_new_gcmd("GET",  "Get a (string) value",	       get_space_in);
  init_new_gcmd("CMD", "determine command id and len", decode_cmd_in);
  init_new_gcmd("REP", "determine  replyid and len",    decode_rep_in);
  init_new_gcmd("SHOW", "Show spaces from a root",      show_space_in);
  init_new_gcmd("NODE", "Show spaces from a root",      add_node_in);
  init_new_gcmd("QUIT", "quit",      cmd_quit);

  init_new_hcmd("POST",            "Post",            get_space_in);
  init_new_hcmd("Host:",           "Host: Port",      cmd_html_dummy);
  init_new_hcmd("User-Agent:",     "User Agent",      cmd_html_dummy);
  init_new_hcmd("Accept:",         "Accept",          cmd_html_dummy);
  init_new_hcmd("Content-Type:",   "Content type",    cmd_html_dummy);
  init_new_hcmd("Content-Length:", "Content length",  cmd_html_len);

  //  Host: 127.0.0.1:5432
  //User-Agent: curl/7.47.0
  //Accept: */*
  //Content-Length: 4
  //Content-Type: application/x-www-form-urlencoded

}

struct space *help_new_gcmds(struct space **base, char *name, struct iosock *in)
{
  return help_new_cmds(g_cmds, NUM_CMDS, base, name, in);
}

struct space *help_new_cmds(struct cmds *cmds, int n,struct space **base, char *name, struct iosock *in)
{
  //int rc = 0;
  int i = 0;
  for (i = 0; i< n; i++,cmds++)
    {
      if(cmds->key != NULL)
	{
	  if(in)in_snprintf(in, NULL, "%s -> %s\n"
			    , cmds->key
			    , cmds->desc
			    );
	}
    }

  return NULL;
}

// TODO add space after cmd
int in_new_cmds(struct cmds * cmds, int n, char * name)
{
  int rc = -1;
  int i;
  for (i = 0; i< n; i++, cmds++)
    {
      if(strcmp(cmds->key, name) == 0)
	{
	  rc = i;
	  break;
	}
    }
  if(i >= n)
    rc = -1;

  return rc;
}

struct space *cmd_quit(struct space **base, char *name, struct iosock *in)
{
  //main
  printf("quitting\n");
  //if(g_lsock>0) close(g_lsock);
  if(in->fd>0) close(in->fd);
  in->fd = -1;
  return NULL;

}
struct space *show_space_in(struct space **base, char *name, struct iosock *in)
{
  int rc = 0;
  char sbuf[4096];
  struct space *sp1=NULL;
  struct space **spb=&g_space;
  char *sp = name;
  rc = sscanf(name,"%s ", sbuf);  // TODO use more secure option
  if(g_debug)
    printf("%s 1 name [%s]\n"
	   ,__FUNCTION__
	   ,name
	   );
  if(in_new_cmds(g_cmds, NUM_CMDS, sbuf)>=0)
    {
      sp = strstr(name, sbuf);
      if(sp)
	{
	  sp += strlen(sbuf);
	  while (*sp &&(*sp == ' '))
	    {
	      sp++;
	    }
	  if(g_debug)
	    printf("%s 2 name [%s] len %lu sp [%s] %x\n"
		   , __FUNCTION__
		   , name
		   , strlen(sp)
		   , sp
		   , *sp
		   );
	  if ((*sp != 0xa) && (*sp != 0xd)) 
	    sp1 = find_space_new(g_space, sp);
	}
    }
  if(sp1)
    {
      spb = &sp1;
    }
  rc = show_spaces_new(in, spb, sbuf, 4096, sbuf);
  return NULL;
}

struct space *decode_cmd_in(struct space **base, char *name, struct iosock *in)
{
  int rc = 0;
  char sbuf[64];
  char scid[64];
  char sclen[64];
  int clen = 0;
  struct space *sp1=NULL;
  struct space **spb=&g_space;
  char *sp = name;
  sbuf[0] =0;
  scid[0] =0;
  sclen[0] =0;
  
  rc = sscanf(name,"%s %s %s", sbuf, scid, sclen);  // TODO use more secure option
  if(rc > 2)
    {
      clen = atoi(sclen);
      if(in->cmdid) free (in->cmdid);
      in->cmdid = strdup(scid);
      in->cmdlen = clen;
      in->cmdbytes = clen;
      in->instate = STATE_IN_CMD;
      printf("%s 1 name [%s] cmd [%s] cid [%s] clen [%s]\n"
	     ,__FUNCTION__
	     , name
	     , sbuf
	     , in->cmdid
	     , sclen
	     );
    }
  else
    {
      printf("%s 2 unable to parse name [%s] cmd [%s] rc %d\n"
	     ,__FUNCTION__
	     , name
	     , sbuf
	     , rc
	     );
    }
  return NULL;
}
// decode reply , TODO look for function to process reply 
struct space *decode_rep_in(struct space **base, char *name, struct iosock *in)
{
  int rc = 0;
  char sbuf[64];
  char scid[64];
  char sclen[64];
  int clen = 0;
  struct space *sp1=NULL;
  struct space **spb=&g_space;
  char *sp = name;
  sbuf[0] =0;
  scid[0] =0;
  sclen[0] =0;
  
  rc = sscanf(name,"%s %s %s", sbuf, scid, sclen);  // TODO use more secure option
  if(rc > 2)
    {
      clen = atoi(sclen);
      if(in->cmdid) free (in->cmdid);
      in->cmdid = strdup(scid);
      // TODO find reply processor      
      in->cmdlen = clen;
      in->cmdbytes = clen;
      in->instate = STATE_IN_REP;
      if(g_debug)
	printf("%s 1 name [%s] cmd [%s] cid [%s] clen [%s]\n"
	       , __FUNCTION__
	       , name
	       , sbuf
	       , in->cmdid
	       , sclen
	       );
    }
  else
    {
      if(g_debug)
	printf("%s 2 unable to parse name [%s] cmd [%s] rc %d\n"
	       ,__FUNCTION__
	       , name
	       , sbuf
	       , rc
	       );
    }
  return NULL;
}

int run_str_in(struct iosock *in, char *stuff, char *cmd)
{
  struct space *space=NULL;
  struct space *attr=NULL;
  int rc;
  rc = run_new_gcmd (cmd, &g_space, stuff, in);
  if(rc >= 0) return 0;
  if(strcmp(cmd, "ADD") == 0)
    {
      add_space_in(&g_space, stuff, in);
      return 0;
    }
  else if(strcmp(cmd, "SET") == 0)
    {
      set_space_in(&g_space, stuff, in);
      return 0 ; 
    }
  else if(strcmp(cmd, "GET") == 0)
    {
      rc = 0;
      get_space_in(&g_space, stuff, in);
      return rc;
    }
  else if(strcmp(cmd, "SHOW") == 0)
    {
      rc = 0;
      show_space_in(&g_space, stuff, in);
      return rc;
    }
  return rc;
}

int xrun_str(char *stuff)
{
  char *sp = stuff;
  char cmd[128];
  cmd[0] = 0;
  sscanf(sp,"%s ", cmd);   //TODO use better sscanf
  return run_str_in(NULL, stuff, cmd);
}

/*
   This code may be used to drive the Adafruit (or clones) Motor Shield.

   The code as written only supports DC motors.

   http://shieldlist.org/adafruit/motor

   The shield pinouts are

   D12 MOTORLATCH
   D11 PMW motor 1
   D10 Servo 1
   D9  Servo 2
   D8  MOTORDATA

   D7  MOTORENABLE
   D6  PWM motor 4
   D5  PWM motor 3
   D4  MOTORCLK
   D3  PWM motor 2

   The motor states (forward, backward, brake, release) are encoded using the
   MOTOR_ latch pins.  This saves four gpios.
*/

/* psw added stdin  and network interfaces
   commands are:
           M n +/-speed[, M n +/-speed]...
                Drive motors +0 = brake -0 = release
           S n +/- pos speed 
              Servo Drive +/- position or aps position plus optional speed 
           D n on or off
*/

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

static uint8_t latch_st;


int init_cmds(struct cmds *cmds, int n)
{
  int i;
  for (i = 0; i<n; i++, cmds++)
    {
      cmds->key = NULL;
      cmds->desc = NULL;
      cmds->handler = NULL;
      cmds->new_hand = NULL;
    }
  return i;
}


int init_new_gcmd(char *key, char *desc, struct space *(*hand)
		 (struct space ** base, char *name, struct iosock *in))
{
  init_new_cmd(g_cmds, NUM_CMDS, key, desc, hand);
}
int init_new_hcmd(char *key, char *desc, struct space *(*hand)
		 (struct space ** base, char *name, struct iosock *in))
{
  init_new_cmd(h_cmds, NUM_CMDS, key, desc, hand);
}


int init_new_cmd(struct cmds *cmds, int ncmds , char *key, char *desc, struct space *(*hand)
		 (struct space ** base, char *name, struct iosock *in))
{
  int i;
  for (i = 0; i< ncmds; i++, cmds++)
  {
    if(cmds->key == NULL)
      {
	cmds->key =  key;
	cmds->desc =  desc;
	cmds->new_hand = hand;
	break;
      }
  }
  if(i == ncmds) i = -1;
  return i;
}

int run_new_gcmd(char *key, struct space **base, char *stuff, struct iosock *in)
{
  return run_new_cmd(g_cmds, NUM_CMDS, key, base, stuff, in);
}
int run_new_hcmd(char *key, struct space **base, char *stuff, struct iosock *in)
{
  return run_new_cmd(h_cmds, NUM_CMDS, key, base, stuff, in);
}



int run_new_cmd(struct cmds *cmds, int ncmds, char *key, struct space **base, char *stuff, struct iosock *in)
{
  int rc=-1;
  int i;
  struct space * sp1 = NULL;
  for (i = 0; i< ncmds; i++, cmds++)
    {
      if(cmds->key && (strcmp(cmds->key, key) == 0))
	{
	  sp1 = cmds->new_hand(base, stuff, in);
	  break;
	}
    }
  rc = i;
  if(i == ncmds)
    rc = -1;
  return rc;
}

int init_hands(void)
{
  int i;
  for (i = 0; i< NUM_HAND; i++)
    {
      hand[i].key = NULL;
      hand[i].desc = NULL;
      hand[i].handler = NULL;
      hand[i].new_hand = NULL;
    }
  return i;
}
//int dummy_handler(int fd, char *id, char *buf, int len)
//init_new_hand("some_id", "Dummy Handler",  dummy_handler);
int init_new_hand(char *key, char *desc, int(*handler)
		  (int fd, char *id,char *buf, int len))
{
  int i;
  for (i = 0; i< NUM_HAND; i++)
  {
    if(hand[i].key == NULL)
      {
	hand[i].key =  key;
	hand[i].desc =  desc;
	hand[i].new_hand = handler;
	break;
      }
  }
  if(i == NUM_HAND) i = -1;
  return i;
}

//int dummy_handler(int fd, char *id, char *buf, int len)
int run_new_hand(char *key, int fd, char *buf, int len)
{
  int rc=-1;
  int i;
  for (i = 0; i< NUM_HAND; i++)
    {
      if(hand[i].key && (strcmp(hand[i].key, key) == 0))
	{
	  rc = hand[i].new_hand(fd, key, buf, len);
	  break;
	}
    }
  rc = i;
  if(i == NUM_HAND)
    rc = -1;
  return rc;
}

int init_iosock(struct iosock *in)
{

  in->fd = -1;
  in->outbptr = 0;
  in->outblen = 0;
  in->iobuf = NULL;
  in->inbuf = NULL;
  //in->cmdptr = NULL;
  in->cmdlen = 0;
  in->hlen = 0;
  in->hidx = -1;
  in->cmdbytes = 0;
  //in->cmdtrm = 0;
  in->cmdid = NULL;
  in->tlen = 0;
  in->nosend = 0;
  in->instate = STATE_IN_NORM;
  return 0;
}

int init_iosocks(void)
{
  int i;
  struct iosock *in;
  for (i = 0; i< NUM_SOCKS; i++)
  {

      in = &g_iosock[i];
      init_iosock(in);
  }
  return i;
}

//struct iobuf/
//{
//struct iobuf *prev;
//struct iobuf *next;
//char *outbuf;
//int outlen;
//int outptr;
//};
struct iobuf *pull_iob(struct iobuf **inp, char **bufp, int *lenp);
struct iobuf *pull_ciob(struct iobuf **inp, struct iobuf *ciob, char **bufp, int *lenp);
int push_ciob(struct iobuf **iobp, struct iobuf *ciob, struct iobuf *iob);

int iob_snprintf(struct iobuf *iob, const char *fmt, ...)
{
    int size;
    char *sp = NULL;
    va_list args;
    struct iobuf *ciob;
#if 0
    char outbuf[128];
  
  va_start(args, fmt);
  size = vsnprintf(outbuf, 0, fmt, args) +1;
  va_end(args);
 
  printf(" size found %d len %lu\n"
	 , size
	 , sizeof(outbuf));
  va_end(args);

  va_start(args, fmt);
  vsnprintf(outbuf, size, fmt, args);
  va_end(args);
  printf(" buf [%s]\n"
	 , outbuf);
  va_end(args);
  return size;
#endif
    if (iob)
    {
        va_start(args, fmt);
        size = vsnprintf(iob->outbuf, 0, fmt, args) +1;
        va_end(args);
	if(g_debug)printf(" size found %d len %d\n"
			  , size
			  , iob->outlen);
	
        if(iob->outptr+ size > iob->outlen)
        {
	  ciob = iob;
	  iob = new_iobuf(ciob->outlen+size);
	  push_ciob(NULL, ciob, iob);
	}
	va_start(args, fmt);
	size = vsnprintf(&iob->outbuf[iob->outptr], iob->outlen-iob->outptr, fmt, args);
	va_end(args);
    }
    return size;
}
//in
int in_snprintf(struct iosock *in, struct iobuf *xiob, const char *fmt, ...)
{
  int size;
  struct iobuf *iob = NULL;
  struct iobuf *ciob;
  va_list args;

  if(xiob)
    iob = xiob;
      
  if((!in || !in->iobuf) && !xiob)
    iob = new_iobuf(128);

  if(in)
    {
      if(in->iobuf)
	iob = in->iobuf;
      else
	in->iobuf=iob;
    }
  if (iob)
    {
      va_start(args, fmt);
      size = vsnprintf(iob->outbuf, 0, fmt, args) +1;
      va_end(args);
      if(g_debug)
	printf(" size found %d len %d\n"
	       , size
	       , iob->outlen);
      
      if(iob->outlen+ size > iob->outsize)
        {
	  ciob = iob;
	  iob = new_iobuf(ciob->outlen+size);
	  push_ciob(NULL, ciob, iob);
	}
	va_start(args, fmt);
	size = vsnprintf(&iob->outbuf[iob->outlen], iob->outsize-iob->outlen, fmt, args);
	va_end(args);
	iob->outlen+= size;
    }
  if(in)in->outblen += size;
  return size;
  
}

struct iobuf *seek_iob(struct iobuf **inp, int len)
{
  struct iobuf *iob = *inp;
  struct iobuf *iobs = *inp;
  while (iob)
    {
      if(iob->outlen >= len)
	break;
      iob = iob->next;
      if(iob == iobs)
	iob = NULL;
    }
  if(iob)
    {
      if(iob == iobs)
	pull_iob(inp, NULL, 0);
      else
	pull_ciob(NULL, iob, NULL, 0);
    }
  return iob;
}

struct iobuf *new_iobuf(int len)
{
  struct iobuf *iob = NULL;

  iob = seek_iob(&g_iob_store, len); 
  if(!iob)
    iob = (struct iobuf *)malloc(sizeof (struct iobuf));

  int xlen = len+5;

  iob->outbuf = (char *)malloc(xlen);
  iob->outbuf[0] = 0;
  iob->outsize=xlen;
  iob->outptr=0;
  iob->outlen=0;
  iob->prev = iob;
  iob->next = iob;
  return iob;
}


int extend_iobuf(struct iobuf * iob,char *buf, int len)
{
  memcpy(iob->outbuf+iob->outptr,buf,len);
  iob->outptr+=len;
}

int push_ciob(struct iobuf **iobp, struct iobuf *ciob, struct iobuf *iob)
{

  struct iobuf *piob = NULL;
  struct iobuf *niob = NULL;
  //if(iobp)ciob= *iobp;
  if(ciob)
    {
      niob = ciob->next;
      piob = ciob->prev;
      if ((piob == niob) && (piob == ciob))
	{
	  if(0)printf("zero insert iob %p ciob %p\n", iob , ciob); 
	  ciob->next = iob;
	  ciob->prev = iob;
	  iob->next = ciob;
	  iob->prev = ciob;
	}
      else
	{
	  if(0)printf("before ciob insert iob %p ciob %p\n", iob , ciob); 
	  iob->next = ciob;
	  iob->prev = piob;
	  ciob->prev = iob;
	  piob->next = iob;
	  
	}
    }
  else
    {
      if(iobp)*iobp=iob;
    }
  return 0;
}

int push_iob(struct iobuf **iobp ,  struct iobuf *iob)
{
  struct iobuf *ciob = NULL;
  if(iobp)ciob= *iobp;
  return push_ciob(iobp, ciob, iob);
}

int store_iob(struct iobuf **iobp ,  struct iobuf *iob)
{
  if(iob)
    {
      iob->outptr = 0;
      push_iob(iobp, iob);
    }
  return 0;

}

int xadd_iob(struct iosock *in, char *buf, int len)
{
  int need_push = 0;
  struct iobuf *iob = NULL;
  struct iobuf *ciob = NULL;
  if (in->iobuf != NULL)
    {
      iob = in->iobuf;
    }
  else
    {
      iob = new_iobuf(len);
      need_push = 1;
    }

  if (iob->outlen+len >= iob->outsize)
    {
      iob = new_iobuf(len);
      need_push = 1;
    }
  extend_iobuf(iob, buf, len);
  if(need_push)
    {
      push_iob(&in->iobuf, iob);
    }
  return 0;
}

struct iobuf *pull_ciob(struct iobuf **inp, struct iobuf *ciob, char **bufp, int *lenp)
{
  struct iobuf *piob = NULL;
  struct iobuf *niob = NULL;
  int rc = -1;
  if(lenp) *lenp = 0;
  if (ciob != NULL)
    {

     if(bufp) *bufp = &ciob->outbuf[ciob->outptr];
     if(lenp) *lenp = ciob->outlen-ciob->outptr;
     if ((ciob->prev == ciob->next) && (ciob->prev == ciob))
       {
	 if(0)printf(" pull last iob %p\n", ciob);
	 if(inp)*inp = NULL;
       }
     else
       {
	 if(0)printf(" pull this iob 1  ciob %p niob %p piob %p\n"
		, ciob, ciob->next, ciob->prev);
	 niob = ciob->next;
	 piob = ciob->prev;
	 if(inp)*inp = niob;
         niob->prev = piob;
	 piob->next = niob;
	 if(0)printf(" pull this iob 2  ciob %p next %p prev %p\n"
		, niob, niob->next,niob->prev);

       }
     ciob->next = ciob;
     ciob->prev = ciob;
    }
  return ciob;
}

struct iobuf *pull_iob(struct iobuf **inp, char **bufp, int *lenp)
{
  struct iobuf *ciob = NULL;
  if(inp)  ciob = *inp;
  return pull_ciob(inp, ciob, bufp, lenp);
}

int print_iob(struct iobuf *iob)
{
  printf("@%p next@%p  out p/l/s %d/%d/%d [%s]\n"
	 , iob
	 , iob->next
	 , iob->outptr   // where we read from
	 , iob->outlen   // where we write to
	 , iob->outsize  // space
	 , iob->outbuf
	 );
  return 0;
}

int print_iobs(struct iobuf *in)
{
  int rc = 0;
  struct iobuf *iob = NULL;
  struct iobuf *siob = NULL;

  siob = in;//in->iobuf;
  iob = in;//in->iobuf;

  while(iob)
    {
      rc++;
      print_iob(iob);
      iob = iob->next;
      if(iob == siob) iob = NULL;
    }
  return rc;
}

int remove_iobs(struct iobuf **in)
{
  int rc = 0;
  struct iobuf *iob = NULL;
  struct iobuf *liob = NULL;
  struct iobuf *siob = NULL;

  siob = *in;//in->iobuf;
  iob = *in;//in->iobuf;

  while(iob)
    {
      rc++;
      if(iob->outbuf)free(iob->outbuf);
      
      liob = iob;
      iob = iob->next;
      if(iob == siob) iob = NULL;
      if(liob)free(liob);
    }
  *in = NULL;
  return rc;
}


//
int test_iob(void)
{
  struct iosock inx;
  struct iosock *in = &inx;
  char *sp;
  int len;
  struct iobuf *iob= NULL;
  struct iobuf *iob1= NULL;
  struct iobuf *iob2= NULL;
  struct iobuf *iob3= NULL;

  init_iosock(in);

  if(g_debug)
    printf(" After init :-\n");
  print_iobs(in->iobuf);
  sp = "1 first inblock\n";
  in_snprintf(in, NULL, "%s", sp);

  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After 1 :-\n");
  print_iobs(in->iobuf);
  sp = "2 next inblock\n";
  in_snprintf(in, NULL, "%s", sp);

  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After 2 :-\n");
  print_iobs(in->iobuf);

  sp = "3 lastst inblock\n";
  in_snprintf(in, NULL, "%s", sp);
  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After last :-\n");
  print_iobs(in->iobuf);
  //ciob = in->iobuf;

  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 1 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  printf("\n\n");

  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 2 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n");
  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 3 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n");
  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 4 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n iobstore follows\n");
  print_iobs(g_iob_store);
  iob = new_iobuf(12);
  if(g_debug)
    printf("\n\n iobstore after small pull %p\n", iob);
  print_iobs(g_iob_store);

  if(iob) store_iob(&g_iob_store, iob);
  iob = new_iobuf(120);
  if(g_debug)
    printf("\n\n iobstore after large pull %p\n", iob);
  print_iobs(g_iob_store);
  if(iob) store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n iobstore after store %p\n", iob);
  print_iobs(g_iob_store);
  remove_iobs(&g_iob_store);
  iob1 = new_iobuf(12);
  in_snprintf(NULL, iob1, "the name [%s] value is %d ", "some_name", 22);
  in_snprintf(NULL, iob1, "more stuff  the name [%s] value is %d ", "some_name", 23);
  if(g_debug)
    printf("\n\n iob 1 %p after snprintf  [%s] prev %p next %p \n"
	   , iob1
	   , iob1->outbuf
	   , iob1->next
	   , iob1->prev
	   );
  iob2 = iob1->next;
  if(g_debug)
    printf("\n\n iob 2 %p after snprintf  [%s] prev %p next %p \n"
	   , iob2
	   , iob2->outbuf
	   , iob2->next
	   , iob2->prev
	   );
  iob3 = iob2->next;
  if(g_debug)
    printf("\n\n iob 3 %p after snprintf  [%s] prev %p next %p \n"
	   , iob3
	   , iob3->outbuf
	   , iob3->next
	   , iob3->prev
	   );

}

int connect_socket(int portno, char *addr)
{
     int sockfd, newsockfd, clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     int data;
     char *sp;

     printf( "using port #%d\n", portno );
    
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     {
	 printf("ERROR opening socket");
	 return -1;
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     if(addr)
       {
	 sp =  addr;
       }
     else
       {
	 sp = "127.0.0.1";
       }
     if(inet_pton(AF_INET, sp, &serv_addr.sin_addr)<=0)
       {
	 printf("\n inet_pton error occured\n");
	 return 1;
       }

     serv_addr.sin_port = htons( portno );
     if (connect(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
     {
	 printf("ERROR on connect");
	 close(sockfd);
	 return -1;
     }
     printf( "connect sock #%d\n", sockfd );
     return sockfd;
} 

int listen_socket(int portno)
{
     int sockfd, newsockfd, clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     int data;
     int optval;
     
     printf( "using port #%d\n", portno );
    
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     {
	 printf("ERROR opening socket");
	 return -1;
     }
     optval = 1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
	       (const void *)&optval , sizeof(int));
      
#if 0
#ifdef SO_REUSEPORT
     setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
#endif
#endif

     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons( portno );
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
     {
	 printf("ERROR on binding");
	 close(sockfd);
	 return -1;
     }
     listen(sockfd,5);
     printf( "listen sock #%d\n", sockfd );

     return sockfd;
} 

int add_socket(int sockfd)
{
  int i;
  for (i = 0; i<NUM_SOCKS; i++)
    {
      if (g_iosock[i].fd < 0)
	{
	  init_iosock(&g_iosock[i]);
	  g_iosock[i].fd = sockfd;
	  g_num_socks++;
	  i = NUM_SOCKS;
	}
    }
  return sockfd;
}

int accept_socket(int sockfd)
{
     int newsock, clilen;
     struct sockaddr_in cli_addr;
     int i;
     if (sockfd != STDIN_FILENO)
     {
	 clilen = sizeof(cli_addr);
	 if ( ( newsock = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
	   {
	     printf("ERROR on accept");
	     return -1;
	   }
	 printf( "opened new communication with client \n" );
     }
     else
     {
	 newsock = sockfd;
     }
     return add_socket(newsock);
}

struct iosock *find_fd(int fsock)
{
    int i;
    struct iosock *in = NULL;
    for (i = 0; i< NUM_SOCKS; i++)
    {
        if (g_iosock[i].fd == fsock)
        {
	    in = &g_iosock[i];
	    break;
        }
    }
    return in;
}

int close_fds(int fsock)
{
    int rc = -1;
    struct iosock *in = find_fd(fsock);
    if (in)
      {
	// TODO drain iobufs	
	rc = 0;
	in->fd = -1;
	//in->inptr = 0;
	//in->inlen = 0;
	
	g_num_socks--;
      }
    return rc;
}

/*
  input may go to the default command processor
  or we may need to scan it for a command block
  the initial command blocks will be ascii
  "CMD IDxxxxxx LENxxxxx "
  BUT we dont know if we are gong to get a CMD
so lets set an initial input length to 4 , enough to get CMD
if we get "CMD "set input length to say 18 to get the rest of the structure
Then we can extract the ID and the length to get the rest of the command.
The input buffer is relocatable anyway.

a cmd terminates with a double \a\a 
note run_str_in should return the length string used 
*/
int find_cmd_term(struct iosock *in, int len, int last)// input buffer
{
  int rc  = 0;
  struct iobuf *inbf;  // input buffer
  char *sp;
  int lend;
  int found = 0;

  inbf = in->inbuf;
  sp = &inbf->outbuf[inbf->outptr];

  lend = inbf->outlen;
  if(g_debug)
    printf("%s lend %d outptr/len %d/%d last %d\n"
	   , __FUNCTION__
	   , lend
	   , inbf->outptr
	   , inbf->outlen
	   , last
	   );
  while (lend)
    {
      rc ++;
      in->tlen = 0;
      if(g_debug_term)
	{
	  if((*sp == 0xa) ||(*sp == 0xd))
	    {
	      printf("%s checking (.) %x rc %d lend %d\n"
		     , __FUNCTION__, *sp ,rc , lend);
	    }
	  else
	    {
	      printf("%s checking (%c) %x rc %d lend %d\n"
		     , __FUNCTION__, *sp, *sp ,rc , lend);
	    }
	}
      if ((*sp == 0xa) ||(*sp == 0xd))
	found++;
      else
	found = 0;

      if (found == 2)
	{
	  return rc;
	}
      sp++;
      lend--;
    }
  
  return 0;
}

int get_rsize(struct iosock *in)
{
  int rc=0;
  int rlen = 0;
  struct iobuf *inbf;  // input buffer
  inbf = in->inbuf;
  rlen = inbf->outsize-inbf->outlen;
  return rlen;
}

int count_buf_bytes(struct iobuf *oubuf)
{
  int rc=0;
  struct iobuf *oust = oubuf;
  while(oubuf)
    {
      rc += oubuf->outlen - oubuf->outptr;
      oubuf = oubuf->next;
      if(oubuf == oust)
	oubuf = NULL;
    }
  
  
  return rc;
}

int handle_input_cmd(struct iosock *in)
{
    int rc=0;
    int len=0;
    int n;
    char cmd[64];
    char *sp;
    struct iobuf *inbf;  // input buffer
    struct iobuf *oubf;  // input buffer
    int rsize;
    int tosend;
    int bytesin;

    inbf = in->inbuf;

    bytesin = inbf->outlen - inbf->outptr;
    if(in->cmdbytes > 0)
      {
	in->cmdlen -= bytesin;
      }
    if(in->cmdlen < 0)
      {
	in->cmdlen = 0;
      }
    
    printf("%s bytesin %d cmdlen %d\n"
	   , __FUNCTION__
	   , bytesin
	   , in->cmdlen);

    sp = &inbf->outbuf[inbf->outptr];

    if(in->cmdlen <= 0)
      {
	//in->cmdptr = sp;

	n = sscanf(sp, "%s ", cmd);   //TODO use better sscanf
	if(0)in_snprintf(in, NULL
			 ," message received [%s] ->"
			 " n %d cmd [%s]\n"
			 , sp //&in->inbuf[in->inptr]
			 , n, cmd );

	in->tlen = in->cmdbytes;
	in->cmdbytes = 0;

      	in->instate = STATE_IN_NORM;
	run_str_in(in, sp, cmd);
	// TODO consume just the current cmd
	inbf->outptr += in->tlen;
	tosend = count_buf_bytes(in->iobuf);

	printf(" %s rc %d n %d  cmd [%s] tosend %d tlen %d cmdid [%s]\n"
	       , __FUNCTION__
	       , rc, n, cmd, tosend
	       , in->tlen
	       , in->cmdid ? in->cmdid :"no id"
	       );
	snprintf(cmd, sizeof(cmd), "REP %s %d\n\n"
		, in->cmdid
		, tosend
		);
	write (in->fd, cmd, strlen(cmd));
	if (inbf->outptr < inbf->outlen)
	  {
	    rc = 1;
	  }
      }
    else
      {
	if(in->cmdbytes > 0)
	  {
	    printf(">>>>>input still needs %d bytes\n"
		   , in->cmdlen );
	    rc = 1;
	  }
      }
    return rc;
}


int handle_input_rep(struct iosock *in)
{
    int rc=0;
    int len=0;
    int n;
    char cmd[64];
    char *sp;
    char buf[1024];
    char *bres;
    int lres;
    char *bufp;
    struct iobuf *inbf;  // input buffer
    struct iobuf *oubf;  // input buffer
    int rsize;
    int tosend;
    int bytesin;

    inbf = in->inbuf;
    sp = &inbf->outbuf[inbf->outptr];

    rsize = in->cmdbytes - in->cmdlen;
    bytesin = inbf->outlen - inbf->outptr;
    if(g_debug)
      printf("%s rsize  %d bytesin %d\n", __FUNCTION__, rsize, bytesin);
    if (bytesin >= rsize)
      {
	in->cmdlen = 0;
	rsize = 0;
	rc = 1;
      }
    if (bytesin < rsize)
      {
	rsize -= bytesin;
	if(g_debug)
	  printf("%s adjusted rsize  %d\n", __FUNCTION__, rsize);

      }
    // we need more
    if(rsize > 0)
      {
	return 0;
      }
    if(g_debug)
      printf("%s read sp [%s] outptr/len %d/%d/ cmd/bytes %d/%d\n"
	     , __FUNCTION__
	     , sp
	     , inbf->outptr
	     , inbf->outlen
	     , in->cmdlen
	     , in->cmdbytes
	     );
	

	sp = &inbf->outbuf[inbf->outptr];
    //TODO only run this if we have received all of the command
    // use in->cmdbytes to count remaining bytes if any
    if(in->cmdlen <= 0)
      {
	n = sscanf(sp, "%s ", cmd);   //TODO use better sscanf
	if(0)in_snprintf(in, NULL
		    ," message received [%s] ->"
		    " n %d cmd [%s]\n"
		    , sp //&in->inbuf[in->inptr]
		    , n, cmd );
	in->instate = STATE_IN_NORM;
	run_new_hand(in->cmdid
		     , in->fd
		     , &inbf->outbuf[inbf->outptr]
		     , in->cmdbytes);
	//run_str_in(in, sp, cmd);
	rsize = in->cmdbytes;
	in->cmdbytes = 0;
	    
	if(g_debug)
	  printf(" %s rc %d n %d cmd [%s] tosend %d cmdid [%s]\n"
		 , __FUNCTION__
		 , rc, n, cmd, tosend
		 , in->cmdid ? in->cmdid :"no id"
		 );
	// TODO consume just the current cmd
	inbf->outptr += rsize;
	if(g_debug)
	  printf(" %s reset buffers rsize %d ptr/len %d/%d\n"
		 , __FUNCTION__
		 , rsize
		 , inbf->outptr
		 , inbf->outlen
		 );
	if(in->cmdbytes > 0)
	  {
	    printf("%s >>>>>reply still needs %d bytes\n"
		   , __FUNCTION__
		   , in->cmdlen );
	  }
	
	if(g_quit_one)
	  {
	    if(g_debug)
	      printf("%s >>>>>forcing quit in->fd %d\n"
		     , __FUNCTION__
		     , in->fd
		     );
	    len = 0;
	    
	    if(in->fd>0) close(in->fd);
	    in->fd = -1;
	  }
      }
    return len;
}
void url_decode(char* src, char* dest, int max) {
    char *p = src;
    char code[3] = { 0 };
    while(*p && --max) {
        if(*p == '%') {
            memcpy(code, ++p, 2);
            *dest++ = (char)strtoul(code, NULL, 16);
            p += 2;
        } else {
            *dest++ = *p++;
        }
    }
    *dest = '\0';
}
				     
int send_html_head(struct iosock *in, char *msg)
{
  int len = 0;
  len = in_snprintf(in, NULL, "HTTP/1.1 200 OK\r\n%s%s%s%s%s",
		    "Content-Type: text/html\r\n\r\n",
		    "<html><head><style>",
		    "body{font-family: monospace; font-size: 13px;}",
		    "td {padding: 1.5px 6px;}",
		    "</style></head><body>\n");
  return len;
}

int send_html_tail(struct iosock *in, char *msg)
{
  int len;
  len - in_snprintf(in, NULL, "</table></body></html>");
 
  return len;
}

int run_str_http(struct iosock *in, char *sp, char *cmd, char *uri, char *vers)
{
  //struct space *space=NULL;
  //struct space *attr=NULL;
  int rc;
  struct iobuf *inbf;  // input buffer
  struct space *sp1;
  
  inbf = in->inbuf;
  rc = inbf->outlen - inbf->outptr;
  printf(" %s >>>> rc %d cmd [%s] sp [%s]\n"
	 , __FUNCTION__
	 , rc
	 , cmd
	 , sp
	 );
  
  if((sp[0] == 0xd) || (sp[0] == 0xa))
    {
      printf(" %s start of data from 0x%x 0x%x hlen %d hidx %d name [%s]\n"
	     , __FUNCTION__
	     , sp[0]
	     , sp[1]
	     , in->hlen
	     , in->hidx
	     , (in->hidx >= 0)?g_spaces[in->hidx]->name:"not found"
	     );
      sp1 = NULL;
      if(in->hidx >= 0)
	{
	  sp1 = g_spaces[in->hidx];
	}
      if(sp1)
	{
	  if (sp1->value) free(sp1->value);
	  sp1->value = malloc(in->hlen+1);
	  memcpy(sp1->value,&sp[2],in->hlen);
	  sp1->value[in->hlen] = 0;
	}
      in->cmdlen = in->hlen;
      in->cmdbytes = in->hlen;
      inbf->outptr += (in->hlen +2);
      // TODO find space referenced and set data
      //len = inbf->outlen - inbf->outptr;

    }
  else
    {
      rc = run_new_gcmd (cmd, &g_space, sp, in);
      rc = run_new_hcmd (cmd, &g_space, sp, in);
    }
  return 0;
}
// scan for double terminators from outptr to outlen 
int handle_input_norm(struct iosock *in)
{
    int rc=0;
    int len;
    int tlen;
    int n;
    char cmd[1024];
    char uri[1024];
    char vers[1024];
    char *sp;
    char buf[1024];
    char *bres;
    int lres;
    char *bufp;
    struct iobuf *inbf;  // input buffer
    struct iobuf *oubf;  // input buffer
    int rsize;
    int tosend;

    inbf = in->inbuf;
    sp = &inbf->outbuf[inbf->outptr];
    len = inbf->outlen - inbf->outptr;
    tlen = find_cmd_term(in, len, 0 /*,in->tlen*/);
    if(tlen == 1)
      in->tlen = 1;
    else
      in->tlen = 0;
    //if tlen == 1 we found one terminator
    // the next char must also be a terminator
    if(g_debug)
      printf("%s read len %d  sp [%s] tlen %d outptr/len %d/%d\n"
	     , __FUNCTION__
	     , len
	     , sp
	     , tlen
	     , inbf->outptr
	     , inbf->outlen
	     );
    
    if(tlen > 1)
      {
	sp = &inbf->outbuf[inbf->outptr];
	cmd[0]=0;
	uri[0]=0;
	vers[0]=0;

	n = sscanf(sp, "%s %s %s", cmd, uri, vers);   //TODO use better sscanf
	//if(1)in_snprintf(in, NULL
	printf(		 " %s message received [%s] ->"
			 " n %d cmd [%s] uri [%s] vers [%s]\n"
			 , __FUNCTION__
			 , sp //&in->inbuf[in->inptr]
			 , n, cmd, uri, vers );
	if(strstr(vers,"HTTP/"))
	  {
	    in->instate = STATE_IN_HTTP;
	  }
	if (in->instate == STATE_IN_HTTP)
	  {
	    run_str_http(in, sp, cmd, uri, vers);
	  }
	else
	  {
	    run_str_in(in, sp, cmd);
	  }
	tosend = count_buf_bytes(in->iobuf);
	if(g_debug)
	  printf(" rc %d n %d cmd [%s] tlen %d tosend %d \n"
		 , rc, n, cmd, tlen, tosend
		 );
	// TODO consume just the current cmd
	// flag the fact that we got more
	
	inbf->outptr += tlen;
	if(inbf->outptr < inbf->outlen)
	  rc  = 1;
	if(g_debug)
	  printf(" reset buffers tlen = %d ptr/len %d/%d more %d\n"
		 , tlen
		 , inbf->outptr
		 , inbf->outlen
		 , rc
		 );
	if (inbf->outptr == inbf->outlen)
	  {
	    inbf->outptr = 0;
	    inbf->outlen = 0;
	    printf(" %s reset buffers tlen = %d ptr/len %d/%d\n"
		   , __FUNCTION__
		   , tlen
		   , inbf->outptr
		   , inbf->outlen
		   );
	  }
      }
    return rc;
}

// A bit complicated here
// we have an input
// we could have one or more commands in the input
// If we get the CMD command then its a bit simpler in tht we now have a
// fixed number of bytes to process before the next input
// We are confused between getting an input and processing a buffer
// once we get an input untill we can process no more
//

int handle_input(struct iosock *in)
{
  int more = 1;
  int len = 0;
  int rsize;
  char *sp;
  struct iobuf *inbf;  // input buffer
  
  if (in->inbuf == NULL)
    in->inbuf = new_iobuf(1024);
  inbf = in->inbuf;
  rsize = get_rsize(in);
  // TODO create a new inbuf
  
  sp = &inbf->outbuf[inbf->outlen];
  len = read(in->fd, sp, rsize);
  
  if(len > 0)
    {
      sp[len] = 0;
      inbf->outlen += len;
      while(more)
	{
	  if(g_debug)
	    printf(" %s running more %d\n"
		   , __FUNCTION__
		   , more
		   );
	  if (in->instate == STATE_IN_CMD)
	    {
	      more = handle_input_cmd(in);
	    }
	  else if (in->instate == STATE_IN_REP)
	    {
	      more = handle_input_rep(in);
	      if(more == 0)
		len = 0;
	    }
	  //else if (in->instate == STATE_IN_HTTP)
	  //{
	  //  more = handle_input_http(in);
	  //  if(more == 0)
	  //	len = 0;
	  //}
	  else
	    {
	      more = handle_input_norm(in);
	    }
	  if(g_debug)
	    printf(" %s done more %d\n"
		   , __FUNCTION__
		   , more
		   );
	}
      
    }
  return len;
}

int handle_output(struct iosock *in)
{
  int rc = 0;
  struct iobuf *iob;
  char *sp;
  int len;
  int bcount = 0;
  // old way
#if 0
  if(in->outptr != in->outlen)
    {
      if(1)printf(" %s running the old way\n", __FUNCTION__);
      rc = write(in->fd,&in->outbuf[in->outptr],in->outlen-in->outptr);
      if(rc >0)
	{
	  in->outptr += rc;
	  if (in->outptr == in->outlen)
	    {
	      in->outptr = 0;
	      in->outlen = 0;
	    }
	}
    }
#endif
  // iobway
  while((bcount++ < 1024) && (in->outbptr != in->outblen))
    {
      len = 0;
      iob = pull_iob(&in->iobuf, &sp, &len);
      if(0)printf(" %s running the new way iob %p len %d sp [%s]\n"
		  , __FUNCTION__, iob, len, sp);
      if(iob)
	{
	  if(0)print_iob(iob);
	}
      else
	{
	  in->outbptr = 0;
	  in->outblen = 0;
	}
      if(len > 0)
	{
	  rc = write(in->fd, sp, len);
	  if(rc <= 0)
	    {
	      // shutdown iobuffer
	      // TODO unload iobs
	      in->outbptr = 0;
	      in->outblen = 0;
	    }
	  if(rc >0)
	    {
	      in->outbptr += rc;
	      if (in->outbptr == in->outblen)
		{
		  in->outbptr = 0;
		  in->outblen = 0;
		}
	    }
	}
      store_iob(&g_iob_store, iob);
    }
  return rc;
}


// read stdin, listen_sock and all accept_socks
int poll_sock(int lsock)
{
    struct pollfd fds[NUM_SOCKS+2];
    int idx = 0;
    int i;
    int timeout = 1000;
    int ret;
    int n;
    char str[1024];
    struct iosock *in = NULL;
    int rc = 1;    

    if(lsock>0)
      {
	fds[idx].fd = lsock;
	fds[idx].events = POLLIN;
	fds[idx].revents = 0;
	idx++;
      }
    
    for (i = 0; i< NUM_SOCKS; i++)
      {
	if (g_iosock[i].fd >= 0)
	  {
	    if(g_debug)
	      printf(" setup fd i %d idx %d fd %d\n"
		     , i
		     , idx
		     , g_iosock[i].fd
		     );
	    
	    fds[idx].fd = g_iosock[i].fd;
	    fds[idx].events = POLLIN;
	    fds[idx].revents = 0;
	    if((g_iosock[i].nosend == 0)
	       && (g_iosock[i].outbptr != g_iosock[i].outblen))
	      {
		fds[idx].events |= POLLOUT;
	      }
	    idx++;
	  }
      }
    if(idx == 0) return -1;
    if(g_debug)printf("poll start idx %d lsock %d \n", idx, lsock);
    ret =  poll(fds, idx, timeout);
    if(g_debug)printf("poll done ret = %d idx %d\n", ret, idx);

    if(ret > 0) 
    {
      if(g_debug)printf("poll ret = %d idx %d\n", ret, idx);
	
	for( i = 0; i < idx; i++) 
	{
	  if(g_debug)
	    printf(" idx %d fd %d revents 0x%08x\n",i, fds[i].fd, fds[i].revents);  
	    if (fds[i].revents & POLLOUT) 
	    {
		in = find_fd(fds[i].fd);
		if(in)
		{
		    n = handle_output(in);
		    if (n <= 0) 
		    {
			printf("error writing (n=%d), closing fd %d \n"
			       ,n,fds[i].fd);
			close(in->fd);
			close_fds(in->fd);
			fds[i].revents = 0;
		    }
		}
	    }
	    if (fds[i].revents & POLLIN) 
	    {
	      if((lsock > 0) && (fds[i].fd == lsock))
		{ 
		    ret = accept_socket(lsock);
		    printf("accept ret = %d \n", ret);
		    
		}
		else
		{
		    in = find_fd(fds[i].fd);
		    if(in)
		    {
		        n = handle_input(in);

			if (n <= 0) 
			{
			  if(g_debug)
			    printf("error reading (n=%d), "
				   "closing fd %d in->fd %d\n"
				   , n
				   ,fds[i].fd
				   , in->fd
				   );
			    close(fds[i].fd);
			    close_fds(fds[i].fd);
			    fds[i].fd = -1;
			    in->fd = -1; 
			}
			else 
			{
			  if(g_debug)
			    printf("message len %d\n", n);
			    
			}
		    }
		}
	    }
	}
    }
    return rc;
}


#if 0
void latch_tx(uint8_t latch_state)
{
   unsigned char i;

   gpioWrite(MOTORLATCH, PI_LOW);

   gpioWrite(MOTORDATA, PI_LOW);

   for (i=0; i<8; i++)
   {
      gpioDelay(10);  // 10 micros delay

      gpioWrite(MOTORCLK, PI_LOW);

      if (latch_state & BIT(7-i)) gpioWrite(MOTORDATA, PI_HIGH);
      else                        gpioWrite(MOTORDATA, PI_LOW);

      gpioDelay(10);  // 10 micros delay

      gpioWrite(MOTORCLK, PI_HIGH);
   }

   gpioWrite(MOTORLATCH, PI_HIGH);
}

void init(uint8_t latch_state)
{
  //latch_state = 0;

   latch_tx(latch_state);

   gpioWrite(MOTORENABLE, PI_LOW);
}

uint8_t DCMotorInit(uint8_t num, uint8_t latch_state)
{

   switch (num)
   {
      case 1: latch_state &= ~BIT(MOTOR1_A) & ~BIT(MOTOR1_B); break;
      case 2: latch_state &= ~BIT(MOTOR2_A) & ~BIT(MOTOR2_B); break;
      case 3: latch_state &= ~BIT(MOTOR3_A) & ~BIT(MOTOR3_B); break;
      case 4: latch_state &= ~BIT(MOTOR4_A) & ~BIT(MOTOR4_B); break;
      default: return latch_state;
   }

   latch_tx(latch_state);

   printf("Latch=%08X\n", latch_state);
   return latch_state;
}

uint8_t DCMotorRun(uint8_t motornum, uint8_t cmd, uint8_t latch_state)
{
   uint8_t a, b;

   switch (motornum)
   {
      case 1: a = MOTOR1_A; b = MOTOR1_B; break;
      case 2: a = MOTOR2_A; b = MOTOR2_B; break;
      case 3: a = MOTOR3_A; b = MOTOR3_B; break;
      case 4: a = MOTOR4_A; b = MOTOR4_B; break;
      default: return latch_state;
   }
 
   switch (cmd)
   {
      case FORWARD:  latch_state |=  BIT(a); latch_state &= ~BIT(b); break;
      case BACKWARD: latch_state &= ~BIT(a); latch_state |=  BIT(b); break;
      case RELEASE:  latch_state &= ~BIT(a); latch_state &= ~BIT(b); break;
      default: return latch_state;
   }

   latch_tx(latch_state);

   printf("Latch=%08X\n", latch_state);
   return latch_state;
}

#endif

int fwd_cmd(void *key, int n, char *data, int speed, int time)
{
    struct cmds *cmd = (struct cmds*)key;

    printf("running [%s] n %d data [%s]  speed %d time %d\n",
	   cmd->key, n, data, speed, time);
    return 0;
}

int init_g_spaces(void)
{
  int i;
  for (i=0 ; i< NUM_IDX; i++)
    g_spaces[i]=NULL;
  return 0;

}

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
int set_spacexx(struct space * base, char *name, char *value)
{
  int rc = -1;
  struct space *sp1;
  char sname[3][128];  // TODO
  char * spv;

  sname[0][0]=0;
  sname[1][0]=0;
  sname[2][0]=0;
  rc = sscanf(name,"%s %s %s", sname[0], sname[1], sname[2]);
  sp1 = find_space_new(base, sname[1]);
  if(sp1)
    {
      spv = sname[2];
      if(value)
	spv =  value;
      if(sp1->value) free(sp1->value);
      sp1->value = NULL;
      if(spv && (strlen(spv) > 0))
	sp1->value = strdup(spv);
      if(sp1->onset)
	sp1->onset(sp1, sp1->idx, name, spv);
      rc = 0;
    }
  return rc;
}

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
struct space *set_space_in(struct space **basep, char *name, struct iosock *in)
{
  int rc = -1;
  struct space *sp1;
  struct space *base =  *basep;
  char sname[3][128];  // TODO
  char * spv;

  sname[0][0]=0;
  sname[1][0]=0;
  sname[2][0]=0;
  rc = sscanf(name,"%s %s %s", sname[0], sname[1], sname[2]);
  sp1 = find_space_new(base, sname[1]);
  if(sp1)
    {
      spv = sname[2];
      ///if(value)
      //spv =  value;
      if(sp1->value) free(sp1->value);
      sp1->value = NULL;
      if(spv && (strlen(spv) > 0))
	sp1->value = strdup(spv);
      if(sp1->onset)
	sp1->onset(sp1, sp1->idx, name, spv);
      rc = 0;
      if(in)in_snprintf(in,NULL,"OK SET %s to [%s]\n",sname[1], sname[2]);
    }
  else
    {
      if(in)in_snprintf(in,NULL,"?? SET [%s] not found \n",sname[1]);
    }
  return sp1;
}

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
int set_space(struct space *base, char *name)
{
  struct space* gbase = base;
  set_space_in(&gbase, name, NULL);
  return 0;
}

//char *get_space(struct space *base, char *name)
struct space *get_space_in(struct space ** basep, char *name, struct iosock *in)
{
  char * sret=NULL;
  struct space *sp1;
  struct space *base =  *basep;
  char *sp;
  char spv[2][128];
  int rc=0;
  spv[0][0]=0;
  spv[1][0]=0;
  rc = sscanf(name,"%s %s", spv[0], spv[1]);
  sp = spv[0];
  if( rc == 2 ) sp = spv[1];
  sp1 = find_space_new(base, sp);
  if(sp1)
    {
      if(sp1->onget)
	sp1->onget(sp1, sp1->idx, name);
      sret = sp1->value;
      if(in)
	{
	  in_snprintf(in, NULL, "OK GET %s value [%s]\n", sp, sret);
	  in->hidx = sp1->idx;
	}
    }
  else
    {
      if(in)
	{
	  in_snprintf(in, NULL, "?? GET [%s] not found \n",sp);
	  in->hidx = -2;
	}
    }
  return sp1;
}

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
char *get_space(struct space *base, char *name)
{
  char *ret =  NULL;
  //TODO
  struct space *gbase = base;
  get_space_in(&gbase, name, NULL);
  return ret;
}

int count = 0;

int show_stuff(int rc, char **vals)
{
  int i;
   for (i = 0 ; i < rc; i++)
     {
       printf (" %s: val[%d] = [%s]\n",__FUNCTION__,  i, vals[i] );

     }
   return 0;
}

int motor_onset(struct space *this, int idx, char *name, char *value)
{
  printf(" running %s for space [%s] %d with value [%s]\n"
	 , __FUNCTION__, this->name, idx, value);
  return 0;

}
int motor_onget(struct space *this, int idx, char *name)
{
  printf(" running %s for space [%s] %d \n"
	 , __FUNCTION__, this->name, idx);
  return 0;

}
int speed_onset(struct space *this, int idx, char *name, char * value)
{
  printf(" running %s for space [%s] %d with value [%s]\n"
	 , __FUNCTION__, this->name, idx, value);
  return 0;

}
int speed_onget(struct space *this, int idx, char *name)
{
  printf(" running %s for space [%s] %d \n"
	 , __FUNCTION__, this->name, idx);
  return 0;

}

int test_iob_out(void)
{

  struct iosock iosock;
  struct iosock *in;
  int rc;
  char *sp;
  sp = "This is a direct test\n";
  in = &iosock;
  init_iosock(in);
  in->fd = 1;
  rc = write(in->fd, sp, strlen(sp));
  printf(" sent %d chars\n", rc);	     

  sp = "** first string**";
  in_snprintf(in, NULL, " in_sprintf 1 sp [%s] num %d\n", sp,21);
  //add_iob(in, sp, strlen(sp));
  printf(" after in_snprintf 1\n");
  print_iobs(in->iobuf);
  printf(" in->outblen %u\n", in->outblen);
  printf(" in->outbptr %u\n", in->outbptr);
  
  sp = " ** adding a much longer second string  twice **";
  in_snprintf(in, NULL, " in_sprintf 2 sp [%s] [%s] num %d\n", sp,sp, 22);

  //add_iob(in, sp, strlen(sp));
  printf(" after in_snprintf 2\n");
  print_iobs(in->iobuf);
  printf(" in->outblen %u\n", in->outblen);
  printf(" in->outbptr %u\n", in->outbptr);
  printf(" ===================\n");
  handle_output(in);
  printf(" ===================\n\n");
  printf(" after handle_output\n\n");
  print_iobs(in->iobuf);

  
  return 0;
}

int dummy_handler(int fd, char *id, char *buf, int len)
{
  if(g_debug)
    printf(" %s reply received, id [%s], len %d [%s]\n"
	   , __FUNCTION__
	   , id
	   , len, buf);
  else
        printf("[%s] \n"
	   , buf);
  return 0;
}

int main (int argc, char *argv[])
{
   int i;
   int csock;
   int csize;
   int rc = 1;
   int depth=0;
   //struct space * sp1 = new_space("Space1", struct space *parent, struct space** root, char *node)
   struct space *sp1;
   struct space *sp2;
   //struct space *sp3;
   char buf[2048];
   char *sp;
   char *vals[64];
   struct iosock ins;
   struct iosock *in = &ins;

   init_g_spaces();
   init_iosocks();
   init_iosock(in);
   init_cmds(g_cmds, NUM_CMDS);
   init_cmds(h_cmds, NUM_CMDS);
   init_hands();
   set_up_new_cmds();

   in->fd = 1;
   test_find_parents();
   //return 0;
   
   if(argc > 1)
     {
       if (strcmp(argv[1], "test_iob_out") == 0)
	 {
	   test_iob_out();
	   return 0;
	 }
       if (strcmp(argv[1], "test_iob") == 0)
	 {
	      test_iob();
	      return 0;
	 }

       if (strcmp(argv[1], "send") == 0)
	 {
	   // send arg 2 3 and maybe 4  to local port and listen for reply
	   if(argc <4 )
	     {
	       snprintf(buf, sizeof(buf),"%s %s", argv[2], argv[3]);
	     }
	   else
	     {
	       snprintf(buf, sizeof(buf),"%s %s %s", argv[2], argv[3], argv[4]);
	     }	       
	   csock = connect_socket(5432, NULL);
	   printf ("sending [%s] to server %s res %d \n", buf, "localhost", csock);
	   if(csock> 0)
	     {
	       //int dummy_hand(int fd, char *id, char *buf, int len)
	       init_new_hand("some_id", "Dummy Handler",  dummy_handler);
	       // run_new_gcmd
	       add_socket(csock);
	       in->fd = -1;
	       csize = snprintf(buf, sizeof(buf),"%s %s", argv[2], argv[3]);
	       //TODO check buf csize
	       // register_handler("some_id", dummy_handler);
	       snprintf(buf, sizeof(buf),"CMD %s %d\n\n", "some_id", csize);
	       rc = write(csock, buf, strlen(buf)); 
	       snprintf(buf, sizeof(buf),"%s %s", argv[2], argv[3]);
	       rc = write(csock, buf, strlen(buf)); 
	       //snprintf(buf, sizeof(buf),"QUIT\n\n");
	       //rc = write(csock, buf, strlen(buf)); 
	       rc = 1;
	       g_debug_term = 0;
	       //while(rc)
	       //{
	       //  rc = read(csock, buf , sizeof(buf)-1);
	       //  if(rc>0)
	       //    {
	       //      dummy_handler(csock, "some_id", buf , rc);
	       //    }
	       //}
	       //close(csock);
	       //return 0;
	       // quit after one _REP
	       g_quit_one = 1;		 
	       g_lsock = -1;

	     }

	 }
     
       else if (strcmp(argv[1], "test") == 0)
	 {

	   char sbuf[4096];

	   rc = parse_stuff(' ', 64, (char **)vals, "this is a bunch/of/stuff to parse",0);
	   printf (" rc = %d\n", rc );
	   show_stuff(rc, vals);
	   
	   sp1 = add_space(&g_space, "ADD uav1/motor1/speed");
	   show_spaces(in, g_space, "All Spaces 1 ",0);
	   
	   sp1 = add_space(&g_space, "ADD uav3/motor2/speed");
	   sp1->onset = speed_onset;
	   sp1->onget = speed_onget;
	   rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
	   sp =  get_space(g_space,"GET uav3/motor2/speed");
	   printf(" >> %s value [%s]\n","uav3/motor2/speed", sp?sp:"no value");
	   return 0;
	   
	   sp1 = add_space(&g_space, "ADD uav1/motor1/size");
	   show_spaces(in,g_space, "All Spaces 1 ",0);
	   sp1 = add_space(&g_space, "ADD uav1/motor2/speed");
	   show_spaces(in,g_space, "All Spaces 2 ",0);
	   sp1 = add_space(&g_space, "ADD uav2/motor2/speed");
	   show_spaces(in,g_space, "All Spaces 3 ",0);
	   sp1 = add_space(&g_space, "uav3/motor2/speed");
	   
	   sp1 = find_space_new(g_space, "uav3/motor2");
	   sp1->onset = motor_onset;
	   sp1->onget = motor_onget;
	   
	   show_spaces_new(in, &g_space, sbuf, 4096, sbuf);
	   sp1 = find_space_new(g_space, "uav1/motor3");
	   printf(" found %s \n", sp1?sp1->name:"no uav1/motor3");
	   sp1 = find_space_new(g_space, "uav1/motor1");
	   printf(" found %s \n", sp1?sp1->name:"no uav1/motor1");
	   sp2  = copy_space_new(sp1, "new_motor1");
	   rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
	   sp =  get_space(g_space,"GET uav3/motor2/speed");
	   printf(" >> %s value [%s]\n","uav3/motor2/speed", sp?sp:"no value");
	   
	   show_spaces_new(in,&sp2, sbuf, 4096, sbuf);
	   
	   //   return 0;
#if 0
	   new_space("Space1", NULL, NULL, "127.0.0.1");
	   sp1 =  g_space;
	   show_space(in, sp1, 0, NULL , 0);
	   sp1 = new_space("Space2", sp1, NULL, "127.0.0.1");
	   show_space(in, g_space, 0, NULL , 0);
	   show_space(sp1, 0, NULL , 0);
	   sp1 = new_space("Space3", sp1, NULL, "127.0.0.1");
	   show_space(in, g_space, 0, NULL , 0);
	   show_space(in,sp1, 0, NULL , 0);
	   
	   //   sp1 = g_space;
	   show_spaces(in, g_space, "Global Spaces 1",0);
	   //           attr        space  class type  value 
	   
	   sp1 = g_space;
	   //struct space *new_space_attr_float(char *name , struct space *parent,1.234)
	   
	   sp2 = new_space_attr_float("foo_float", sp1, 1.2345);
	   sp2 = new_space_attr_int("foo_int", sp1, 2345);
	   sp2 = new_space_attr_str("foo_str", sp1, "x2345");
	   
	   show_spaces(in, sp1->attr, "Sp1 attr",0);
	   
#endif
	 }
     }
   
   if(g_lsock == 0)
     {
       accept_socket(STDIN_FILENO);
       g_lsock = listen_socket(5432);
     }
   rc = 1;
   while(rc>0 && count < 10)
   {
       rc = poll_sock(g_lsock);
       //count++;
   }

#if 0
   if(0)
     {
       if (gpioInitialise()<0) return 1;

       gpioSetMode(MOTORLATCH,  PI_OUTPUT);
       gpioSetMode(MOTORENABLE, PI_OUTPUT);
       gpioSetMode(MOTORDATA,   PI_OUTPUT);
       gpioSetMode(MOTORCLK,    PI_OUTPUT);
       
       gpioSetMode(MOTOR_3_PWM, PI_OUTPUT);
       gpioSetMode(MOTOR_4_PWM, PI_OUTPUT);
       
       gpioPWM(MOTOR_3_PWM, 0);
       gpioPWM(MOTOR_4_PWM, 0);
       
       init(latch_st);
       
       for (i=60; i<160; i+=20)
	 {
	   gpioPWM(MOTOR_3_PWM, i);
	   gpioPWM(MOTOR_4_PWM, 220-i);
	   
	   latch_st = DCMotorRun(3, FORWARD, latch_st);
	   latch_st = DCMotorRun(4, BACKWARD, latch_st);
	   
	   sleep(2);
	   
	   latch_st = DCMotorRun(3, RELEASE, latch_st);
	   latch_st = DCMotorRun(4, RELEASE, latch_st);
	   
	   sleep(2);
	   
	   gpioPWM(MOTOR_4_PWM, i);
	   gpioPWM(MOTOR_3_PWM, 220-i);
	   
	   latch_st = DCMotorRun(3, BACKWARD, latch_st);
	   latch_st = DCMotorRun(4, FORWARD, latch_st);
	   
	   sleep(2);
	   
	   latch_st = DCMotorRun(3, RELEASE, latch_st);
	   latch_st = DCMotorRun(4, RELEASE, latch_st);
	   
	   sleep(2);
	 }
       
       gpioPWM(MOTOR_4_PWM, 0);
       gpioPWM(MOTOR_3_PWM, 0);
       
       latch_st = DCMotorRun(3, RELEASE, latch_st);
       latch_st = DCMotorRun(4, RELEASE, latch_st);
       
       gpioTerminate();
     }
#endif

}
