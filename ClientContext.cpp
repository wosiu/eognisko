/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "ClientContext.hpp"

ClientContext::ClientContext(uint32_t id, tcp::socket tcp_socket) : id(id), tcp_socket(std::move(tcp_socket)){
	last_min = last_max = 0;
}

void ClientContext::correctlastUdpTime() {
	last_udp_time = boost::posix_time::microsec_clock::local_time();
}

bool ClientContext::isActiveUDP() {
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	boost::posix_time::time_duration diff = now - last_udp_time;
	return diff.total_seconds() <= ALLOWED_UDP_INTERVAL_S;
}
