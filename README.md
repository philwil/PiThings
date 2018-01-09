# PiThings

This project the result of many years trying to find a novel way to let things talk to each other.

You set up a server which can talk to other servers on internet connected things
When you connect to the server you can get the value of things that the server knows about.
At the start the server its list of things is blank
All can be restored from a database or one or more files
.
When you connect you "ADD" things to the server.
You can use telnet to connect to the default port (5432) and then issue the following command

THe main function will also connect to the server allowing command line operation.

Some typical commands include :-

ADD system/status/cpu/temp
=> OK ADDED system/status/cpu/temp
=> ?? FOUND system/status/cpu/temp

The value of this item is set up with the SET command

SET system/status/cpu/temp 72.34
=> SET system/status/cpu/temp 72.34

The value can be retrieved

GET system/status/cpu/temp
=> GET system/status/cpu/temp 72.34

or in short form

GETS system/status/cpu/temp
=>72.34

You can search for a data base items

FIND system/status/cpu/temp
=> FIND system/status/cpu/temp 112 72.34

IDS system/status/cpu/temp
=>112


FIND system/status/cpu/temp2
=>?? FIND system/status/cpu/temp2

You can get a list of items in the database

FIND system/status/  ( note the trailing slash ) 
=>system/status/cpu/temp
=>system/status/disk1/used
=>system/status/disk1/mount
=>system/status/disk1/avail

The database tree can be saved to a file

SAVE system/   /var/pithings/system.txt

The database tree can be loaded from a file

LOAD system/   /var/pithings/system.txt

So much for local operation

Consider this command

FIND /some_node/system/status/  ( note the leading and trailing slashes ) 

This will attempt to use the find command on a different node "some_node"
THe system will attempt to set up a connection to some_node and then run the

FIND system/status command on that node


Nodes can be set up as follows

NODE /10.1.2.3:5432/ /some_node/

(use NODES) to list nodes 
The NODE command will also set up and maintain  a link to the remote node 


Database interface ( to do ) using names or ids

DBSAVE system/disk1/space  system/cpu/temp /db_name/
DBSAVE 123  128 /db_name/

DBLOAD system/disk1/space  system/cpu/temp /db_name/
DBLOAD 123  128 /db_name/

DBLIST system/disk1/space  system/cpu/temp /db_name/ /start_date:num//end_date:num/
DBLIST 123  128 /db_name/ /start/date/[num/] /end/date/[num/]

Actions can be defined for value sets and gets


SET_ACT  system/led/led1  /act_file_name_on_set/ [/action file data/]
GET_ACT  system/led/led1  /act_file_name_on_get/ [/action file data/]

These action files are executed with the node, variable and value as arguments

The SET_ACT is executed after setting the value.
The GET_ACT is executed before getting the value.


Typical worked example

We have 3 room temperature sensors and one heating controller with three zones.
Each room runs a different thing to measure the temp
EAch room will measure the temp and send it to the master

One each room at set up

ADD sensors/temp1

And set up the master node

NODE /10.1.2.3:5432/ /master/


/room2/sensors/temp1
/room3/sensors/temp1

Then measure the temp every few minutes
SET sensors/temp1 <value>

ALso send the temp to the master controller
SET /master/room1/sensors/temp1 <value>

Note this handy feature
COPY /master/room1 sensors

On the Master system we can run pseudo code as follows
for each room

if (GETS [room1/]sensor/temp1 < GETS [room1/]sensor/low_temp)
   SET [room1/]heater/mode heat
   SET [room1/]heater/mode_led red


else if (GETS [room1/]sensor/temp1 > GETS [room1/]sensor/hi_temp)
   SET [room1/]heater/mode cool
   SET [room1/]heater/mode_led blue
   

else
   SET [room1/]heater/mode off
   SET [room1/]heater/mode_led green

DBSAVE [room1/]sensor/temp1 [room1/]heater/mode [room1/]heater/mode_led /room1_temps/

Status

Story so far
   Many things work but curently testing the flexible, resusable  io buffers to hold io data. Once thee work we will be able to send lists ot things to connections
   
  Next feature will be to add remote nodes
  

   