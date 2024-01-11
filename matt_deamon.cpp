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

#define	CLIENTS_MAX 3

class Tintin_reporter {
	public:
		std::ofstream	log_stream;

		Tintin_reporter() {
			mkdir("/var/log/matt_daemon", 0777);
			log_stream.open("/var/log/matt_daemon/matt_daemon.log", std::ios::app);
			log("Started.");
		};

		Tintin_reporter(const Tintin_reporter &other) {};

		~Tintin_reporter() {
			log("Quitting.");
		};

		Tintin_reporter &operator=(const Tintin_reporter &other) { return *this;};

		void log(const std::string &message, std::string type = "INFO", int pid = 0) {
			if (log_stream.is_open()) {
				time_t now = time(0);
				tm *ltm = localtime(&now);
				char timestamp[250];

				strftime(timestamp, sizeof(timestamp), "[%d/%m/%Y-%H:%M:%S]", ltm);
				log_stream << timestamp << " [ " <<  type << " ] - Matt_daemon: " << message;
				if (pid)
					log_stream << pid << ".";
				log_stream << std::endl;
			} else
				std::cerr << "Error opening log file!" << std::endl;
		}
};

int main() {
	sigset_t		sig_mask;
	int				lock_fd, serv_sock, client_sock;
	Tintin_reporter	tintin;
	sockaddr_un		serv_addr, client_addr;
	socklen_t		socklen;

	if (getuid()) {
		std::cerr << "This program must be run as root." << std::endl;
		return EXIT_FAILURE;
	}
	sigfillset(&sig_mask);
	sigprocmask(SIG_SETMASK, &sig_mask, NULL);
	if ((lock_fd = open("/var/lock/matt_daemon.lock", O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
		tintin.log("Can't open lock file.", "ERROR");
		std::cerr << "Can't open lock file." << std::endl;
		return EXIT_FAILURE;
	}

	if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
		tintin.log("Error file locked.", "ERROR");
		std::cerr << "Another instance is already running." << std::endl;
		return EXIT_FAILURE;
	}
	tintin.log("Entering Daemon mode.");
	if (fork()) {
		usleep(100000);
		return EXIT_SUCCESS;
	}
	setsid();
	tintin.log("started. PID: ", "INFO", getpid());
	if ((serv_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		tintin.log("Can't create socket.", "ERROR");
		return EXIT_FAILURE;
	}
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, "/tmp/matt_daemon_socket");
	if (bind(serv_sock, (struct sockaddr *)&serv_addr,
			strlen(serv_addr.sun_path) + sizeof(sa_family_t))) {
		tintin.log("Can't bind socket.", "ERROR");
		close(serv_sock);
		return EXIT_FAILURE;	
	}
	if (listen(serv_sock, CLIENTS_MAX)) {
		tintin.log("Can't listen on port.", "ERROR");
		close(serv_sock);
		return EXIT_FAILURE;
	}
	if ((client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &socklen)) < 0) {
		tintin.log("Can't accept connection.", "ERROR");
		close(serv_sock);
		return EXIT_FAILURE;
	}
	while (42);
	tintin.log("Connection accepted.");
	close(lock_fd);
	std::remove("/var/lock/matt_daemon.lock");
	return EXIT_SUCCESS;
}
