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

extern struct list *g_node_list;
extern struct list *g_conn_list;
extern struct list *g_space_list;

extern char *g_myname;
extern char *g_myaddr;
extern int g_port_no;


int show_hvals(char *key, char *hvals[], int num)
{
  int i;
  for (i=0; i<num; i++)
    {
      if(hvals[i])
	{
	  printf("==== %s @%d [%s]\n", key, i, hvals[i]);
	}
    }
  return 0;
}

int show_hmsg(struct hmsg *hm)
{
  printf("=======Show hmsg @%p=======\n",hm);
  printf("===   sp [%s]\n",hm->sp);
  printf("===action[%s]\n",hm->action);
  printf("===   url[%s]\n",hm->url);
  printf("===  vers[%s]\n",hm->vers);
  printf("==qstring[%s]\n",hm->qstring);
  printf("==   dlen[%d]\n",hm->dlen);
  printf("==   data[%s]\n",hm->data);
  show_hvals("    hvals1", hm->hvals1, NUM_HVALS1);
  show_hvals("    hvals2", hm->hvals2, NUM_HVALS2);
  show_hvals("    attrs",  hm->attrs,  NUM_ATTRS);
  show_hvals("    snames", hm->snames, NUM_SPACES);
  return 0;

}

int init_hmsg(struct hmsg *hm)
{
  int i;
  hm->sp = NULL;
  hm->url = NULL;
  hm->action = NULL;
  hm->qstring = NULL;
  hm->data = NULL;
  hm->dlen = 0;
  for (i=0; i<NUM_HVALS1; i++)
    {
      hm->hvals1[i] = NULL;
    }
  for (i=0; i<NUM_HVALS2; i++)
    {
      hm->hvals2[i] = NULL;
    }
  for (i=0; i<NUM_ATTRS; i++)
    {
      hm->attrs[i] = NULL;
    }
  for (i=0; i<NUM_SPACES; i++)
    {
      hm->snames[i] = NULL;
    }
  for (i=0; i<NUM_SPACES; i++)
    {
      hm->spaces[i] = NULL;
    }
  return 0;

}

void clean_sp(char **spp)
{
  char *sp;
  sp = *spp;
  if(sp)free(sp);
  *spp = NULL;
}

void clean_vals(char *vals[], int num)
{
  int i;
  for (i=0; i<num; i++)
    {
      if(vals[i]) free(vals[i]);
      vals[i] = NULL;
    }
}

void clean_spaces(struct space *spaces[], int num)
{
  int i;
  for (i=0; i<num; i++)
    {
      if(spaces[i]) free_space(spaces[i]);
      spaces[i] = NULL;
    }
}

int clean_hmsg(struct hmsg *hm)
{
  clean_sp(&hm->sp);
  clean_sp(&hm->url);
  clean_sp(&hm->action);
  clean_sp(&hm->qstring);
  clean_sp(&hm->vers);
  clean_sp(&hm->data);
  hm->dlen = 0;
  clean_vals(hm->hvals1, NUM_HVALS1);
  clean_vals(hm->hvals2, NUM_HVALS2);
  clean_vals(hm->attrs,  NUM_ATTRS);
  clean_vals(hm->snames, NUM_SPACES);

  return 0;
}


char *get_valx(char *valx[], int num)
{
  char *sp;
  sp = valx[num];
  valx[num] =  NULL;
  return sp;
}

int null_valx(char *valx[], int num)
{
  while(num--)
    {
      if(valx[num]) 
	{
	  //free (valx[num]);
	  valx[num] = NULL;
	}
    }
  return num;
}

int clean_valx(char *valx[], int num)
{
  while(num--)
    {
      if(valx[num]) 
	{
	  free (valx[num]);
	  valx[num] = NULL;
	}
    }
  return num;
}

int decode_content_length(char *hsp)
{
  char tmp[128];
  char *tsp;
  int dlen;

  hsp +=strlen("Content-Length:");
  tsp = tmp;
  tsp[0]=0;
  while (*hsp && (*hsp == ' ') && (*hsp != '\n') )hsp++; 
  //printf(" After no Space sp [%s]\n",hsp);
  while (*hsp && (*hsp != '\n') )*tsp++ = *hsp++;
  *tsp = 0;
  //printf(" After decode tmp [%s]\n",tmp);
  dlen = atoi(tmp);
  //printf(" After decode dlen [%d]\n",dlen);
  return dlen; 
}

