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
	ClientContext(size_t id, tcp::socket tcp_socket, uint16_t fifo_size,
			uint16_t low_mark, uint16_t high_mark);

	enum DataFifoState {
		FILLING, ACTIVE
	} data_fifo_state;

	size_t getId() const { return id; }
	size_t getFIFOSize() const { return mix.len; }
	size_t getAllowedWin() const { return mix_fifo_maxsize - mix.len; }
	tcp::socket& getTcpSocket() { return tcp_socket; }
	const udp::endpoint& getUdpEndpoint() const { return udp_endpoint; }
	size_t getExpectedAck() const { return expected_ack; }
	uint16_t getMixFifoLastMin() const { return mix_fifo_last_min; }
	uint16_t getMixFifoLastMax() const { return mix_fifo_last_max; }

	bool isActiveUDP() const;
	void correctlastUdpTime();

	void setUdpEndpoint(const udp::endpoint& udpEndpoint) {
		udp_endpoint = udpEndpoint;
		// Client may need some more time in the beginning before send `KEEPALIVE`
		last_udp_time = boost::posix_time::microsec_clock::local_time()
				+ boost::posix_time::milliseconds(2000);
	}

	void addData(const char* data, size_t s);
	void resetDataStatus();
	void consumeData();

	mixer_input mix;

private:
	size_t id;
	tcp::socket tcp_socket;
	uint16_t mix_fifo_maxsize, low_mark, high_mark, mix_fifo_last_min, mix_fifo_last_max;
	udp::endpoint udp_endpoint;
	boost::posix_time::ptime last_udp_time;
	const static uint16_t ALLOWED_UDP_INTERVAL_MS = 1000;
	size_t expected_ack; // must be equal to next UPLOAD -> nr <- datagram
};

#endif /* CLIENTCONTEX_HPP_ */
