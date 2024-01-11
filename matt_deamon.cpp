#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <ctime>
#include <fcntl.h>

class Tintin_reporter {
	public:
		static void log(const std::string& message) {
			std::ofstream logFile("/var/log/matt_daemon/matt_daemon.log", std::ios::app);
			if (logFile.is_open()) {
				time_t now = time(0);
				tm *ltm = localtime(&now);
				char timestamp[20];
				strftime(timestamp, sizeof(timestamp), "[%d/%m/%Y-%H:%M:%S]", ltm);
				logFile << timestamp << " [" << " INFO " << "] - Matt_daemon: " << message << std::endl;
				logFile.close();
			} else {
				std::cerr << "Error opening log file!" << std::endl;
			}
		}
};

volatile sig_atomic_t signalReceived = 0;

void signalHandler(int signum) {
	signalReceived = signum;
	Tintin_reporter::log("Signal handler.");
}

void createLockFile() {
	std::ofstream lockFile("/var/lock/matt_daemon.lock");
	if (!lockFile) {
		Tintin_reporter::log("Error file locked.");
		exit(EXIT_FAILURE);
	}
	lockFile.close();
}

void deleteLockFile() {
	remove("/var/lock/matt_daemon.lock");
}

int main() {
	int		lock_fd;
	int		log_fd;

	if (getuid()) {
		std::cerr << "This program must be run as root." << std::endl;
		return EXIT_FAILURE;
	}

	// Check if another instance is running by attempting to open the lock file
	std::ifstream testLockFile("/var/lock/matt_daemon.lock");
	if ((lock_fd = open("/var/lock/matt_daemon.lock", O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
		//nikoumouk
		return EXIT_FAILURE;
	}

	if (flock(lock_fd, LOCK_EX | LOCK_NB) == EWOULDBLOCK) {
		std::cerr << "Another instance is already running." << std::endl;
	}
	if (testLockFile.is_open()) {
		return EXIT_FAILURE;
	}
	testLockFile.close();

	createLockFile();

	// Signal handling
	signal(SIGTERM, signalHandler);
	// Other signal handling for necessary signals

	// Daemonize the process
	// Fork the process and handle child/parent termination

	// Create socket and listen to port 4242
	// Manage incoming connections and limit to 3 clients

	// Log daemon startup
	Tintin_reporter::log("Started.");

	// Main daemon logic to handle client interactions
	// Log incoming messages or commands

	// When receiving "quit", log and shutdown the daemon
	if (signalReceived == SIGTERM) {
		Tintin_reporter::log("Request quit.");
		// Clean up resources
		deleteLockFile();
		Tintin_reporter::log("Quitting.");
		// Properly terminate the daemon
	}

	return EXIT_SUCCESS;
}