char *find_data(char *hsp)
{
  char *sp = NULL;
  sp = strstr(hsp,"\r\n\r\n");
  if(sp)sp  += strlen("\r\n\r\n");
  if (!sp)sp = strstr(hsp,"\n\r\n\r");
  if(sp)sp  += strlen("\n\r\n\r");
  if (!sp)sp = strstr(hsp,"\n\n");
  if(sp)sp  += strlen("\n\n");
  if(sp)
    {
      printf(" After %s  [%x] [%x]\n",__FUNCTION__, sp[0], sp[1]);
    }
  else
    {
      printf(" After %s  data not found\n",__FUNCTION__);
    }
  return sp;
}

char *setup_hmsg(struct hmsg *hm, char *insp)
{
  int idx;
  char *sp=NULL;
  /*
  sp ="POST /pine1/gpios/gpio1?value=somenewvalue&fum=2345 HTTP/1.1\n"
    "User-Agent: curl/7.26.0\n"
    "Host: 127.0.0.1:5432\n"
    "Accept: xxx\n"
    "Content-Length: 19\n"
    "Content-Type: application/x-www-form-urlencoded\n\r"
    "\n\r"
    "thisistherealnumber\n";
*/
  if(!hm->sp)
    hm->sp = strdup(insp);
  idx = parse_stuff(' ', 3 , (char **)hm->hvals1, hm->sp,' ');
  if(1)printf(" %s parse_stuff 1  idx %d got [%s] [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , hm->hvals1[0]
	      , hm->hvals1[1]
	      , hm->hvals1[2]
	      );
  hm->action = get_valx(hm->hvals1, 0);  //POST GET
  hm->vers = get_valx(hm->hvals1, 2);  //POST GET
  sp = get_valx(hm->hvals1, 1);                // uri + query

  idx = parse_stuff('?', 4 , (char **)hm->hvals2, sp,'?');
  if(1)printf(" %s parse_stuff 2 idx %d got [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , hm->hvals2[0]
	      , hm->hvals2[1]
	      );

  hm->url = get_valx(hm->hvals2, 0);  //url
  hm->qstring = get_valx(hm->hvals2, 1);  //query
  sp = hm->qstring;
  if(sp)
    {
      idx = parse_stuff('&', 8 , (char **)hm->attrs, sp,' ');
      if(1)printf(" %s parse_stuff idx %d got [%s] [%s] [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , hm->attrs[0]
	      , hm->attrs[1]
	      , hm->attrs[2]
	      , hm->attrs[3]
	      );
    }
  sp = hm->url;
  if(sp)
    {
      idx = parse_stuff('/', 8 , (char **)hm->snames, sp,' ');
      if(1)printf(" %s parse_stuff idx %d got [%s] [%s] [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , hm->snames[0]
	      , hm->snames[1]
	      , hm->snames[2]
	      , hm->snames[3]
	      );
    }
  // "Content-Length: 19\n"
  hm->dlen = 0;
  hm->data = NULL;
  sp = strstr(insp, "Content-Length:");
  if(sp)
    {
      //printf("decode content len from [%s]\n",sp);
      hm->dlen = decode_content_length(sp);
    }
  if(hm->dlen)
    sp = find_data(hm->sp);
  if(sp)
    {
      hm->data=strdup(sp);
      hm->data[hm->dlen]=0;
    }
  else
    {
      hm->dlen =0;
      hm->data =NULL;
    }
  return sp;
}

char *get_query(char *sp, char *qname)
{
  char *valx[16];
  int idx;
  int i;

  null_valx(valx, 16);

  /*
  sp ="POST /pine1/gpios/gpio1?value=somenewvalue&fum=2345 HTTP/1.1\n"
    "User-Agent: curl/7.26.0\n"
    "Host: 127.0.0.1:5432\n"
    "Accept: xxx\n"
    "Content-Length: 19\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "\n"
    "thisistherealnumber\n";
*/
  
  idx = parse_stuff('?', 4 , (char **)valx, sp,'?');
  if(1)printf(" %s parse_stuff idx %d got [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , valx[0]
	      , valx[1]
	      );

  sp = get_valx(valx, 1);
  clean_valx(valx, 4);
  if(sp)
    {
      idx = parse_stuff('&', 8 , (char **)valx, sp,' ');
      if(1)printf(" %s parse_stuff idx %d got [%s] [%s] [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , valx[0]
	      , valx[1]
	      , valx[2]
	      , valx[3]
	      );
      free(sp);
      
      sp = get_valx(valx, 0);
      clean_valx(valx, 8);
      
      idx = parse_stuff('=', 8 , (char **)valx, sp,' ');
      if(1)printf(" %s parse_stuff idx %d got [%s] [%s] [%s] [%s]\n"
		  , __FUNCTION__
		  , idx
		  , valx[0]
		  , valx[1]
		  , valx[2]
		  , valx[3]
		  );
      
      free(sp);
      sp = NULL;
      for (i = 0; i< idx; i+=2)
	{
	  if ((i<(idx-1)) && valx[i] && (strcmp(valx[i], qname) == 0))
	    {
	      sp = get_valx(valx, i+1);
	      break;
	    }
	}
      
      clean_valx(valx, 8);
    }
  return sp;
}


