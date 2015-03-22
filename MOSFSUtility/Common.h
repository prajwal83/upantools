#ifndef _COMMON_H_
#define _COMMON_H_

typedef unsigned char byte ;

typedef enum
{
	TRUE = 1,
	FALSE = 0
} boolean ;

#define EXTERNAL_ERROR 100

#define PACKED __attribute__ ((packed))

#define RETURN_IF_NOT(RetVal, Func, CheckVal) \
RetVal = Func ;\
if(RetVal != CheckVal) \
return RetVal ;

#define LSEEK(fd, off, attr) lseek(fd, (uiLBAStartSector * 512) + off, attr)

char errBuf[512] ;

#define PUT_ERROR_MSG(str) sprintf(errBuf, "%s %s - %d ", str, __FILE__, __LINE__) ;\
perror(errBuf) ;

int fd_DiskImageFile ;
char* DiskImageFileName ;
unsigned uiLBAStartSector ;

#endif
