#ifndef TINTIN_REPORTER
#define TINTIN_REPORTER

#include "includes.hpp"

class Tintin_reporter {
	public:
		Tintin_reporter();
		Tintin_reporter(const Tintin_reporter &other) {};
		~Tintin_reporter(void);
		Tintin_reporter &operator=(const Tintin_reporter &other) { return *this; };

		void	 info(const std::string &message, int pid = 0);
		int		 error(const std::string &message);
		void	 log(const std::string &message);

	private:
		std::ofstream	log_stream;

		void	log_hdr(const std::string &message);
};

#endif // TINTIN_REPORTER
