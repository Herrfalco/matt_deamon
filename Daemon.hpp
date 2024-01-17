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
		int		event_loop(int event_nb, struct epoll_event *events);
		void	serv_loop(void);

		Tintin_reporter		logger;
		int					lock_fd;
		int					serv_sock;
		int					sig_fd;
		int					epoll;
		std::list<int>		clients;
};
