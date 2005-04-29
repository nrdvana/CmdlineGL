#ifndef PROCESS_INPUT_H
#define PROCESS_INPUT_H

#define P_SUCCESS 0
#define P_CMD_ERR 1
#define P_NOT_FOUND 2
#define P_EOF 3

int ProcessFD(int fd);

#endif
