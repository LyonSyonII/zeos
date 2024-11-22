#ifndef DEVICES_H__
#define  DEVICES_H__

#include "list.h"
int sys_write_console(char *buffer,int size);

extern struct list_head blocked;
#endif /* DEVICES_H__*/
