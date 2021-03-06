/* from
http://abyz.me.uk/rpi/pigpio/ex_motor_shield.html
cc -o pimotor pimotor.c -lpigpio -lpthread -lrt
*/

/*
 for now forget the motor stuff
 we aim to build up a repository of things
 a thing has a name, 
 belongs to a class 
 has a type
 has an id
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
get foo/boo/fum
set foo/boo/fum value
onget foo/boo/fum <function>
onset foo/boo/fum <function>

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

#define NUM_THINGS 1024
#define NUM_SPACES 32
#define CLASS_VAR 1
#define CLASS_CLASS 2
#define CLASS_TYPE 3

#define SPACE_ROOT 0
#define SPACE_FLOAT 1
#define SPACE_INT 2
#define SPACE_STR 3

struct thing {
  char name[64];
  char desc[64];
  int idx;
  int id;
  int type;
  int class;
  int last_idx;
  int next_idx;

  char str[64];
  int ival;
  double fval;
};

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
  // we'll get rid of this stuff soon
  //struct thing *things;
  //int things_max;
  //int things_used;
  //struct space **clones;
  //int clones_max;
  //int clones_used;


};

#define NUM_IDX 1024
struct space *space=NULL;
struct space *g_space=NULL;
int g_space_idx = 1;
struct space *g_spaces[NUM_IDX];

struct thing things[NUM_THINGS];
struct thing t_types[NUM_THINGS];


char *get_space(struct space * base, char *name, char *buf, int len);
int set_space(struct space * base, char *name, char *value, char *buf, int len);
struct space *make_space(struct space **root, char *name, char *buf, int len);

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

int show_spaces(struct space *base, char *desc, int indent, char *buf, int len);

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



int show_space_new(struct space *base, char *desc, int len, char *buf)
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
      printf(" >> [%s]\n", buf);
      return 0;
    }
  // foreach child do the same
  slen -= strlen(sp);
  sp += strlen(sp);
  while (child != NULL)
    {

      show_space_new(child, sp, slen, buf);
      child = child->next;
      if (child == base->child) 
	child= NULL;
    }
  ret =  strlen(buf);
  return ret;
}

int show_spaces_new(struct space *base, char *desc, int len, char *buf)
{
  int rc  = -1;
  struct space *start=base;
  struct space *xstart=NULL;
  while (start)
    {
      show_space_new(start, desc, len, buf);
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
  char vals[64][64];
  int idx = 0;
  int idv = 0;
  char *sp = name;
  int rc;
  int i;
  rc = 1;

  while(*sp && (rc>0) && (idx < 64))
    {
      rc = 0;
      idv = 0;
      while (*sp && *sp != '/')
	{
	  vals[idx][idv] = *sp;
	  rc++;
	  sp++;
	  if(idv < sizeof(vals[idx]-1))
	    {
	      idv++;
	    }
	}
      if(*sp)sp++;

	
      if(rc>0)
	{
	  vals[idx][idv] = 0;
	  if(0)printf("rc %d val[%d] [%s] ", rc, idx, vals[idx]);
	  if(0)printf("sp [%s] \n", sp);
	  idx++;
	}
    }
  // now find the space at each step
  i = 0;
  start = base;
  while (base)
    {
      if(0)printf(" looking for [%s] found [%s] i %d idx %d\n"
		  , vals[i], base->name
		  , i
		  , idx
		  );
      if(strcmp(base->name, vals[i])==0)
	{

          if(i < idx) i++;
	  if(i == idx)
	    return base;
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
	      if(i < idx) i++;
	      if(i == idx)
		return NULL;
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

// main
int init_things(void)
{
  int i;
  struct thing *item;
  item = &things[0];

  for (i=0; i< NUM_THINGS; i++)
    {
      item->idx = i;
      item->name[0] = 0;
      item++;
    }  
  item = &t_types[0];

  for (i=0; i< NUM_THINGS; i++)
    {
      item->idx = i;
      item->name[0] = 0;
      item++;
    }  
  return 0;
}
//     name  class   type  value
// ADD foo   data    float 2.3456

struct thing *new_thing(struct space *base, char *name, int class, int type, int y)
{
  int i;
  struct thing *item;
  item = &things[0];
  //  if(base)
  //item = base->things;

  for (i=0; i< NUM_THINGS; i++)
    {
      if(item->name[0] == 0)
	{
	  strncpy(item->name,name,64);
	  item->class = class;
	  item->type = type;
	  break;
	}
      item++;
    }
  if(i ==  NUM_THINGS)
    item = NULL;
  printf("new_thing name[%s] item %p\n"
	 , name
	 , item
	 );

  return item;

}

struct thing *get_thing(struct space *base, char *name, int class, int type, int y)
{

  int i;
  struct thing *item;
  item = &t_types[0];
  //  if(base)
  //  item = base->things;
  printf("get_thing 1 name[%s] class %d base%p\n", name, class, base);


  for (i=0; i< NUM_THINGS; i++)
    {
      if((item->class == class ) && (strcmp(item->name,name) == 0))
	{
	  break;
	}
      item++;
    }  
  printf("get_thing 2 name[%s] class %d base%p\n", name, class, base);

  if(i ==  NUM_THINGS)
    item = new_thing(base, name, class, type, y);
  printf("get_thing name[%s] class %d item %p\n"
	 , name
	 , class
	 , item
	 );

  return item;
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

// split up multi/space/name
// look for children of the same name 
// return found name or new space object
struct space *make_space(struct space **root, char *name, char *buf, int len)
{
  struct space *parent=NULL;
  struct space *space=NULL;
  char sps[3][64];
  char vals[64][64];
  int idx = 0;
  int idv = 0;
  int cidx = 0;
  char *sp = name;
  int rc;
  int i;
  rc = 1;

  sps[0][0] = 0;
  sps[1][0] = 0;

  rc = sscanf(name,"%s %s",sps[0], sps[1]);
  if(rc == 1)sp = sps[0];
  if(rc == 2)sp = sps[1];

  while(*sp && (rc>0) && (idx < 64))
    {
      rc = 0;
      idv = 0;
      while (*sp && *sp != '/')
	{
	  vals[idx][idv] = *sp;
	  rc++;
	  sp++;
	  if(idv < sizeof(vals[idx]-1))
	    {
	      idv++;
	    }
	}
      if(*sp)sp++;

	
      if(rc>0)
	{
	  vals[idx][idv] = 0;
	  printf("rc %d val[%d] [%s] ", rc, idx, vals[idx]);
	  printf("sp [%s] \n", sp);
	  idx++;
	}
    }
  for (i = 0 ; i < idx; i++)
    {
      space = NULL;
      if(parent)
	{
	  space = find_space(&parent->child, vals[i]);
	}
      else
	{
	  space = find_space(root, vals[i]);
	}
      if (!space)
	{
	  if(parent)
	    {
	      printf(" New Space for [%s] parent->name [%s]\n", vals[i], parent->name);
	      space = new_space(vals[i], parent->child, &parent->child, NULL); 
	      add_child(parent,space);

	    }
	  else
	    {
	      printf(" New Space for [%s] at root\n", vals[i]);
	      space = new_space(vals[i], NULL, &g_space, NULL); 
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
	  printf(" Space found [%s]\n", vals[i]);
	}

      parent = space;
    }
    
  printf("[%s] found %d spaces\n",name, idx);

  return space;
}


//           attr        space  class type  value 
//   run_str("ADD item foo in Space1/multi/option data  float 2.3456");
int run_str_add(char *stuff, char *buf, int len)
{
  char cmd[128];
  snprintf(cmd, sizeof(cmd),"%s",stuff);
  char vals[64][64];
  int idx = 0;
  int cidx = 0;
  char *sp = cmd;
  int rc;
  struct space *space=NULL;
  struct space *class=NULL;
  struct space *attr=NULL;


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
  
  if(
     (strcmp(vals[cidx+1], "item") == 0) &&
     (cidx + 3) < idx)
    
    {
      printf(" Running the add_item [%s] \n", vals[cidx+2]);
      if (
	  (strcmp(vals[cidx+3], "in") == 0) &&
	  ((cidx + 4) < idx)
	  )
	{
	  space = find_space(NULL, vals[cidx+4]);
	}
      if(!space)
	{
	  printf(" Adding space [%s]\n", vals[cidx+4]);
	  space = new_space(vals[cidx+4], NULL, &g_space, NULL);
	}
      if(space)
	{
	  printf("found space [%s] \n", space->name);
	  class=find_space(&space->class, vals[cidx+5]);
	  if(class)
	    {
	      printf("   found class [%s] \n", class->name);
	    }
	  else
	    {
	      printf("   adding class  [%s] to [%s]\n"
		     , vals[cidx+5]
		     , vals[cidx+4]
		     );
	      //class =  new_space_attr_float(vals[cidx+2], space, atof(vals[cidx+7]));
	      class =  new_space_class(vals[cidx+5], space);

	    }
	  if(!class) class =  space;
	  attr=find_space(&class->attr, vals[cidx+2]);
	  if(attr)
	    {
	      printf("   found attr [%s] \n", attr->name);
	    }
	  else
	    {
	      printf("   adding attr  [%s] type [%s]\n"
		     , vals[cidx+2]
		     , vals[cidx+6]
		     );
	      if(strcmp(vals[cidx+6],"float") ==0)
		{
		  new_space_attr_float(vals[cidx+2], class, atof(vals[cidx+7]));
		}
	      else if(strcmp(vals[cidx+6],"int") ==0)
		{
		  new_space_attr_int(vals[cidx+2], class, atoi(vals[cidx+7]));
		}
	      else
		{
		  new_space_attr_str(vals[cidx+2], class, vals[cidx+7]);
		}

	    }
	}  
      if((cidx+5) < idx)
	  printf("using class [%s] \n", vals[cidx+5]);
      if((cidx+6) < idx)
	  printf("using type [%s] \n", vals[cidx+6]);
      if((cidx+7) < idx)
	  printf("using value [%s] \n", vals[cidx+7]);

      // cidx+4 is class
      // cidx+5 is type
    
    }

  return idx;
}
//           attr        space  class type  value 
//   run_str("SET item foo in Space1 data  value 2.3456");
int run_str_set(char *stuff, char *buf, int len)
{
  char cmd[128];
  snprintf(cmd, sizeof(cmd),"%s",stuff);
  char vals[64][64];
  int idx = 0;
  int cidx = 0;
  char *sp = cmd;
  struct space *space=NULL;
  struct space *attr=NULL;
  struct space *class=NULL;
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
  
  if(
     (strcmp(vals[cidx+1], "item") == 0) &&
     (cidx + 3) < idx)
    
    {
      printf(" Running the set_item [%s] \n", vals[cidx+2]);
      if (
	  (strcmp(vals[cidx+3], "in") == 0) &&
	  ((cidx + 4) < idx)
	  )
	{
	  space = find_space(NULL, vals[cidx+4]);
	}
      if(!space)
	{
	  printf(" No space [%s]\n", vals[cidx+4]);
          goto out;
	}
      if(space)
	{
	  printf("found space [%s] \n", space->name);

	  class=find_space(&space->class, vals[cidx+5]);
	  if(!class)
	    {
	      printf(" No space [%s]\n", vals[cidx+5]);
	      goto out;
	    }
	  printf("found class [%s] \n", class->name);
	  attr=find_space(&class->attr, vals[cidx+2]);
	  if(attr)
	    {
	      printf("   found attr [%s] \n", attr->name);
	    }
	  else
	    {
	      printf("   No attr [%s] \n", attr->name);
	      goto out;
	    }
	  if(attr->type == SPACE_FLOAT)
	    attr->fval = atof(vals[cidx+6]);
	  else if(attr->type == SPACE_INT)
	    attr->ival = atoi(vals[cidx+6]);
	  else
	    attr->cval = vals[cidx+6];
	}
      if((cidx+6) < idx)
	printf("using value [%s] \n", vals[cidx+6]);
      
      // cidx+4 is class
      // cidx+5 is type
      
    }
 out:
  return rc;
}
//           attr        space  class type  value 
//   run_str("GET item foo in Space1 data  value");
int run_str_get(char *stuff, char *buf, int len)
{
  char cmd[128];
  snprintf(cmd, sizeof(cmd),"%s",stuff);
  char vals[64][64];
  int idx = 0;
  int cidx = 0;
  char *sp = cmd;
  struct space *space=NULL;
  struct space *attr=NULL;
  struct space *class=NULL;
  int rc=-1;
  if (!buf) return rc;
  buf[0]=0;

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
  
  if(
     (strcmp(vals[cidx+1], "item") == 0) &&
     (cidx + 3) < idx)
    
    {
      printf(" Running the get_item [%s] \n", vals[cidx+2]);
      if (
	  (strcmp(vals[cidx+3], "in") == 0) &&
	  ((cidx + 4) < idx)
	  )
	{
	  space = find_space(NULL, vals[cidx+4]);
	}
      if(!space)
	{
	  printf(" No space [%s]\n", vals[cidx+4]);
          goto out;
	}
      if(space)
	{
	  printf("found space [%s] \n", space->name);

	  class=find_space(&space->class, vals[cidx+5]);
	  if(!class)
	    {
	      printf(" No space [%s]\n", vals[cidx+5]);
	      goto out;
	    }
	  printf("found class [%s] \n", class->name);
	  attr=find_space(&class->attr, vals[cidx+2]);
	  if(attr)
	    {
	      printf("   found attr [%s] type %d\n"
		     , attr->name
		     , attr->type
		     );
	    }
	  else
	    {
	      printf("   No attr [%s] \n", attr->name);
	      goto out;
	    }
	  if(attr->type == SPACE_FLOAT)
	    {
	      snprintf(buf, len, "%f", attr->fval);
	    }
	  else if(attr->type == SPACE_INT)
	    {
	      snprintf(buf, len, "%d", attr->ival);
	      printf("   int buf set  [%s] \n", buf);

	    }

	  //attr->ival = atoi(vals[cidx+6]);
	  else
	    {
	      snprintf(buf, len, "%s", attr->cval);
	    }
	    printf("   buf set  [%s] \n", buf);

	  //attr->cval = vals[cidx+6];
	}
      rc =  strlen(buf);
      // cidx+4 is class
      // cidx+5 is type
      
    }
 out:
  return rc;
}

//           attr        space  class type  value 
//   run_str("ADD item foo in Space1 data  float 2.3456");
int run_str(char *stuff, char *buf, int len)
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
  while((rc>0) && (idx < 1))
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
      make_space(&g_space, stuff, buf, len);
      return 0;
    }
  //return run_str_add(stuff,buf,len);
  else if(strcmp(vals[cidx], "SET") == 0)
    {
      rc = set_space(g_space, stuff, NULL, buf, len);
      return rc ; 
    }
  else if(strcmp(vals[cidx], "GET") == 0)
    {
      rc = 0;
      sp = get_space(g_space, stuff, buf, len);
      if(buf)
	{
	  snprintf(buf,len,"%s",sp);
	  rc = strlen(buf);
	}
      return rc;
    }

  //   return run_str_get(stuff, buf, len);
  return 0;
}


// ADD foo data float 2.3456
// add_thing("ADD foo data float 2.3456")

int add_thing(char *stuff)
{
  int rc;
  char cmd[64];
  char v1[64];
  char v2[64];
  char v3[64];
  char v4[64];
  char v5[64];
  struct space *space=NULL;
  struct thing *item_1;
  struct thing *item_2;
  struct thing *item_3;

  rc = sscanf(stuff, "%s %s %s %s %s %s"
	      , cmd
	      , v1
	      , v2
	      , v3
	      , v4
	      , v5
	      );
  printf(" cmd = [%s] v1=[%s] rc = %d\n", cmd, v1, rc );
  item_1 = get_thing(space, v1, CLASS_VAR ,0,0);
  item_2 = get_thing(space, v2, CLASS_CLASS,0,0);
  item_3 = get_thing(NULL/*&t_types[0]*/,v3, CLASS_TYPE,0,0);
  item_1->class = item_2->idx;
  item_1->type = item_3->idx;

  return 0;
}


