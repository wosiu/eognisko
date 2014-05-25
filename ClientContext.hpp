/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef CLIENTCONTEX_HPP_
#define CLIENTCONTEX_HPP_

#include <cstring>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "mixer.hpp"
#include "commons.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;


class ClientContext {
public:
	ClientContext(uint32_t id, tcp::socket tcp_socket, uint16_t fifo_size, uint16_t low_mark, uint16_t high_mark);
	~ClientContext();
	uint32_t getId() {
		return id;
	}

	size_t getFIFOSize() const {
		return mix.len;
	}

	size_t getAllowedWin() const {
		return mix_fifo_maxsize - mix.len;
	}

	tcp::socket& getTcpSocket() {
		return tcp_socket;
	}

	uint32_t getExpectedAck() const {
		return expected_ack;
	}

	uint16_t last_min, last_max, mix_fifo_maxsize, low_mark, high_mark;

	//TODO in client:
	//uint32_t expected_data_nr; //init with -1
	//uint32_t expected_max_window;

	enum DataFifoState {FILLING, ACTIVE} data_fifo_state;

	void correctlastUdpTime();
	bool isActiveUDP();

	const udp::endpoint& getUdpEndpoint() const {
		return udp_endpoint;
	}

	void setUdpEndpoint(const udp::endpoint& udpEndpoint) {
		udp_endpoint = udpEndpoint;
	}

	void addData(const char* data, size_t s);
	void resetDataStatus();
	void consumeData();



	mixer_input mix;
private:
	uint32_t id;
	tcp::socket tcp_socket;
	udp::endpoint udp_endpoint;
	boost::posix_time::ptime last_udp_time;
	const static uint16_t ALLOWED_UDP_INTERVAL_S = 1;
	// must be equal to next UPLOAD -> nr <- datagram
	uint32_t expected_ack;
	//FIFO
};

#endif /* CLIENTCONTEX_HPP_ */
