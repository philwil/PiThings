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
extern struct cmds h_cmds[];
extern struct space *g_space;
extern struct space *g_spaces[];
extern int g_space_idx;


int test_find_parents(void)
{
  int num=0;
  int rc=0;
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
  //struct space *root = NULL;
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
  //int i;
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
  //int i;
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
  return space;
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
  //struct space *start=base;
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
  //struct space *xstart=NULL;
  while (start)
    {
      rc = show_space_new(in, start, desc, len, bdesc);
      if(g_debug)
	printf(" >> [%s]\n", desc);
      if(in)in_snprintf(in, NULL, ">>[%s]\n", desc);
      //xstart = start->next;
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
  //char *sp = name;
  char *spv = NULL;
  int i;

  //int rc;
  int idx = 0;
  int idv = 0;
  char *valv[64];
  char *valx[64];
  parse_name(&idx, (char **)valx, &idv, (char **)valv, 64, name);
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
struct space *show_space_in(struct space **base, char *name, struct iosock *in)
{
  //int rc = 0;
  char sbuf[4096];
  struct space *sp1=NULL;
  struct space **spb=&g_space;
  char *sp = name;
  sscanf(name,"%s ", sbuf);  // TODO use more secure option
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
  show_spaces_new(in, spb, sbuf, 4096, sbuf);
  return NULL;
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
  //int rc = -1;
  struct space *sp1;
  struct space *base =  *basep;
  char sname[3][128];  // TODO
  char * spv;

  sname[0][0]=0;
  sname[1][0]=0;
  sname[2][0]=0;
  sscanf(name,"%s %s %s", sname[0], sname[1], sname[2]);
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
      //rc = 0;
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
