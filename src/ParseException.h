/*
 * ParseException.h
 *
 *  Created on: 2012-10-17
 *      Author: david
 */

#ifndef PARSEEXCEPTION_H_
#define PARSEEXCEPTION_H_

#include <exception>
#include <sstream>


class ParseException: public std::exception
{
private:
	std::string expected;
	std::string got;
public:
	ParseException(std::string expected, std::string got) {
		this->expected = expected;
		this->got = got;
	}
	virtual const char* what() const throw() {
		std::stringstream ss;
		ss << "Read '" << got << "' where '" << expected << "' was expected!";
		return ss.str().c_str();
	}
	virtual ~ParseException() throw() {}
};


#endif /* PARSEEXCEPTION_H_ */
