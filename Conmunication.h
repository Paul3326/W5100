#ifndef _Communication_H
#define _Communication_H

extern OS_STK Conmunication_osTask[];
extern void Conmunication_Task(void *p_arg);


typedef struct _pack_information_{
    char name;
    char len;
}PACK_INF;

PACK_INF ZipPack;

#endif