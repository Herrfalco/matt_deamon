#ifndef TINTIN_REPORTER
#define TINTIN_REPORTER

#include "MyError.hpp"

class Tintin_reporter {
	public:
		Tintin_reporter(void);
		Tintin_reporter(const Tintin_reporter &rep) : log_fd(rep.log_fd) {};
		~Tintin_reporter(void);
		Tintin_reporter &operator=(const Tintin_reporter &rep);

		void	 info(const std::string &message, int pid = 0);
		int		 error(const MyError &error);
		void	 log(const std::string &message);

	private:
		int		log_fd;

		void	log_hdr(const std::string &message);
};

#endif // TINTIN_REPORTER
