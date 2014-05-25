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
	std::map<int, std::shared_ptr<ClientContext>> clients; //key: id
	std::map<udp::endpoint, std::shared_ptr<ClientContext> > map_udp_endpoint;
	std::shared_ptr<ClientContext> addClient(tcp::socket tcp_socket);
	bool removeClient(int id);
	std::string mix();

private:
	uint32_t next_id;
};

#endif /* SERVERCONTROLLER_HPP_ */