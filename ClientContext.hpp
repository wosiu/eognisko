/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef CLIENTCONTEX_HPP_
#define CLIENTCONTEX_HPP_

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class ClientContext {
public:
	ClientContext(uint32_t id, tcp::socket tcp_socket);
	uint32_t getId() {
		return id;
	}
	//TODO
	uint16_t getFIFOSize() {
		return 0;
	}
	tcp::socket& getTcpSocket() {
		return tcp_socket;
	}
	uint16_t last_min, last_max;
	void correctlastUdpTime();
	bool isActiveUDP();

	const udp::endpoint& getUdpEndpoint() const {
		return udp_endpoint;
	}

	void setUdpEndpoint(const udp::endpoint& udpEndpoint) {
		udp_endpoint = udpEndpoint;
	}

private:
	uint32_t id;
	tcp::socket tcp_socket;
	udp::endpoint udp_endpoint;
	boost::posix_time::ptime last_udp_time;
	const static uint16_t ALLOWED_UDP_INTERVAL_S = 1;
	//FIFO
};

#endif /* CLIENTCONTEX_HPP_ */
