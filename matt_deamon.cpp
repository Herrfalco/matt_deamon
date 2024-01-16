#include "Daemon.hpp"


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
		dm.run();
	} catch (MyError &e) {
		return (error(e));
	}

	return EXIT_SUCCESS;
}