char *get_uri(char *sp)
{
  char *valx[8];
  int idx;
  
  null_valx(valx, 8);

  /*
  sp ="POST /pine1/gpios/gpio1?value=somenewvalue&fum=2345 HTTP/1.1\n"
    "User-Agent: curl/7.26.0\n"
    "Host: 127.0.0.1:5432\n"
    "Accept: xxx\n"
    "Content-Length: 19\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "\n"
    "thisistherealnumber\n";
*/
  
  idx = parse_stuff(' ', 4 , (char **)valx, sp,'?');
  if(1)printf(" %s get_uri 1 idx %d got [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , valx[0]
	      , valx[1]
	      );
  sp = get_valx(valx, 1);

  idx = parse_stuff('?', 4 , (char **)valx, sp,'?');
  if(1)printf(" %s get_uri 1 idx %d got [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , valx[0]
	      , valx[1]
	      );

  free(sp);

  sp = get_valx(valx, 0);
  clean_valx(valx, 4);

  return sp;
}

int test_parse_stuff(void)
{
  char *sp;
  char *rsp;
  sp ="POST /pine1/gpios/gpio1?value=somenewvalue&fum=2345 HTTP/1.1\n"
    "User-Agent: curl/7.26.0\n"
    "Host: 127.0.0.1:5432\n"
    "Accept: */*\n"
    "Content-Length: 19\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "\n"
    "thisistherealnumber\n";
  rsp =   get_uri(sp);
  if(1)printf(" %s get_ui got [%s]\n"
	      , __FUNCTION__
	      , rsp
	      );
  free(rsp);
  rsp =   get_query(sp, "value");
  if(1)printf(" %s get_ui got [%s]\n"
	      , __FUNCTION__
	      , rsp
	      );
  free(rsp);
  return 0;
}

int test_hmsg(void)
{
  int idx=0;
  char *sp;
  struct hmsg hmsg;

  sp ="POST /pine1/gpios/gpio1?value=somenewvalue&fum=2345 HTTP/1.1\n"
    "User-Agent: curl/7.26.0\n"
    "Host: 127.0.0.1:5432\n"
    "Accept: */*\n"
    "Content-Length: 19\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "\n"
    "thisistherealnumber\n";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  clean_hmsg(&hmsg);

  sp ="GET /pine1/gpios/gpio1\n\n";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  clean_hmsg(&hmsg);

  sp ="GET /pine1/gpios/gpio1?value=1\n\n";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  clean_hmsg(&hmsg);

  sp ="POST /pine1/gpios/gpio21?value=1&action=set\n Content-Length: 6\n\n123456789\n\n";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  //clean_hmsg(&hmsg);

  idx = find_hmsg_spaces(&g_space_list,&hmsg);
  printf("idx val = %d\n", idx);
  idx = add_hmsg_spaces(&g_space_list,&hmsg);
  printf("idx val = %d\n", idx);
  idx = find_hmsg_spaces(&g_space_list,&hmsg);
  printf("idx val = %d\n", idx);

  return 0;
}

struct space*find_space_name(struct list *root, char *name)
{
  struct space *sp1 = NULL;
  struct list *item = root;
  struct list *ritem = NULL;

  while (foreach_item(&ritem, &item))
    {
      if(item->data) 
	{
	  sp1 = item->data;
	  if (strcmp(sp1->name, name) == 0)
	    {
	      break;
	    }
	  else
	    {
	      sp1 = NULL;
	    }
	}
    }
  return sp1;
}



