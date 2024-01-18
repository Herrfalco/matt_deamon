#include "Daemon.hpp"

Daemon::Daemon(void) : logger(), serv_sock(0), sig_fd(0), epoll(0), clients() {
	if ((lock_fd = open(LOCK_FILE,
					O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
		logger.error(MyError("Can't open lock file"));
	if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
		close(lock_fd);
		logger.error(MyError("Another process tried to create/open matt_daemon.lock"));
	}
}

Daemon	&Daemon::operator=(const Daemon &daemon) {
	logger = daemon.logger;
	lock_fd = daemon.lock_fd;
	serv_sock = daemon.serv_sock;
	sig_fd = daemon.sig_fd;
	epoll = daemon.epoll;
	clients = daemon.clients;
	return (*this);
}

Daemon::~Daemon(void) {
	if (serv_sock > 0)
		close(serv_sock);
	if (sig_fd > 0)
		close(sig_fd);
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
			return (logger.error(MyError("Can't daemonize process")));
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
	int						i, cli, flags;
	ssize_t					r_ret;
    char					buff[BUFF_SZ + 1] = { 0 };
    struct epoll_event		event;
	struct signalfd_siginfo	sig_info;

	event.events = EPOLLIN;
	for (i = 0; i < event_nb; ++i) {
		if (events[i].data.fd == sig_fd) {
			if (read(events[i].data.fd, &sig_info, sizeof(sig_info)) != sizeof(sig_info))
				logger.error(MyError("Can't read signal"));
			logger.signal(sig_info.ssi_signo);
			return (0);
		} else if (events[i].data.fd == serv_sock) {
			if (clients.size() >= CLIENT_NB)
				continue;
			if ((cli = accept(serv_sock, NULL, NULL)) < 0)
				logger.error(MyError("Can't accept new connection"));
			clients.push_back(cli);
			logger.info("New connection accepted");
			if ((flags = fcntl(cli, F_GETFL, 0)) < 0
					|| fcntl(cli, F_SETFL, flags | O_NONBLOCK) < 0)
				logger.error(MyError("Can't configure new client socket"));
			event.data.fd = cli;
			if (epoll_ctl(epoll, EPOLL_CTL_ADD, cli, &event))
				logger.error(MyError("Can't add client socket to epoll instance"));
		} else if ((r_ret = read(events[i].data.fd, buff, BUFF_SZ)) > 0) {
			buff[r_ret - 1] = '\0';
			if (!strcmp(buff, "quit")) {
				logger.info("Request quit");
				return (0);
			}
			else
				logger.log(buff);
		} else {
			if (epoll_ctl(epoll, EPOLL_CTL_DEL, events[i].data.fd, NULL))
				logger.error(MyError("Can't delete client socket from epoll instance"));
			close(events[i].data.fd);
			clients.remove(events[i].data.fd);
			logger.info("Client disconnected");
		}
	}
	return (1);
}

void	Daemon::serv_loop(void) {
	int					event_nb;
    struct epoll_event	event {
		.events = EPOLLIN,
		.data = { .fd = serv_sock },
	}, events[CLIENT_NB + 1];
	sigset_t			sigset;

	sigfillset(&sigset);
	if ((sig_fd = signalfd(-1, &sigset, 0)) < 0)
		logger.error(MyError("Can't create signal fd"));
    if ((epoll = epoll_create1(0)) < 0)
		logger.error(MyError("Can't create epoll instance"));
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, serv_sock, &event))
		logger.error(MyError("Can't add server socket to epoll instance"));
	event.data.fd = sig_fd;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, sig_fd, &event))
		logger.error(MyError("Can't add signal fd to epoll instance"));
	do
		if ((event_nb = epoll_wait(epoll, events, CLIENT_NB + 1, -1)) < 0)
			logger.error(MyError("Can't get pending events"));
	while (event_loop(event_nb, events));
	logger.info("Quitting");
}

void	Daemon::run(void) {
	int				reuse = 1;
	sockaddr_in		serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(DAEMON_PORT),
		.sin_addr = { .s_addr = INADDR_ANY },
		.sin_zero = 0
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
