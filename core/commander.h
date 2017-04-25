//
//  commander.h
//  pod
//
//  Created by Eddie Hurtig on 2/25/17.
//
//

#ifndef commander_h
#define commander_h

#include "commands.h"
#include "pod.h"

typedef struct commander {
  char cmdbuffer[CMD_OUT_BUF];
  bool first_client;
  int serverfd;
  int clients[MAX_CMD_CLIENTS];
  int nclients;
} commander_t;


int cmd_start_tcp_server(int portno);
int cmd_do_command(size_t inputc, char *input, size_t outputc, char output[]);
int cmd_process_client(int fd);
int cmd_reject_client(int fd);
int cmd_accept_client(int fd);
int cmd_process_request(int infd, int outfd, commander_t* commander);
int cmd_server(void);
void *command_main(__unused void *arg);

#endif /* commander_h */
