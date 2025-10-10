#ifndef WRAPPER_H   //if header.h is not define 
#define WRAPPER_H    //then define the header.h .it is applied to prevent multiple inclusion 
#include <stdio.h>

void file_close(FILE** fptr ,int debug)
{
    if (fptr !=NULL && *fptr !=NULL) {
        fclose(*fptr);
        *fptr = NULL;
    }
}
#endif