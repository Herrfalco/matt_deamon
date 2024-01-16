#include "Daemon.hpp"

Daemon::Daemon(void) : logger(), serv_sock(0), epoll(0), clients() {
	if ((lock_fd = open(LOCK_FILE,
					O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
		logger.error(MyError("Can't open lock file"));
	if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
		close(lock_fd);
		logger.error(MyError("Another instance is already running"));
	}
}

Daemon	&Daemon::operator=(const Daemon &daemon) {
	logger = daemon.logger;
	lock_fd = daemon.lock_fd;
	serv_sock = daemon.serv_sock;
	epoll = daemon.epoll;
	clients = daemon.clients;
	return (*this);
}

Daemon::~Daemon(void) {
	if (serv_sock > 0)
		close(serv_sock);
	if (epoll > 0)
		close(epoll);
	for (std::list<int>::iterator it = clients.begin();
			it != clients.end(); ++it)
		close(*it);
	std::remove(LOCK_FILE);
	flock(lock_fd, LOCK_UN);
	close(lock_fd);
}

void	Daemon::detach(void) {
	sigset_t	sig_mask;

	logger.info("Entering Daemon mode");
	switch (fork()) {
		case -1:
			logger.error(MyError("Can't daemonize process"));
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

int		Daemon::event_loop(int event_nb, struct epoll_event *events) {
	int					i, cli, flags;
	ssize_t				r_ret;
    char				buff[BUFF_SZ + 1] = { 0 };
    struct epoll_event	event { .events = EPOLLIN };

	for (i = 0; i < event_nb; ++i) {
		if (events[i].data.fd == serv_sock) {
			if (clients.size() >= CLIENT_NB)
				continue;
			if ((cli = accept(serv_sock, NULL, NULL)) < 0)
				throw (MyError("Can't accept new connection"));
			clients.push_back(cli);
			if ((flags = fcntl(cli, F_GETFL, 0)) < 0
					|| fcntl(cli, F_SETFL, flags | O_NONBLOCK) < 0)
				throw (MyError("Can't configure new client socket"));
			event.data.fd = cli;
			if (epoll_ctl(epoll, EPOLL_CTL_ADD, cli, &event))
				throw (MyError("Can't add client socket to epoll instance"));
		} else if ((r_ret = read(events[i].data.fd, buff, BUFF_SZ)) > 0) {
			buff[r_ret] = '\0';
			// ptit souci si depassement de buffer (plusieurs ligne de logs
			// et detection de quit dans le log suivant... acceptable ?)
			if (!strcmp(buff, "quit"))
				return (0);
			else
				logger.info(buff);
		} else {
			close(events[i].data.fd);
			clients.remove(events[i].data.fd);
			if (epoll_ctl(epoll, EPOLL_CTL_DEL, events[i].data.fd, NULL))
				throw (MyError("Can't delete client socket from epoll instance"));
		}
	}
	return (1);
}

void	Daemon::serv_loop(void) {
	int					event_nb, quit = 0;
    struct epoll_event	event {
		.events = EPOLLIN,
		.data = { .fd = serv_sock },
	}, events[CLIENT_NB + 1];

    if ((epoll = epoll_create1(0)) < 0)
		throw (MyError("Can't create epoll instance"));
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, serv_sock, &event))
		throw (MyError("Can't add server socket to epoll instance"));
	do
		if ((event_nb = epoll_wait(epoll, events, CLIENT_NB + 1, -1)) < 0)
			throw (MyError("Can't get pending events"));
	while (event_loop(event_nb, events));
	logger.info("Quitting");
}

void	Daemon::run(void) {
	int				reuse = 1;
	sockaddr_in		serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(DAEMON_PORT),
		.sin_addr = { .s_addr = INADDR_ANY },
	};

	if ((serv_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK,
					IPPROTO_TCP)) < 0)
		logger.error(MyError("Can't create socket"));
	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)))
		logger.error(MyError("Can't set socket options"));
	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
		logger.error(MyError("Can't bind socket"));
	if (listen(serv_sock, MAX_QUEUE))
		logger.error(MyError("Can't listen on port"));
	serv_loop();
}