int show_things(struct thing *base)
{
  int i;
  struct thing *item;
  item = &things[0];
  if(base)
    item = base;
  for (i=0; i< NUM_THINGS; i++)
    {
      if(item->name[0] != 0)
	{
	  printf("idx %d item name [%s] class %d type %d \n"
		 , item->idx
		 , item->name
		 , item->class
		 , item->type
		 );
	}
      item++;
    }  
  return 0;
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

#define NUM_SOCKS 16
#define NUM_CMDS 16
#define INSIZE 1024
#define OUTSIZE 1024

int num_socks=0;

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
};

struct cmds
{
  char *key;
  char *cmd;
  int (*handler)(void *key, int n, char *cmd, int speed, int time);
};
  



struct insock insock[NUM_SOCKS];
struct cmds cmds[NUM_CMDS];

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
  }
  return i;
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
    int speed=0;
    int time=0;

    len = read(in->fd,&in->inbuf[in->inptr],in->insize-in->inptr);

    if(len > 0)
      {
	in->inlen += len;
	in->inbuf[in->inlen] = 0;
	sp = &in->inbuf[in->inptr];
	n = sscanf(sp,"%s %d %d", cmd, &speed, &time);
	rc = snprintf(&in->outbuf[in->outlen], in->outsize-in->outptr
		      ," message received [%s] ->"
		      " n %d cmd [%s] speed %d time %d\n"
		      , &in->inbuf[in->inptr]
		      , n, cmd , speed, time);
	in->outlen =+ rc;
	run_cmd (cmd, n, sp, speed, time);

	printf(" rc %d n %d cmd [%s] speed %d time %d\n"
	       , rc, n, cmd , speed, time);
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
    
    fds[idx].fd = lsock;
    fds[idx].events = POLLIN;
    fds[idx].revents = 0;
    idx++;
    
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
		if(fds[i].fd == lsock)
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

int set_space(struct space * base, char *name, char *value, char *buf, int len)
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
      rc = 0;
    }
  return rc;
}

