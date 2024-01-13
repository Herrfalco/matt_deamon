#ifndef MY_ERROR
#define MY_ERROR

#include "includes.hpp"

class	MyError : public std::exception {
	public:
		MyError(std::string msg = "Default error") : msg(msg) {}
		MyError(const MyError &error) : msg(error.msg) {}
		~MyError(void) {}
		MyError	&operator=(const MyError &error) {
			msg = error.msg;
			return *this;
		}

		const char		*what() const noexcept {
			return (msg.c_str());
		}

	private:
		std::string		msg;
};

#endif // MY_ERROR