int do_hmsg_spaces(struct list **root, struct hmsg *hm, int add)
{
  int i = -1;
  struct space *sp1;
  struct space *sp2=NULL;
  struct list *sp_list;
  //struct list * item;;
  char *sp;
  int idx=-1;

  sp_list = *root;
  //  item = g_space_list;


  for (i=0; i<NUM_SPACES; i++)
    {
      sp = hm->snames[i];
      //if (*sp == '/') sp++;
      printf(" looking for %d [%s]\n", i, sp?sp:"none");
      if(hm->snames[i] && (strlen(hm->snames[i]) > 0))
	{
	  sp = hm->snames[i];
	  //if (*sp == '/') sp++;
	  printf(" looking for [%s] ... ", sp);

	  sp1 = find_space_name(sp_list, sp);
	  if(sp1)
	    {
	      printf(" found name [%s]\n", sp1->name);
	      sp_list = sp1->child;
	      hm->spaces[1] = sp1;
	      idx = sp1->idx;
	    }
	  else
	    {
	      if(add)
		{
		  printf(" .. adding [%s] !\n", sp);
		  sp1=new_space(sp,sp2,&sp_list,NULL);
		  idx = sp1->idx;
		}
	      else
		{
		  printf(" .. urgh !\n");
		  idx = -1;
		}
	    }
	  if(!sp1 || !sp_list)
	    {
	      printf(" List finised at index %d\n", i); 
	      break;
	    }
	}
      else
	{
	      printf(" List finised at index %d\n", --i); 
	      break;
	}
      sp2 = sp1;
    }
  return idx;
}

int find_hmsg_spaces(struct list **root, struct hmsg *hm)
{
  return do_hmsg_spaces(root, hm, 0);
}

int add_hmsg_spaces(struct list **root, struct hmsg *hm)
{
  return do_hmsg_spaces(root, hm, 1);
}

int process_hmsg(struct hmsg *hmsg)
{
  //char *cmd;
  //char *action;
  return 0;
}

//  sock = connect_socket(iport, addr);
//int dummy_hand(int fd, char *id, char *buf, int len)
//      init_new_hand("some_id", "Dummy Handler",  dummy_handler);
// run_new_gcmd
//      add_socket(csock);
//      in->fd = -1;
int send_command(int sock, char *buf, int blen, char *id)
{
  char sbuf[128];
  int rc;
  int ret = 0;
  snprintf(sbuf, blen,"CMD %s %d\n\n", id, blen);
  rc = write(sock, buf, strlen(buf)); 
  ret =  rc;
  if(rc >0)
    {
      rc = write(sock, buf, blen);
    }
    if(rc >0)
      {
	ret += rc;
      }
    else
      {
	ret =  rc;
      }
    return ret;
}


//   idx = parse_stuff(' ', 64, (char **)valx, name);
//   rc = parse_name(&idx (char **)valx, &idy (char **)valy, 64, name);
int parse_name(int *idx, char **valx, int *idy , char **valy, int size, char *name)
{
  int  rc = 1;
  char *sp;
  *idx = parse_stuff(' ', size, (char **)valx, name, 0);
  if(g_debug)
    {
      printf(" parse_name 1 name [%s] *idx %d valx[0/1] %p/%p\n", name, *idx
	     , valx[0], valx[1]); 
      if(*idx>0)
	{
	  printf(" parse_name 1 valx[0] [%s]\n"
		 , valx[0]);
	}
      if(*idx>1)
	{
	  printf(" parse_name 1 valx[1] [%s]\n"
		 , valx[1]);
	}
    }
    sp = valx[0];
  if(*idx >= 2)sp = valx[1];

  *idy = parse_stuff('/', size, (char **)valy, sp,'?');
  if(g_debug)
    printf(" parse_name 2 name [%s] *idy %d valy[0/1] %p/%p [%s]-[%s]\n"
	      , name, *idy
	      , valy[0], valy[1]
	      , valy[0], (*idy>1)?valy[1]:"none"
	      ); 
  return rc;
}

struct space *cmd_html_expect(struct list **root, char *name,
			    struct iosock *in)
{
  int i;
  int idx = 0;
  char *valx[64];

  idx = parse_stuff(' ', 64, (char **)valx, name,'\n');
  //in->hlen = 0;
  
  for (i = 0; i<idx; i++)
    {
      printf(" >> String %d [%s]\n", i, valx[i]);
    }
  
