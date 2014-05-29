/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "ClientContext.hpp"

ClientContext::ClientContext(size_t id, tcp::socket tcp_socket,
		uint16_t fifo_size, uint16_t low_mark, uint16_t high_mark) :
			id(id),
			tcp_socket(std::move(tcp_socket)),
			mix_fifo_maxsize(fifo_size),
			low_mark(low_mark),
			high_mark(high_mark) {

	mix_fifo_last_min = mix_fifo_last_max = 0;
	mix.consumed = mix.len = 0;
	expected_ack = 0;
	mix.data = new char[mix_fifo_maxsize];
	data_fifo_state = FILLING;
}

void ClientContext::correctlastUdpTime() {
	last_udp_time = boost::posix_time::microsec_clock::local_time();
}

bool ClientContext::isActiveUDP() const {
	boost::posix_time::ptime now =
			boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration diff = now - last_udp_time;
	return diff.total_milliseconds() <= ALLOWED_UDP_INTERVAL_MS;
}

void ClientContext::addData(const char* data, size_t s) {
	if ( s + mix.len > mix_fifo_maxsize ) {
		ERR("Too big data to add to client FIFO. Probably bad window announced.");
		s = std::min(s, mix_fifo_maxsize - mix.len);
	}
	memmove((char *) mix.data + mix.len, data, s);
	mix.len += s;
	mix_fifo_last_max = std::max((size_t)mix_fifo_last_max, mix.len);

	if ( mix.len >= high_mark ) {
		data_fifo_state = ACTIVE;
	}

	expected_ack++;
}

void ClientContext::resetDataStatus() {
	mix_fifo_last_min = mix_fifo_last_max = mix.len;
}

void ClientContext::consumeData() {
	if ( mix.consumed > mix.len ) {
		ERR("Order to consume more than fifo lengh");
		mix.consumed = mix.len;
	}

	uint16_t res = mix.len - mix.consumed;

	memmove(mix.data, (char *)mix.data + mix.consumed, res);
	mix.len = res;
	mix_fifo_last_min = std::min(mix_fifo_last_min, res);

	if ( mix.len <= low_mark ) {
		data_fifo_state = FILLING;
	}
	mix.consumed = 0;
}
