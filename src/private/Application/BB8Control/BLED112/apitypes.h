
#ifndef APITYPES_H_
#define APITYPES_H_
#define BD_ADDR_LENGTH 6
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef signed short   int16;
typedef unsigned long  uint32;
typedef signed char    int8;

typedef struct bd_addr_t
{
	union{
    uint8 addr[BD_ADDR_LENGTH];
	unsigned long laddr;
	};

}bd_addr;

typedef bd_addr hwaddr;
typedef struct
{
    uint8 len;
    uint8 data[];
}uint8array;



#endif
