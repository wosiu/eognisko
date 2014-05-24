/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef CLIENTCONTEX_HPP_
#define CLIENTCONTEX_HPP_

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ClientContext {
public:
	ClientContext( uint32_t id, tcp::socket tcp_socket );
	uint32_t getId() { return id; }
	//TODO
	uint16_t getFIFOSize() { return 0; }
	tcp::socket& getTcpSocket() { return tcp_socket; }
	tcp::endpoint& getTcpEndpointCopy() { return tcp_endpoint_copy; }
	uint16_t last_min, last_max;

private:
	uint32_t id;
	tcp::socket tcp_socket;
	tcp::endpoint tcp_endpoint_copy; //copy in case of disconnection
	//socket udp
	//FIFO
};

#endif /* CLIENTCONTEX_HPP_ */
