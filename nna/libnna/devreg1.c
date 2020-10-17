/* This file has been generated by the Hex-Rays decompiler.
   Copyright (c) 2007-2017 Hex-Rays <info@hex-rays.com>

   Detected compiler: GNU C++
*/

#include <defs.h>


//-------------------------------------------------------------------------
// Function declarations

void *__fastcall xreg_open(unsigned int a1);
int xreg_close(void);
int __fastcall xregr(unsigned int a1);
int __fastcall xdummy_regw(unsigned int a1, unsigned int a2);
int __fastcall xregw(int result, unsigned int a2);
int __fastcall xreg_wait(int result, unsigned int a2, unsigned int a3);
// int open(const char *file, int oflag, ...);
// int printf(const char *format, ...);
// void *mmap(void *addr, size_t len, int prot, int flags, int fd, __off_t offset);
// int munmap(void *addr, size_t len);
// int *_errno_location(void);
// char *strerror(int errnum);
// int fprintf(FILE *stream, const char *format, ...);
// void exit(int status);
// int close(int fd);

//-------------------------------------------------------------------------
// Data declarations

int reg_base; // weak
int dev_fd; // weak
// extern struct _IO_FILE *stderr;


//----- (00000004) --------------------------------------------------------
void *__fastcall xreg_open(unsigned int a1)
{
  unsigned int offset; // r4
  int fd; // r0
  void *result; // r0

  offset = a1;
  fd = open("/dev/mem", 2050);
  dev_fd = fd;
  if ( fd < 0 )
    return (void *)printf("open(/dev/mem) failed.[%x]", offset);
  result = mmap(0, 0x20000u, 3, 1, fd, offset);
  reg_base = (int)result;
  return result;
}
// 144: using guessed type int reg_base;
// 1DC: using guessed type int dev_fd;

//----- (00000058) --------------------------------------------------------
int xreg_close(void)
{
  FILE *v0; // r5
  int *v1; // r0
  int v2; // r4
  char *v3; // r0
  int result; // r0

  if ( munmap((void *)reg_base, 0x20000u) == -1 )
  {
    v0 = (FILE *)stderr;
    v1 = _errno_location();
    v2 = *v1;
    v3 = strerror(*v1);
    fprintf(v0, "Error at line %d, file %s (%d) [%s]\n", 48, "devreg.cpp", v2, v3);
    exit(1);
  }
  result = dev_fd;
  if ( dev_fd )
    result = close(dev_fd);
  return result;
}
// 144: using guessed type int reg_base;
// 1DC: using guessed type int dev_fd;

//----- (000000C8) --------------------------------------------------------
int __fastcall xregr(unsigned int a1)
{
  return *(_DWORD *)(reg_base + a1);
}
// 144: using guessed type int reg_base;

//----- (000000D8) --------------------------------------------------------
int __fastcall xdummy_regw(unsigned int a1, unsigned int a2)
{
  return printf("%04X,%08X\n", a1, a2);
}

//----- (000000EC) --------------------------------------------------------
int __fastcall xregw(int result, unsigned int a2)
{
  *(_DWORD *)(reg_base + result) = a2;
  return result;
}
// 144: using guessed type int reg_base;

//----- (000000FC) --------------------------------------------------------
int __fastcall xreg_wait(int result, unsigned int a2, unsigned int a3)
{
  _DWORD *v3; // r7
  int v4; // r4

  v3 = (_DWORD *)(reg_base + result);
  v4 = 0;
  while ( 1 )
  {
    ++v4;
    if ( (*v3 & a2) == a3 )
      break;
    if ( v4 == 899 )
      return printf("\n# wait time out: %02X:%02X v:%02X loop:%d\n", result, *v3, a3, 899);
  }
  return result;
}
// 144: using guessed type int reg_base;

// ALL OK, 6 function(s) have been successfully decompiled