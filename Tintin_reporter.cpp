#include "Tintin_reporter.hpp"

Tintin_reporter::Tintin_reporter(void) {
	mkdir("/var/log/matt_daemon", 0777);
	if ((log_fd = open("/var/log/matt_daemon/matt_daemon.log",
					O_WRONLY | O_CREAT | O_APPEND)) < 0)
		throw (MyError("Can't open log file"));		
	info("Started");
}	

Tintin_reporter::~Tintin_reporter(void) {
	info("Quitting");
	close(log_fd);
}

Tintin_reporter	&Tintin_reporter::operator=(const Tintin_reporter &rep) {
	log_fd = rep.log_fd;
	return *this;
}

void	Tintin_reporter::log_hdr(const std::string &type) {
	time_t		now = time(0);
	struct tm	*ltm = localtime(&now);
	char		timestamp[BUFF_SZ];

	strftime(timestamp, BUFF_SZ, "[%d/%m/%Y-%H:%M:%S]", ltm);
	dprintf(log_fd, "%s [ %s ] - Matt_daemon: ", timestamp, type.c_str());
}

void	Tintin_reporter::info(const std::string &msg, int pid) {
	log_hdr("INFO");
	dprintf(log_fd, "%s", msg.c_str());
	if (pid)
		dprintf(log_fd, ". PID: %d", pid);
	dprintf(log_fd, ".\n");
}

void	Tintin_reporter::error(const MyError &err) {
	log_hdr("ERROR");
	dprintf(log_fd, "%s.\n", err.what());
	exit(EXIT_FAILURE);
}

void	Tintin_reporter::log(const std::string &msg) {
	log_hdr("LOG");
	dprintf(log_fd, "User input: %s\n", msg.c_str());
}
