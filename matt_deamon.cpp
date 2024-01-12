#include "Tintin_reporter.hpp"

int			error(std::string msg) {
	std::cerr << "Error: Matt_daemon: " << msg << ".\n";
	return (EXIT_FAILURE);
}

int main() {
	sigset_t		sig_mask;
	int				lock_fd, serv_sock, client_sock;
	Tintin_reporter	tintin;
	sockaddr_un		serv_addr, client_addr;
	socklen_t		socklen;

	if (getuid())
		return error("This program must be run as root");
	sigfillset(&sig_mask);
	sigprocmask(SIG_SETMASK, &sig_mask, NULL);
	if ((lock_fd = open("/var/lock/matt_daemon.lock", O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
		tintin.error("Can't open lock file");
		return (error("Can't open lock file"));
	}

	if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
		tintin.error("Another instance is already running");
		return (error("Another instance is already running"));
	}

	tintin.log("Entering Daemon mode");

	if (fork()) {
		usleep(100000);
		return EXIT_SUCCESS;
	}
	setsid();
	tintin.info("started", getpid());
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

	while (42);

	tintin.log("Connection accepted");
	close(lock_fd);
	std::remove("/var/lock/matt_daemon.lock");
	return EXIT_SUCCESS;
}
