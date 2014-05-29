/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef SERVERCONTROLLER_HPP_
#define SERVERCONTROLLER_HPP_

#include <memory>
#include <map>
#include <vector>
#include <boost/asio.hpp>
#include "ClientContext.hpp"
#include "commons.hpp"
#include "mixer.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class ServerController {
public:
	ServerController();
    uint16_t port, fifo_size, low_mark, high_mark, buffer_len, tx_interval;
	const static uint16_t MAX_CLIENT_NO = 1000;
    std::map<size_t, std::shared_ptr<ClientContext>> clients; //key: id
	typedef std::map<udp::endpoint, std::shared_ptr<ClientContext> > map_udp_endpoint_t;
	map_udp_endpoint_t map_udp_endpoint;
	std::shared_ptr<ClientContext> addClient(tcp::socket tcp_socket);
	bool removeClient(int id);
	std::string mix();
	void turnOnTcpSever();
	void turnOnUdpSever();
	bool is_tcp_server_on, is_udp_server_on; //for developing aspects

private:
	uint32_t next_id;

	//buffers:
	char mixer_buffer[354000];
	mixer_input mixer_inputs_buffer[MAX_CLIENT_NO];
	std::shared_ptr<ClientContext> active_ptr_buffer[MAX_CLIENT_NO];
};

#endif /* SERVERCONTROLLER_HPP_ */
