#include "Tintin_reporter.hpp"

Tintin_reporter::Tintin_reporter(void) {
	mkdir("/var/log/matt_daemon", 0777);
	log_stream.open("/var/log/matt_daemon/matt_daemon.log",
			std::ios::app);
	log_stream.exceptions(std::ios_base::failbit
			| std::ios_base::badbit);
	log("Started.");
}	

Tintin_reporter::~Tintin_reporter(void) {
	log("Quitting.");
	log_stream.close();
}

void	Tintin_reporter::log_hdr(const std::string &type) {
	time_t			now = time(0);
	struct 	tm		*ltm = localtime(&now);
	char			timestamp[128];

	strftime(timestamp, sizeof(timestamp), "[%d/%m/%Y-%H:%M:%S]", ltm);
	log_stream << timestamp << " [ " <<  type << " ] - Matt_daemon: ";
}

void	Tintin_reporter::info(const std::string &msg, int pid) {
	log_hdr("INFO");
	log_stream << msg;
	if (pid)
		log_stream << ". PID:" << pid;
	log_stream << ".\n";
}

int		Tintin_reporter::error(const std::string &msg) {
	log_hdr("ERROR");
	log_stream << msg << ".\n";
	return (EXIT_FAILURE);
}

void	Tintin_reporter::log(const std::string &msg) {
	log_hdr("LOG");
	log_stream << "User input: " << msg << std::endl;
}
