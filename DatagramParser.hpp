/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <boost/regex.hpp>
#include "commons.hpp"

class DatagramParser {
public:
	DatagramParser();
	const boost::regex regexp_client_id, regexp_ack, regexp_retransmit,
			regexp_keepalive, regexp_upload, regexp_data;

	//static const size_t MAX_HEADER_SIZE = 30;
	bool matches_client_id(const char*, size_t&);
	bool matches_client_id(std::string&, size_t&);
	bool matches_ack(const char*, size_t&, size_t&);
	bool matches_retransmit(const char*, size_t&);
	bool matches_keepalive(const char*);
	bool matches_upload(const char*, size_t datagram_size, size_t&, char*, size_t&);
	bool matches_data(const char*, size_t datagram_size, size_t&, size_t&, size_t&, char*, size_t&);
};

#endif /* PARSER_HPP_ */
