#include "Tintin_reporter.hpp"

int			error(std::string msg) {
	std::cerr << "Error: Matt_daemon: " << msg << ".\n";
	return (EXIT_FAILURE);
}

int main() {
	sigset_t		sig_mask;
	int				lock_fd, serv_sock, client_sock, reuse = 1;
	Tintin_reporter	tintin;
	sockaddr_in		serv_addr, client_addr;
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

	tintin.info("Entering Daemon mode");

	if (fork()) {
		usleep(100000);
		return EXIT_SUCCESS;
	}
	setsid();
	tintin.info("started", getpid());
	if ((serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return (tintin.error("Can't create socket"));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(4242);
	inet_aton("127.0.0.1", (struct in_addr *)&serv_addr.sin_addr.s_addr);
	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse))) {
		close(serv_sock);
		return (tintin.error("Can't set socket option"));
	}
	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
		close(serv_sock);
		return (tintin.error("Can't bind socket"));
	}
	if (listen(serv_sock, CLIENTS_MAX)) {
		perror("listen");
		close(serv_sock);
		return (tintin.error("Can't listen on port"));
	}
	if ((client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &socklen)) < 0) {
		close(serv_sock);
		return (tintin.error("Can't accept connection"));
	}

	tintin.info("Connection accepted");
	while (42);

	close(lock_fd);
	std::remove("/var/lock/matt_daemon.lock");
	return EXIT_SUCCESS;
}