  if(idx>1)
    {
      if(strstr(valx[1],"100-continue"))
	{
	  
	  send_html_continue(in, NULL);
	}
    }
  // TODO free vals
  return NULL;
}

struct space *cmd_html_cont(struct list **root, char *name,
			    struct iosock *in)
{
  int i;
  int idx = 0;
  char *valx[64];
  char *spm = "multipart/form-data;";
  char *spb = "boundary=";
  char *spv = NULL;
  idx = parse_stuff(' ', 64, (char **)valx, name,'\n');
  //in->hlen = 0;
  
  for (i = 0; i<idx; i++)
    {
      printf(" >> String %d [%s]\n", i, valx[i]);
    }
  
  if(idx>2)
    {
      if(strstr(valx[1],spm))
	{
	  spv = valx[2];
	  spv += strlen(spb);
	  printf(" >> >>found boundary as [%s]\n", spv);
		 
	}
    }
  // TODO free vals
  return NULL;
}
//
struct space *cmd_html_len(struct list **root, char *name,
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

struct space *cmd_html_host(struct list **root, char *name,
			    struct iosock *in)
{
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
      if(in->host)free(in->host);
      in->host = strdup(valx[1]);
      printf(" %s setting in %p host  [%s]\n"
	     , __FUNCTION__
	     , in
	     , in->host
	     );
      
    }

  return NULL;
}

struct space *cmd_html_refer(struct list **root, char *name,
			    struct iosock *in)
{
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
      if(in->referer)free(in->referer);
      in->referer = strdup(valx[1]);
      printf(" %s setting in %p referer  [%s]\n"
	     , __FUNCTION__
	     , in
	     , in->referer
	     );
      
    }

  return NULL;
}

