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

*/

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


// OK the space becomes the real thing
// A space can have kids and attributes
// both or which are spaces  
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
  char *node;
  int (*onset)(struct space *this, int idx, char *name, char * value, char **buf, int *len);
  int (*onget)(struct space *this, int idx, char *name, char **buf, int *len);

};

#define NUM_SOCKS 16
#define NUM_CMDS 16
#define INSIZE 1024
#define OUTSIZE 1024

int num_socks=0;

struct iobuf;

struct iobuf
{
  struct iobuf *prev;
  struct iobuf *next;
  char *outbuf;
  int outlen;
  int outptr;
};

struct insock
{
  int fd;
  char inbuf[INSIZE];
  int inptr;
  int inlen;
  int insize;
  char outbuf[OUTSIZE];
  int outptr;
  int outlen;
  int outsize;
  struct iobuf *iobuf;
};

struct cmds
{
  char *key;
  char *cmd;
  int (*handler)(void *key, int n, char *cmd, int speed, int time);
};
  

struct insock insock[NUM_SOCKS];
struct cmds cmds[NUM_CMDS];
#define NUM_IDX 1024
struct space *space=NULL;
struct space *g_space=NULL;
int g_space_idx = 1;
struct space *g_spaces[NUM_IDX];



char *get_space(struct space * base, char *name, char **buf, int *len);
int set_space(struct space * base, char *name, char *value, char **buf, int *len);
struct space *make_space(struct space **root, char *name, char **buf, int *len);
int show_spaces(struct space *base, char *desc, int indent, char *buf, int len);
int parse_stuff(char delim, int num, char **vals, char *stuff);

struct space * setup_space(char *name, struct space*parent)
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

struct space *new_space(char *name , struct space *parent, struct space **root_space,char *node)
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
      space->node = strdup(node);
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

int show_space_class(struct space *base, int indent, char *buf, int len)
{
  char atname[128];
  snprintf(atname, sizeof(atname),"  %s Class", base->name); 
  if(base->class)
  {
    len -= show_spaces(base->class, atname, indent, buf , len);
  }
  return len;
}

int show_space_attr(struct space *base, int indent, char *buf, int len)
{
  char atname[128];
  snprintf(atname, sizeof(atname),"  %s Attr", base->name); 
  if(base->attr)
  {
    len -= show_spaces(base->attr, atname, indent, buf, len);
  }
  return len;
}

int show_space(struct space *base, int indent, char *buf, int len)
{
  int rc=-1;

  if(!base)return rc;
 
  rc =  indent;
  while(rc--)
    {
      if(buf)
	{
	  if(len > 0)
	    {
	      *buf++ = ' ';
	      len--;
	    }
	}
      else
	printf(" ");
    }
  if(buf)
    {
      rc = snprintf(buf,len," name [%s](%d)\n"
	     , base->name
	     , base->idx
	     );
      if(rc < len)
	{
	  len -= strlen(buf);
	  buf += strlen(buf);
	}
    }
  else
    {
      printf(" %p space %03d name [%s] node [%s] next name [%s] prev name [%s]\n"
	     , base
	     , base->idx
	     , base->name
	     , base->node
	     , base->next->name
	     , base->prev->name
	     );
    }
  show_space_attr(base, indent+3, buf, len);

  if(buf){
    len -= strlen(buf);
    buf += strlen(buf);
  }
  show_space_class(base, indent+5, buf, len);
  if(buf){
    len -= strlen(buf);
    buf += strlen(buf);
  }
  return len;
}


int show_spaces(struct space *base, char *desc, int indent, char *buf, int len)
{
  struct space *start=base;
  int rc;
  if(buf)
    {
      rc= snprintf(buf,len,">>>%s\n", desc ? desc:" ");
      if(rc < len)
	{
	  len -= strlen(buf);
	  buf += strlen(buf);
	}
    }
  else
    printf("spaces ... %s\n", desc ? desc:" ");

  while (base)
    {
      rc = show_space(base, indent, buf, len);
      if(rc < len)
	{
	  len -= strlen(buf);
	  buf += strlen(buf);
	}
      if(base->child) 
	{
	  rc =  show_spaces(base->child,"child",indent+2, buf,len);
	}
      if(base->next != start)
	base=base->next;
      else
	base =  NULL;
    }
  return len;
}