char *get_space(struct space * base, char *name, char *buf, int len)
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
      sret = sp1->value;
    }
  return sret;
}

int count = 0;
int main (int argc, char *argv[])
{
   int i;
   int lsock;
   int rc = 1;
   int depth=0;
   //struct space * sp1 = new_space("Space1", struct space *parent, struct space** root, char *node)
   struct space *sp1;
   struct space *sp2;
   //struct space *sp3;
   char buf[2048];
   char * sp;
   init_g_spaces();

   sp1 = make_space(&g_space, "ADD uav1/motor1/speed", NULL, 0);
   show_spaces(g_space, "All Spaces 1 ", 0, NULL , 0);
   sp1 = make_space(&g_space, "ADD uav1/motor1/size", NULL, 0);
   show_spaces(g_space, "All Spaces 1 ", 0, NULL , 0);
   sp1 = make_space(&g_space, "ADD uav1/motor2/speed", NULL, 0);
   show_spaces(g_space, "All Spaces 2 ", 0, NULL , 0);
   sp1 = make_space(&g_space, "ADD uav2/motor2/speed", NULL, 0);
   show_spaces(g_space, "All Spaces 3 ", 0, NULL , 0);
   sp1 = make_space(&g_space, "uav3/motor2/speed", NULL, 0);
   show_spaces_new(g_space, buf, 2048, buf);
   sp1 = find_space_new(g_space, "uav1/motor3");
   printf(" found %s \n", sp1?sp1->name:"no uav1/motor3");
   sp1 = find_space_new(g_space, "uav1/motor1");
   printf(" found %s \n", sp1?sp1->name:"no uav1/motor1");
   sp2  = copy_space_new(sp1, "new_motor1");
   rc  = set_space(g_space, "SET uav3/motor2/speed 3500", NULL, NULL, 0);
   sp =  get_space(g_space,"GET uav3/motor2/speed", NULL,0);
   printf(" >> %s value [%s]\n","uav3/motor2/speed", sp?sp:"no value");

   show_spaces_new(sp2, buf, 2048, buf);

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

   //sp1 = make_space(&g_space, "uav2/motor2/speed", NULL, 0);
   run_str("ADD uavx/led1/on", buf, sizeof(buf));
   run_str("ADD uavx/led1/color", buf, sizeof(buf));
   run_str("ADD uavx/motor1/speed", buf, sizeof(buf));
   run_str("ADD uavx/motor1/size", buf, sizeof(buf));
   run_str("ADD uavx/motor2/speed", buf, sizeof(buf));
   run_str("ADD uavx/motor2/size", buf, sizeof(buf));
   //run_str("ADD item foo_float in Space1 data  float 2.3456", buf, sizeof(buf));
   //run_str("ADD item foo_int in Space4 data  int 2233", buf, sizeof(buf));
   //run_str("ADD item foo_float1 in Space4 data  float 1.233", buf, sizeof(buf));
   //run_str("ADD item foo_float2 in Space4 data  float 2.233", buf, sizeof(buf));
   //run_str("ADD item foo_str in Space4 data  str xxx2.233", buf, sizeof(buf));

   run_str("SET uavx/led1/on 1", buf, sizeof(buf));
   run_str("SET uavx/led1/color red", buf, sizeof(buf));

   //run_str("SET item foo_int in Space4 data value 2234", buf, sizeof(buf));
   rc = run_str("GET uavx/led1/color", buf, sizeof(buf));

   printf("GET rc %d buf [%s]\n",rc, buf);

#if 0
   show_spaces(g_space, "Global Spaces 2", 0, NULL , 0);
   rc = show_spaces(g_space, "Global Spaces Buf", 0, buf , sizeof(buf));
   printf("rc %d buf [%s]\n",rc, buf);
   return 0;

   run_str("ADD item foo  in Space1 data float 2.3456", buf, sizeof(buf));
   run_str("ADD item foo1 in Space1 data int 234", buf, sizeof(buf));
   init_things();
   add_thing("ADD item foo1 in Space1 data int 234");
   add_thing("ADD item  foo2 in Space1 data str \"val 234\"");

   show_things(NULL);
   show_things(&t_types[0]);

   init_cmds();
   init_cmd("FWD", fwd_cmd);
   run_cmd ("FWD", 2, "Some data", 100, 50);
#endif

   init_insocks();
   accept_socket(STDIN_FILENO);
   lsock = listen_socket(5432);
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