struct space *cmd_html_dummy(struct list **root, char *name,
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


struct node *new_node(char *aport, char *addr)
{
  struct list *item;
  struct node * node;
  item = get_node_list(&g_node_list, addr, atoi(aport));
  node = (struct node *) item->data;
  //node->addr = strdup(addr);
  //node->port = atoi(aport);

  if(node->fd == -1)
    {
      node->fd = connect_socket(node->port, node->addr);;
    }
  node->in = NULL;
  return node;
}

// CONN pine1 127.0.0.1 5432 
struct node *new_conn(char *aport, char *addr, char *name)
{
  struct list *item;
  struct node * node;
  char cmd[1024];
  item = get_node_list(&g_conn_list, addr, atoi(aport));
  node = (struct node *) item->data;
  //node->addr = strdup(addr);
  //node->port = atoi(aport);

  if(node->fd == -1)
    {
      node->fd = connect_socket(node->port, node->addr);;
    }
  if(node->fd > 0)
    {
      snprintf(cmd, 1024, "NODE %s %s %d\n\n"
	       , name
	       , g_myaddr
	       , g_port_no
	       );
	printf("sending command [%s] to %s:%d fd:%d g_conn_list %p\n"
	       , cmd
	       , node->addr
	       , node->port
	       , node->fd
	       , g_conn_list
	       );

	write(node->fd, cmd, strlen(cmd));
    }
  //TODO 
  node->in = NULL;
  return node;
}

// struct node
// "NODE name addr port 
struct space *add_node_in(struct list **root, char *name,
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
      printf(" %s >> Arg %d [%s]\n", __FUNCTION__ , i, valx[i]);
    }
  // connect
  space = add_space_in(root, name, in);

  if(idx >= 3)
    {
      node =  new_node(valx[3], valx[2]);
      fd = node->fd;
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
// struct node
// "NODE name/n/n addr port 
struct space *add_conn_in(struct list **root, char *name,
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

  if(g_myname == NULL )
    {
      printf(" name not set up for connect\n");
      return space;
    }
  parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);

  for (i = 0; i<idx; i++)
    {
      printf(" %s >> Arg %d [%s]\n", __FUNCTION__ , i, valx[i]);
    }
  // connect
  space = add_space_in(root, name, in);

  if(idx >= 3)
    {
      node =  new_conn(valx[3], valx[2], g_myname);
      fd = node->fd;
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


struct space *add_space(struct list **root, char *name)
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

// look ofr the double terms
int spisterm (char sp)
{
  int rc  = 0;
  if((sp == 0xa) || (sp == 0xd))
    rc = 1;
  return rc;
}
// This will do for now
// TODO dynamic sizing
//      allow escape
//
//  *idx = parse_stuff(' ', size, (char **)valx, name, 0);

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
      // used to break if term found and we did not have a delim
      while (*sp && (*sp!= cstop) && ((skip == 1) || (*sp != delim)) && (rc < (val_size-1)))
	{
	  skip = 0;
	  //	  if ((*sp != 0xa)&&(*sp != 0xd) &&(*sp != cstop))
	  if (!spisterm(sp[0]) && (*sp != cstop))
	    {
	      rc++;
	      //if(g_debug)
	      //printf(" xxx processing %x \n", *sp);
	      *spv++ = *sp;

	    }
	  if(*sp && *sp == cstop) done = 1;
	  if(*sp && spisterm(sp[1]))
	    {
	      sp++;
	      rc++;
	      done = 1;
	      break;
	    }
	  if(*sp)sp++;
	}
      if(*sp)sp++;
      *spv = 0;
	
      if(rc>0)
	{
	  vals[idx] = strdup(val);
	  if(g_debug)
	    printf("rc %d val [%s] val[%d] [%s] %x %x\n"
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
  init_new_gcmd("HELP", "Print this help",              help_new_gcmds);
  init_new_gcmd("ADD",  "Create a new space",           add_space_in);
  init_new_gcmd("SET",  "Set a (string) value",	        set_space_in);
  init_new_gcmd("GET",  "Process HTML GET",	        get_html_in);
  init_new_gcmd("POST",  "Process HTML POST",           get_html_in);
  init_new_gcmd("SEE",   "See (Get) a value",           get_space_in);
  init_new_gcmd("CMD", "determine command id and len",  decode_cmd_in);
  init_new_gcmd("REP", "determine  replyid and len",    decode_rep_in);
  init_new_gcmd("SHOW", "Show spaces from a root",      show_space_in);
  init_new_gcmd("NODE", "Allow remote items",           add_node_in);
  init_new_gcmd("CONN", "Register with this NODE",      add_conn_in);
  init_new_gcmd("QUIT", "quit",      cmd_quit);


  init_new_hcmd("Referer:",        "Host: Port",      cmd_html_refer);
  init_new_hcmd("Host:",           "Host: Port",      cmd_html_host);
  init_new_hcmd("User-Agent:",     "User Agent",      cmd_html_dummy);
  init_new_hcmd("Accept:",         "Accept",          cmd_html_dummy);
  init_new_hcmd("Content-Type:",   "Content type",    cmd_html_cont);
  init_new_hcmd("Content-Length:", "Content length",  cmd_html_len);
  init_new_hcmd("Expect:",         "100-continue",    cmd_html_expect);
  //init_new_hcmd("multipart/form-data;", "form boundary",cmd_html_mpf);

  //  Host: 127.0.0.1:5432
  //User-Agent: curl/7.47.0
  //Accept: */*
  //Content-Length: 4
  //Content-Type: application/x-www-form-urlencoded
  return rc;
}

struct space *help_new_gcmds(struct list **base, char *name, struct iosock *in)
{
  return help_new_cmds(g_cmds, NUM_CMDS, base, name, in);
}

struct space *help_new_cmds(struct cmds *cmds, int n,struct list **base, char *name, struct iosock *in)
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

struct space *cmd_quit(struct list **base, char *name, struct iosock *in)
{
  //main
  printf("quitting\n");
  //if(g_lsock>0) close(g_lsock);
  if(in->fd>0) close(in->fd);
  in->fd = -1;
  return NULL;

}

struct space *decode_cmd_in(struct list **base, char *name, struct iosock *in)
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
struct space *decode_rep_in(struct list **base, char *name, struct iosock *in)
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
  rc = run_new_gcmd (cmd, &g_space_list, stuff, in);
  if(rc >= 0) return 0;
  if(strcmp(cmd, "ADD") == 0)
    {
      add_space_in(&g_space_list, stuff, in);
      return 0;
    }
  else if(strcmp(cmd, "SET") == 0)
    {
      set_space_in(&g_space_list, stuff, in);
      return 0 ; 
    }
  else if(strcmp(cmd, "GET") == 0)
    {
      rc = 0;
      get_space_in(&g_space_list, stuff, in);
      return rc;
    }
  else if(strcmp(cmd, "SHOW") == 0)
    {
      rc = 0;
      show_space_in(&g_space_list, stuff, in);
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
		 (struct list ** base, char *name, struct iosock *in))
{
  return init_new_cmd(g_cmds, NUM_CMDS, key, desc, hand);

}

int init_new_hcmd(char *key, char *desc, struct space *(*hand)
		 (struct list ** base, char *name, struct iosock *in))
{
  return init_new_cmd(h_cmds, NUM_CMDS, key, desc, hand);
}


int init_new_cmd(struct cmds *cmds, int ncmds , char *key, char *desc, struct space *(*hand)
		 (struct list ** base, char *name, struct iosock *in))
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

int run_new_gcmd(char *key, struct list **list, char *stuff, struct iosock *in)
{
  return run_new_cmd(g_cmds, NUM_CMDS, key, list, stuff, in);
}
int run_new_hcmd(char *key, struct list **list, char *stuff, struct iosock *in)
{
  return run_new_cmd(h_cmds, NUM_CMDS, key, list, stuff, in);
}



int run_new_cmd(struct cmds *cmds, int ncmds, char *key, struct list **list, char *stuff, struct iosock *in)
{
  int rc=-1;
  int i;
  //struct space *sp1 = NULL;
  for (i = 0; i< ncmds; i++, cmds++)
    {
      if(cmds->key && (strcmp(cmds->key, key) == 0))
	{
	  //sp1 =
	  cmds->new_hand(list, stuff, in);
	  break;
	}
    }
  rc = i;
  if(i == ncmds)
    {
      rc = -1;
      printf("%s unable to decode key [%s]\n", __FUNCTION__, key);
    }
  return rc;
}

#if 0
int xiob_snprintf(struct iobuf *iob, const char *fmt, ...)
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
#endif
int g_new_iob_size = 128;

//in
// now uses iobuf_list
// need to populate three pointers
// in->oubuf_list
// in->ouitem
// in->oubuf


int in_snprintf(struct iosock *in, struct iobuf *xiob, const char *fmt, ...)
{
  int size;
  struct iobuf *iob = NULL;
  //struct iobuf *ciob;
  struct list *item;
  va_list args;
  if(g_debug)
    printf(" %s starting in (%p) xiob %p oubuf (%p) ouitem (%p)\n"
	   , __FUNCTION__
	   , in, xiob
	   , in?in->oubuf:0
	   , in?in->ouitem:0
	 );

  if(xiob)
    {
      iob = xiob;
    }
  else
    {
      if(!in) return -1;
    
      if(!in->ouitem)
	{
	  item = new_iobuf_item(g_new_iob_size);
	  in->ouitem = item;
	  in->oubuf = item->data;
	  iob = item->data;
	  iob->outptr = 0;
	  iob->outlen = 0;
	  if(g_debug)
	    printf(" %s got iob %p size %d\n"
		   , __FUNCTION__
		   , iob, iob->outsize);
	  //item=new_list(iob);
	  push_list(&in->oubuf_list, item);

	}
    }

  iob = in->oubuf;
  va_start(args, fmt);
  size = vsnprintf(iob->outbuf, 0, fmt, args) +1;
  va_end(args);
  if(g_debug)
    printf(" size needed %d iob size %d space left %d\n"
	   , size
	   , iob->outsize 
	   , iob->outsize - iob->outlen
	   
	   );
  
  if(iob->outlen+ size >= iob->outsize)
    {
      //ciob = iob;
      item = new_iobuf_item(iob->outlen+size);
      in->ouitem = item;
      in->oubuf = item->data;
      iob = item->data;
      iob->outptr = 0;
      iob->outlen = 0;
      //push_ciob(NULL, ciob, iob);
      //item=new_list(iob);
      push_list(&in->oubuf_list, item);
      
    }
  
  if(g_debug)
    printf("%s starting output\n",__FUNCTION__);
  
  va_start(args, fmt);
  size = vsnprintf(&iob->outbuf[iob->outlen], iob->outsize-iob->outlen, fmt, args);
  va_end(args);
  iob->outlen+= size;
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
OK Watch files they terminate with a single 0xa

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
      if ((*sp == 0xa) || (*sp == 0xd))
	{
	  //if(lend > 1)
	  //{
	  //  found++;
	  //  if ((sp[1] == 0xa) ||(sp[1] == 0xd))
	  //	{
	  //	  sp++;
	  //	  lend--;
	  //	}
	  //}
	found++;
	}
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
