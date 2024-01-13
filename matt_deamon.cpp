#include "Tintin_reporter.hpp"

class	Daemon {
	public:
		Daemon(void);
		Daemon(const Daemon &daemon) : logger(daemon.logger), lock_fd(daemon.lock_fd) {};
		~Daemon(void);

		Daemon	&operator=(const Daemon &daemon);
		void	detach(void);
//		void	run(void);

	private:
		Tintin_reporter		logger;
		int					lock_fd;
};

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

	logger.log("Entering Daemon mode");
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

/*
void	Daemon::run(void) {
	int				serv_sock, client_sock;
	sockaddr_un		serv_addr, client_addr;
	socklen_t		socklen;

	if ((serv_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		tintin.error("Can't create socket");
		return EXIT_FAILURE;
	}
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, "/tmp/matt_daemon_socket");
	if (bind(serv_sock, (struct sockaddr *)&serv_addr,
			strlen(serv_addr.sun_path) + sizeof(sa_family_t))) {
		close(serv_sock);
		return (tintin.error("Can't bind socket"));
	}

	if (listen(serv_sock, CLIENTS_MAX)) {
		close(serv_sock);
		return (tintin.error("Can't listen on port"));
	}

	if ((client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &socklen)) < 0) {
		close(serv_sock);
		return (tintin.error("Can't accept connection"));
	}

	tintin.log("Connection accepted");
}
*/

int			error(const MyError &err) {
	std::cerr << "Error: Matt_daemon: " << err.what() << ".\n";
	return (EXIT_FAILURE);
}

int main() {
	sigset_t		sig_mask;

	sigfillset(&sig_mask);
	sigprocmask(SIG_SETMASK, &sig_mask, NULL);
	if (getuid())
		return error(MyError("This program must be run as root"));

	try {
		Daemon			dm;

		dm.detach();
		while (42);
//		dm.run();
	} catch (MyError &e) {
		return (error(e));
	}

	return EXIT_SUCCESS;
}
