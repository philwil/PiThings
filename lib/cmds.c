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
#include "../inc/pithings.h"

extern struct cmds g_cmds[];
extern  struct cmds h_cmds[];
extern struct space *g_space;

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
  //  int rc;
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
  //struct iobuf * iob;
  //int cidx = 0;
  //char *sp = name;
  int new = 0;
  int i;
  //int rc;
  int idx = 0;
  int idv = 0;
  char *valv[64];
  char *valx[64];

  parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);

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
  //int rc = 0;
  int i = 0;
  int fd = 0;
  struct node *node = NULL;

  parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);

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
  // char *sp1 = stuff;
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
  return rc;
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
	  if(in)
	    {
	      in_snprintf(in, NULL, "%s -> %s\n"
			  , cmds->key
			  , cmds->desc
			  );
	    }
	  else
	    {
	      printf("%s -> %s\n"
			  , cmds->key
			  , cmds->desc
			  );
	    }
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

struct space *decode_cmd_in(struct space **base, char *name, struct iosock *in)
{
  int rc = 0;
  char sbuf[64];
  char scid[64];
  char sclen[64];
  int clen = 0;
  //struct space *sp1=NULL;
  //struct space **spb=&g_space;
  //char *sp = name;
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
  //struct space *sp1=NULL;
  //struct space **spb=&g_space;
  //char *sp = name;
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
  //struct space *space=NULL;
  //struct space *attr=NULL;
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
  return init_new_cmd(g_cmds, NUM_CMDS, key, desc, hand);

}

int init_new_hcmd(char *key, char *desc, struct space *(*hand)
		 (struct space ** base, char *name, struct iosock *in))
{
  return init_new_cmd(h_cmds, NUM_CMDS, key, desc, hand);
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
  //struct space *sp1 = NULL;
  for (i = 0; i< ncmds; i++, cmds++)
    {
      if(cmds->key && (strcmp(cmds->key, key) == 0))
	{
	  //sp1 =
	  cmds->new_hand(base, stuff, in);
	  break;
	}
    }
  rc = i;
  if(i == ncmds)
    rc = -1;
  return rc;
}

int iob_snprintf(struct iobuf *iob, const char *fmt, ...)
{
    int size;
    //char *sp = NULL;
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
