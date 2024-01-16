#include "Tintin_reporter.hpp"

class	Daemon {
	public:
		Daemon(void);
		Daemon(const Daemon &daemon) : logger(daemon.logger), lock_fd(daemon.lock_fd) {};
		~Daemon(void);

		Daemon	&operator=(const Daemon &daemon);
		void	detach(void);
		void	run(void);

	private:
		Tintin_reporter		logger;
		int					lock_fd;
		int					serv_sock;
		int					client_sock;
		pthread_t			thrds[3];

		void	request_quit(void);
		void	*handle_client(void *arg);
};