int show_space_new(struct space *base, char *desc, int len, char **bufp, int *lenp)
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

  child = base->child;
  if(child == NULL)
    {
      snprintf(sp, slen,
	       "/%s => %d\n"
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
      //printf(" >> [%s]\n", buf);
      return 0;
    }
  // foreach child do the same
  slen -= strlen(sp);
  sp += strlen(sp);
  while (child != NULL)
    {

      slen += show_space_new(child, sp, slen, bufp, lenp);
      child = child->next;
      if (child == base->child) 
	child= NULL;
    }
  //  ret =  strlen(buf);
  ret =  slen;
  return ret;
}

int show_spaces_new(struct space *base, char *desc, int len, char **bufp, int *lenp)
{
  int rc  = -1;
  struct space *start=base;
  struct space *xstart=NULL;
  while (start)
    {
      rc = show_space_new(start, desc, len, bufp, lenp);
      printf(" >> [%s]\n", desc);

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
  //  char vals[64][64];
  //int idx = 0;
  //int idv = 0;
  char *sp = name;
  //int rc;
  int i;
  //rc = 1;

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
      if(0)printf(" looking for [%s] found [%s] i %d idx %d\n"
		  , valv[i], base->name
		  , i
		  , idx
		  );
      if(strcmp(base->name, valv[i])==0)
	{

          if(i < idv) i++;
	  if(i == idv)
	    {
	      free_stuff(idv, valv);
	      free_stuff(idx, valx);
	      
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


int add_child(struct space *base, struct space *child)
{
  if(base->child == NULL)
    base->child = child;
  else
    add_space(base->child, child);
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
      printf(" >>> find space [%s] parent name [%s] ... \n", name
	     ,start?start->name ? start->name:"No name":"No start");

    }
  else
    {
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
  printf(" >>> find space name [%s] space %p... \n", name, base);
  return base;
}



/*
before any s0->p = s0 s0->n=s0
after add s1 s0->p = s1 s0->n = s1 s1->p = s0 s1->n=s0
after add s2 s0->p = s2 s0->n = s1 s1->p = s0 s1->n=s2 s2->p = s1 s2->n=s0
 */

int add_space(struct space *parent, struct space *space)
{

  struct space *last_space;

  last_space = parent->prev;

  printf( " add_space  parent [%s] pprev [%s] next [%s]\n"
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
  return 0;
}

//   idx = parse_stuff(' ', 64, (char **)valx, name);
//   rc = parse_name(&idx (char **)valx, &idy (char **)valy, 64, name);
int parse_name( int *idx, char ** valx, int *idy , char ** valy, int size, char * name)
{

  int  rc = 1;
  char *sp;
  *idx = parse_stuff(' ', size, (char **)valx, name);

  sp = valx[0];
  if(*idx >= 2)sp = valx[1];
  *idy = parse_stuff('/', size, (char **)valy, sp);
  return rc;
}
// split up multi/space/name
// look for children of the same name 
// return found name or new space object
//    sp1 = make_space(&g_space, "ADD uav1/motor1/speed", NULL, 0);

struct space *make_space(struct space **root, char *name, char **buf, int *len)
{
  struct space *parent=NULL;
  struct space *space=NULL;
  //char sps[3][64];
  //char vals[64][64];
  int cidx = 0;
  char *sp = name;
 
  int i;

  int rc;
  int idx = 0;
  int idv = 0;
  char *valv[64];
  char *valx[64];

  rc = parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);

  for (i = 0 ; i < idv; i++)
    {
      space = NULL;
      if(parent)
	{
	  space = find_space(&parent->child, valv[i]);
	}
      else
	{
	  space = find_space(root, valv[i]);
	}
      if (!space)
	{
	  if(parent)
	    {
	      printf(" New Space for [%s] parent->name [%s]\n", valv[i], parent->name);
	      space = new_space(valv[i], parent->child, &parent->child, NULL); 
	      add_child(parent,space);

	    }
	  else
	    {
	      printf(" New Space for [%s] at root\n", valv[i]);
	      space = new_space(valv[i], NULL, &g_space, NULL); 
	    }
	  if(i == 0)
	    {
	      if (*root == NULL)
		{
		  *root = space;
		}
	      else
		{
		  add_space(*root, space);
		}
	    }
	}
      else
	{
	  printf(" Space found [%s]\n", valv[i]);
	}
      parent = space;
    }
    
  printf("[%s] found %d spaces\n",name, idx);
  free_stuff(idv, valv);
  free_stuff(idx, valx);
  return space;
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
int parse_stuff(char delim, int num, char **vals, char *stuff)
{
  int idx = 0;
  char *sp = stuff;
  char *sp1 = stuff;
  int rc = 1;
  char *val;
  char  *spv;
  int val_size;

  val_size = 64;
  val = malloc(64);
  val[0]=0;

  printf("%s start stuff[%s] \n", __FUNCTION__, stuff);
  //vals[idx] = strdup(sp);
  //idx++;
  while(*sp && (rc>0) && (idx < num))
    {
      rc = 0;
      spv = val;
      while (*sp && *sp != delim && (rc < (val_size-1)))
	{
          *spv++ = *sp++;
	  rc++;
	}
      if(*sp)sp++;
      *spv = 0;
	
      if(rc>0)
	{
	  vals[idx] = strdup(val);
	  printf("rc %d val[%d] [%s] ", rc, idx, vals[idx]);
	  printf("sp [%s] \n", sp);
	  idx++;
	}
    }
  printf("%s done idx %d\n", __FUNCTION__, idx);
  free(val);

  return idx;
}

int run_str(char *stuff, char **bufp, int *len)
{
  char cmd[128];
  snprintf(cmd, sizeof(cmd),"%s",stuff);
  char vals[64][64];
  int idx = 0;
  int cidx = 0;
  char *sp = cmd;
  struct space *space=NULL;
  struct space *attr=NULL;
  int rc;

  rc = 1;
  while((rc>0) && (idx < 64))
    {
      rc = sscanf(sp, "%s"
	      , vals[idx]
		  );
      if(rc>0)
	{
	  sp = strstr(sp, vals[idx]);
	  sp += strlen(vals[idx]);
	  while (*sp && (*sp == ' ')) sp++;
	  printf("rc %d val[%d] [%s] ", rc, idx, vals[idx]);
	  printf("sp [%s] \n", sp);
	  idx++;
	}
    }
  cidx =  0;
  
  if(strcmp(vals[cidx], "ADD") == 0)
    {
      //make_space(&g_space, idx, vals, stuff, buf, len);
      make_space(&g_space, stuff, bufp, len);
      return 0;
    }
  //return run_str_add(stuff,buf,len);
  else if(strcmp(vals[cidx], "SET") == 0)
    {
      rc = set_space(g_space, stuff, NULL, bufp, len);
      return rc ; 
    }
  else if(strcmp(vals[cidx], "GET") == 0)
    {
      rc = 0;
      sp = get_space(g_space, stuff, bufp, len);
      if(bufp)
	{
	  int xlen = strlen(sp) +1;
	  char *buf = malloc(xlen);
	  snprintf(buf,xlen,"%s",sp);
	  rc = strlen(buf);
	  *bufp = buf;
	  *len = rc;

	}
      return rc;
    }
  return 0;
}


int run_str_in(struct insock *in, char *str, char **bufp, int *len)
{
  int rc;
  rc =  run_str(str, bufp,len);
  return rc;
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



int init_cmds(void)
{
  int i;
  for (i = 0; i< NUM_CMDS; i++)
    {
      cmds[i].key = NULL;
      cmds[i].handler = NULL;
    }
  return i;
}
		 
int init_cmd(char *key, int (*hand)(void *key, int n, char *data, int speed, int time))
{
  int i;
  for (i = 0; i< NUM_CMDS; i++)
  {
    if(cmds[i].key == NULL)
      {
	cmds[i].key =  key;
	cmds[i].handler = hand;
	break;
      }
  }
  if(i == NUM_CMDS) i = -1;
  return i;
}


int run_cmd (char *key, int n, char *data, int speed, int time)
{
  int rc=-1;
  int i;
  for (i = 0; i< NUM_CMDS; i++)
    {
      if(cmds[i].key && (strcmp(cmds[i].key, key) == 0))
	{
	  rc = cmds[i].handler(&cmds[i], n, data, speed, time);
	  break;
	}
    }
  return rc;
}


int init_insocks(void)
{
  int i;
  for (i = 0; i< NUM_SOCKS; i++)
  {
      insock[i].fd = -1;
      insock[i].inptr = 0;
      insock[i].inlen = 0;
      insock[i].insize = INSIZE;
      insock[i].outsize = OUTSIZE;
      insock[i].outptr = 0;
      insock[i].outlen = 0;
      insock[i].iobuf = NULL;
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

struct iobuf *new_iobuf(char *buf, int len)
{
  struct iobuf *iob = (struct iobuf *)malloc(sizeof (struct iobuf));
  int xlen = len+64;
  iob->outptr=len;
  iob->outlen=xlen;
  iob->outbuf = (char *)malloc(xlen);
  memcpy(iob->outbuf,buf,len);
  iob->prev = iob;
  iob->next = iob;
  return iob;
}

int extend_iobuf(struct iobuf * iob,char *buf, int len)
{
  memcpy(iob->outbuf+iob->outptr,buf,len);
  iob->outptr+=len;
}

int add_iobuf(struct insock *in, char *buf, int len)
{
  struct iobuf *iob = NULL;
  struct iobuf *ciob = NULL;
  struct iobuf *piob = NULL;
  struct iobuf *niob = NULL;
  if (in->iobuf != NULL)
    {
     iob = in->iobuf;
    }
  else
    {
      iob = new_iobuf(buf, len);
      in->iobuf = iob;
      return 0;
    }
  if (iob->outptr+len > iob->outlen)
    {
      iob = new_iobuf(buf, len);
      ciob = in->iobuf;
      niob = ciob->next;
      piob = ciob->prev;
      if (piob == niob)
	{
	  ciob->next = iob;
	  ciob->prev = iob;
	  iob->next = ciob;
	  iob->prev = ciob;
	}
      else
	{
	  iob->next = ciob;
	  ciob->prev = iob;
	  piob->next = iob;
	  iob->prev = piob;
	}
    }
  else
    {
      extend_iobuf(iob,buf, len);
    }
  return 0;
}

int pull_iob(struct insock *in, char **bufp, int *lenp)
{
  struct iobuf *piob = NULL;
  struct iobuf *niob = NULL;
  struct iobuf *iob = NULL;
  int rc = -1;
  if (in->iobuf != NULL)
    {
     iob = in->iobuf;
     if(bufp) *bufp = iob->outbuf;
     if(lenp) *lenp = iob->outptr;
     if (iob->prev == iob->next)
       {
	 in->iobuf = NULL;
	 free(iob);
	 return 0;
       }
     else
       {
	 niob = iob->next;
	 piob = iob->prev;
	 in->iobuf = niob;
         niob->prev = piob;
	 free(iob);
	 return 1;
       }
    }
  return rc;
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

     printf( "using port #%d\n", portno );
    
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     {
	 printf("ERROR opening socket");
	 return -1;
     }
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
     for (i = 0; i< NUM_SOCKS; i++)
     {
	 if (insock[i].fd < 0)
	 {
	     insock[i].fd = newsock;
	     insock[i].inptr = 0;
	     insock[i].inlen = 0;
	     insock[i].outptr = 0;
	     insock[i].outlen = 0;
	     num_socks++;
	     break;
	 }
     }
     return newsock;
}

struct insock *find_fd(int fsock)
{
    int i;
    struct insock *in = NULL;
    for (i = 0; i< NUM_SOCKS; i++)
    {
        if (insock[i].fd == fsock)
        {
	    in = &insock[i];
	    break;

        }
    }
    return in;
}

int close_fds(int fsock)
{
    int rc = -1;
    struct insock *in = find_fd(fsock);
    if (in)
    {
	rc = 0;
	in->fd = -1;
	in->inptr = 0;
	in->inlen = 0;
	in->outptr = 0;
	in->outlen = 0;
	num_socks--;
    }
    return rc;
}

/*
  commands are 
  fwd [speed] [time]
  back [speed] [time]
  stop
  right [speed] [time]
  left [speed] [time]
*/

int handle_input(struct insock *in)
{
    int rc;
    int len;
    int n;
    char cmd[64];
    char *sp;
    char buf[1024];
    char *bres;
    int lres;
    char * bufp;

    len = read(in->fd,&in->inbuf[in->inptr],in->insize-in->inptr);

    if(len > 0)
      {
	in->inlen += len;
	in->inbuf[in->inlen] = 0;
	sp = &in->inbuf[in->inptr];
	n = sscanf(sp,"%s ", cmd);
	rc = snprintf(&in->outbuf[in->outlen], in->outsize-in->outptr
		      ," message received [%s] ->"
		      " n %d cmd [%s]\n"
		      , &in->inbuf[in->inptr]
		      , n, cmd );
	in->outlen =+ rc;
	//run_cmd (cmd, n, sp, speed, time);
	run_str_in(in, sp, &bufp, &lres);
	printf(" rc %d n %d cmd [%s]\n"
	       , rc, n, cmd );
	in->outlen =+ rc;
	in->inptr= 0;
	in->inlen= 0;
      }
    return len;
}

int handle_output(struct insock *in)
{
    int rc;
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
    struct insock *in = NULL;
    int rc = 1;    
    //fds[idx].fd = STDIN_FILENO;
    //fds[idx].events = POLLIN;
    //fds[idx].revents = 0;
    //idx++;
    if(lsock>0)
      {
	fds[idx].fd = lsock;
	fds[idx].events = POLLIN;
	fds[idx].revents = 0;
	idx++;
      }
    
    for (i = 0; i< NUM_SOCKS; i++)
      {
	if (insock[i].fd >= 0)
	  {
	    fds[idx].fd = insock[i].fd;
	    fds[idx].events = POLLIN;
	    fds[idx].revents = 0;
	    if(insock[i].outptr != insock[i].outlen)
	      {
		fds[idx].events |= POLLOUT;
	      }
	    idx++;
	  }
      }

    printf("poll start idx %d\n", idx);
    ret =  poll(fds, idx, timeout);
    printf("poll done ret = %d idx %d\n", ret, idx);

    if(ret > 0) 
    {
	printf("poll ret = %d idx %d\n", ret, idx);
	
	for( i = 0; i < idx; i++) 
	{
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
	      if((lsock > 0)&& (fds[i].fd == lsock))
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
			    printf("error reading (n=%d), closing fd %d \n",n,fds[i].fd);
			    close(fds[i].fd);
			    close_fds(fds[i].fd);
			}
			else 
			{
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

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500", NULL, NULL, 0);
int set_space(struct space * base, char *name, char *value, char **buf, int *len)
{
  int rc = -1;
  struct space *sp1;
  char sname[2][128];  // TODO
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
	sp1->onset(sp1, sp1->idx, name, spv, buf, len);
      rc = 0;
    }
  return rc;
}

char *get_space(struct space *base, char *name, char **buf, int *len)
{
  char * sret=NULL;
  struct space *sp1;
  char *sp;
  char spv[2][128];
  int rc;
  spv[0][0]=0;
  spv[1][0]=0;
  rc = sscanf(name,"%s %s", spv[0], spv[1]);
  sp = spv[0];
  if( rc == 2 ) sp = spv[1];
  sp1 = find_space_new(base, sp);
  if(sp1)
    {
      if(sp1->onget)
	sp1->onget(sp1, sp1->idx, name, buf, len);
      sret = sp1->value;
    }
  return sret;
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

int motor_onset(struct space *this, int idx, char *name, char *value, char **buf, int *len)
{
  printf(" running %s for space [%s] %d with value [%s]\n"
	 , __FUNCTION__, this->name, idx, value);
  return 0;

}
int motor_onget(struct space *this, int idx, char *name, char **buf, int *len)
{
  printf(" running %s for space [%s] %d \n"
	 , __FUNCTION__, this->name, idx);
  return 0;

}
int speed_onset(struct space *this, int idx, char *name, char * value, char **buf, int *len)
{
  printf(" running %s for space [%s] %d with value [%s]\n"
	 , __FUNCTION__, this->name, idx, value);
  return 0;

}
int speed_onget(struct space *this, int idx, char *name, char **buf, int *len)
{
  printf(" running %s for space [%s] %d \n"
	 , __FUNCTION__, this->name, idx);
  return 0;

}
int main (int argc, char *argv[])
{
   int i;
   int lsock = 0;
   int csock;
   int rc = 1;
   int depth=0;
   //struct space * sp1 = new_space("Space1", struct space *parent, struct space** root, char *node)
   struct space *sp1;
   struct space *sp2;
   //struct space *sp3;
   char buf[2048];
   char *sp;
   char *vals[64];
   init_g_spaces();
   init_insocks();

   if(argc > 1)
     {
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
	   printf ("sending [%s] to server %d \n", buf, csock);
	   if(csock> 0)
	     {
	       accept_socket(csock);
	       //rc = write(in->fd,&in->outbuf[in->outptr],in->outlen-in->outptr);
	       //rc = write(in->fd,&in->outbuf[in->outptr],in->outlen-in->outptr);
	       rc = write(csock, buf, strlen(buf)); 
	       lsock = -1;

	     }

	 }
     
       else if (strcmp(argv[1], "test") == 0)
	 {

	   char sbuf[4096];

	   rc = parse_stuff(' ', 64, (char **)vals, "this is a bunch/of/stuff to parse");
	   printf (" rc = %d\n", rc );
	   show_stuff(rc, vals);
	   
	   sp1 = make_space(&g_space, "ADD uav1/motor1/speed", NULL, 0);
	   show_spaces(g_space, "All Spaces 1 ", 0, NULL , 0);
	   
	   sp1 = make_space(&g_space, "ADD uav3/motor2/speed", NULL, 0);
	   sp1->onset = speed_onset;
	   sp1->onget = speed_onget;
	   rc  = set_space(g_space, "SET uav3/motor2/speed 3500", NULL, NULL, 0);
	   sp =  get_space(g_space,"GET uav3/motor2/speed", NULL,0);
	   printf(" >> %s value [%s]\n","uav3/motor2/speed", sp?sp:"no value");
	   return 0;
	   
	   sp1 = make_space(&g_space, "ADD uav1/motor1/size", NULL, 0);
	   show_spaces(g_space, "All Spaces 1 ", 0, NULL , 0);
	   sp1 = make_space(&g_space, "ADD uav1/motor2/speed", NULL, 0);
	   show_spaces(g_space, "All Spaces 2 ", 0, NULL , 0);
	   sp1 = make_space(&g_space, "ADD uav2/motor2/speed", NULL, 0);
	   show_spaces(g_space, "All Spaces 3 ", 0, NULL , 0);
	   sp1 = make_space(&g_space, "uav3/motor2/speed", NULL, 0);
	   
	   sp1 = find_space_new(g_space, "uav3/motor2");
	   sp1->onset = motor_onset;
	   sp1->onget = motor_onget;
	   
	   show_spaces_new(g_space, sbuf, 4096, NULL, 0);
	   sp1 = find_space_new(g_space, "uav1/motor3");
	   printf(" found %s \n", sp1?sp1->name:"no uav1/motor3");
	   sp1 = find_space_new(g_space, "uav1/motor1");
	   printf(" found %s \n", sp1?sp1->name:"no uav1/motor1");
	   sp2  = copy_space_new(sp1, "new_motor1");
	   rc  = set_space(g_space, "SET uav3/motor2/speed 3500", NULL, NULL, 0);
	   sp =  get_space(g_space,"GET uav3/motor2/speed", NULL,0);
	   printf(" >> %s value [%s]\n","uav3/motor2/speed", sp?sp:"no value");
	   
	   show_spaces_new(sp2, sbuf, 4096, NULL, 0);
	   
	   //   return 0;
#if 0
	   new_space("Space1", NULL, NULL, "127.0.0.1");
	   sp1 =  g_space;
	   show_space(sp1, 0, NULL , 0);
	   sp1 = new_space("Space2", sp1, NULL, "127.0.0.1");
	   show_space(g_space, 0, NULL , 0);
	   show_space(sp1, 0, NULL , 0);
	   sp1 = new_space("Space3", sp1, NULL, "127.0.0.1");
	   show_space(g_space, 0, NULL , 0);
	   show_space(sp1, 0, NULL , 0);
	   
	   //   sp1 = g_space;
	   show_spaces(g_space, "Global Spaces 1", 0, NULL , 0);
	   //           attr        space  class type  value 
	   
	   sp1 = g_space;
	   //struct space *new_space_attr_float(char *name , struct space *parent,1.234)
	   
	   sp2 = new_space_attr_float("foo_float", sp1, 1.2345);
	   sp2 = new_space_attr_int("foo_int", sp1, 2345);
	   sp2 = new_space_attr_str("foo_str", sp1, "x2345");
	   
	   show_spaces(sp1->attr, "Sp1 attr", 0, NULL , 0);
	   
#endif
#if 0	   
	   //sp1 = make_space(&g_space, "uav2/motor2/speed", NULL, 0);
	   run_str("ADD uavx/led1/on", buf, sizeof(buf));
	   run_str("ADD uavx/led1/color", buf, sizeof(buf));
	   run_str("ADD uavx/motor1/speed", buf, sizeof(buf));
	   run_str("ADD uavx/motor1/size", buf, sizeof(buf));
	   run_str("ADD uavx/motor2/speed", buf, sizeof(buf));
	   run_str("ADD uavx/motor2/size", buf, sizeof(buf));
	   
	   run_str("SET uavx/led1/on 1", buf, sizeof(buf));
	   run_str("SET uavx/led1/color red", buf, sizeof(buf));
	   
	   rc = run_str("GET uavx/led1/color", buf, sizeof(buf));
	   
	   printf("GET rc %d buf [%s]\n",rc, buf);

#endif
	   
#if 0
	   show_spaces(g_space, "Global Spaces 2", 0, NULL , 0);
	   rc = show_spaces(g_space, "Global Spaces Buf", 0, buf , sizeof(buf));
	   printf("rc %d buf [%s]\n",rc, buf);
	   return 0;
	   
	   init_cmds();
	   init_cmd("FWD", fwd_cmd);
	   run_cmd ("FWD", 2, "Some data", 100, 50);
#endif
	 }
     }
   
   accept_socket(STDIN_FILENO);
   if(lsock == 0)
     {
       lsock = listen_socket(5432);
     }
   rc = 1;
   while(rc>0 && count < 10)
   {
       rc = poll_sock(lsock);
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
