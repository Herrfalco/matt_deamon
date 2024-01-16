#ifndef INCLUDES_H
#define INCLUDES_H

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <poll.h>
#include <pthread.h>

#define BUFF_SZ			256
#define	CLIENT_NB		3
#define FD_NB			1
#define TIMEOUT			0
#define LOCALHOST		"127.0.0.1"
#define	DAEMON_PORT		4242
#define LOCK_FILE		"/var/lock/matt_daemon.lock"
#define MS_2_WAIT		100000

#endif // INCLUDES_H
