#include "Daemon.hpp"

Daemon::Daemon(void) : logger() {
	MyError		err;

	if ((lock_fd = open(LOCK_FILE,
					O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
		logger.error((err = MyError("Can't open lock file")));
		throw (err);
	}
	if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
		logger.error((err = MyError("Another instance is already running")));
		throw (err);
	}
}

Daemon	&Daemon::operator=(const Daemon &daemon) {
	logger = daemon.logger;
	lock_fd = daemon.lock_fd;
	return (*this);
}

Daemon::~Daemon(void) {
	close(lock_fd);
	std::remove(LOCK_FILE);
}

void	Daemon::detach(void) {
	sigset_t	sig_mask;
	MyError		err;

	logger.info("Entering Daemon mode");
	switch (fork()) {
		case -1:
			logger.error((err = MyError("Can't daemonize process")));
			throw (err);
		case 0:
			setsid();
			sigfillset(&sig_mask);
			sigprocmask(SIG_SETMASK, &sig_mask, NULL);
			logger.info("started", getpid());
			return;
		default:
			usleep(MS_2_WAIT);
			exit(EXIT_SUCCESS);
	}
}

void	Daemon::run(void) {
	int				reuse = 1;
	sockaddr_in		serv_addr;
	MyError			err;
	struct pollfd	poll_fd[1];
	char			buff[BUFF_SZ];
	int64_t			read_sz;

	if ((serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return (logger.error((err = MyError("Can't create socket"))));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(DAEMON_PORT);
	inet_aton(LOCALHOST, (struct in_addr *)&serv_addr.sin_addr.s_addr);
	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse))) {
		close(serv_sock);
		return (logger.error(err = MyError("Can't set socket options")));
	}
	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
		close(serv_sock);
		return (logger.error(err = MyError("Can't bind socket")));
	}
	if (listen(serv_sock, CLIENT_NB)) {
		close(serv_sock);
		return (logger.error(err = MyError("Can't listen on port")));
	}
	poll_fd[0].fd = serv_sock;
	poll_fd[0].events = POLLIN;
	while (42) {
		if (poll(poll_fd, FD_NB, TIMEOUT)) {
			if ((client_sock = accept(serv_sock, NULL, NULL)) < 0) {
				close(serv_sock);
				return (logger.error(err = MyError("Can't accept client connection")));
			}
			while ((read_sz = read(client_sock, buff, sizeof(buff))) > 0) {
				buff[read_sz] = '\0';
				if (!strncmp(buff, "quit", 4))
					return (request_quit());
				logger.log(buff);
				bzero(buff, sizeof(buff));
			}
		}
	}
}

void	Daemon::request_quit(void) {
	logger.info("Request quit");
	close(client_sock);
}
