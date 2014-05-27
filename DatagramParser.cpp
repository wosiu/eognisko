/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "DatagramParser.hpp"

DatagramParser::DatagramParser() :
		regexp_client_id("CLIENT ([0-9]*)\n"), regexp_ack(
				"ACK ([0-9]*) ([0-9]*)\n"), regexp_retransmit(
				"RETRANSMIT ([0-9]*)\n"), regexp_keepalive("KEEPALIVE\n"), regexp_upload(
				"UPLOAD ([0-9]*)\n"), regexp_data(
				"DATA ([0-9]*) ([0-9]*) ([0-9]*)\n") {
}

bool DatagramParser::matches_client_id(const char* input_string, size_t& client_id) {
	boost::cmatch groups;
	if (boost::regex_match(input_string, groups, regexp_client_id)) {
		client_id = std::strtoul(groups[1].first, NULL, 0);
		return true;
	}
	return false;
}

bool DatagramParser::matches_ack(const char* input_string, size_t& ack, size_t& win) {
	boost::cmatch groups;
	if (boost::regex_match(input_string, groups, regexp_ack)) {
		ack = std::strtoul(groups[1].first, NULL, 0);
		win = std::strtoul(groups[2].first, NULL, 0);
		return true;
	}
	return false;
}

bool DatagramParser::matches_retransmit(const char* input_string, size_t& nr) {
	boost::cmatch groups;
	if (boost::regex_match(input_string, groups, regexp_retransmit)) {
		nr = std::strtoul(groups[1].first, NULL, 0);
		return true;
	}
	return false;
}

bool DatagramParser::matches_keepalive(const char* input_string) {
	boost::cmatch groups;
	if (boost::regex_match(input_string, groups, regexp_keepalive))
		return true;
	return false;
}

bool DatagramParser::matches_upload(const char* input_string, const size_t datagram_size, size_t& nr, char* data, size_t& data_size) {
	boost::cmatch groups;
	const char* end = (const char*) memchr(input_string, '\n', datagram_size);
	if ( end == NULL ) {
		return false;
	}
	end++;
	if (boost::regex_match(input_string, end, groups, regexp_upload)) {
		nr = std::strtoul(groups[1].first, NULL, 0);
		size_t header_size = end - input_string;
		data_size = datagram_size - header_size;
		memcpy(data, end, data_size);
		return true;
	}
	return false;
}

bool DatagramParser::matches_data(const char* input_string, const size_t datagram_size, size_t& nr, size_t& ack,
		size_t& win, char* data, size_t& data_size) {
	boost::cmatch groups;
	const char* end = (const char*) memchr(input_string, '\n', datagram_size);
	if ( end == NULL ) {
		return false;
	}
	end++;
	if (boost::regex_match(input_string, end, groups, regexp_data)) {
		nr = std::strtoul(groups[1].first, NULL, 0);
		ack = std::strtoul(groups[2].first, NULL, 0);
		win = std::strtoul(groups[3].first, NULL, 0);
		size_t header_size = end - input_string;
		data_size = datagram_size - header_size;
		memcpy(data, end, data_size );
		return true;
	}
	return false;
}
