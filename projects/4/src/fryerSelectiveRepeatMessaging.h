#ifndef FRYER_P4_MESSAGING_H
#define FRYER_P4_MESSAGING_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>

void redStdout(const char *msg);

void greenStdout(const char *msg);

void yellowStdout(const char *msg);

void blueStdout(const char *msg);

void magentaStdout(const char *msg);

void cyanStdout(const char *msg);

int usage(char *arg1, char *arg2);

void handleFatalErrorNo(int en, const char *msg, int sockfd);

int handleErrorNoRet(int en, int retval, const char *msg);

void handleErrorNoMsg(int en, const char *msg);

void handleFatalError(const char *msg, int sockfd);

int handleErrorRet(int retval, const char *msg);

void handleErrorMsg(const char *msg);


#endif
