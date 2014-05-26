/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "commons.hpp"

std::string endpointToString(boost::asio::ip::udp::endpoint ep) {
	return ep.address().to_string() + ":" + _(ep.port());
}

std::string endpointToString(boost::asio::ip::tcp::endpoint ep) {
	return ep.address().to_string() + ":" + _(ep.port());
}
