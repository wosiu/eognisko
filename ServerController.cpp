/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */
#include "ServerController.hpp"

ServerController::ServerController() {
	// setting defaults
	next_id = 0;
	port = 10000 + (336071 % 10000);
	fifo_size = 10560;
	low_mark = 0;
	high_mark = fifo_size;
	buffer_len = 10;
	tx_interval = 5;
}

std::shared_ptr<ClientContext> ServerController::addClient(
		tcp::socket tcp_socket) {
	std::shared_ptr<ClientContext> cc(
			new ClientContext(next_id, std::move(tcp_socket)));
	clients.insert(std::make_pair(next_id, cc));
	LOG("Client added, id: " + std::to_string(next_id));
	next_id++;
	return cc;
}

bool ServerController::removeClient(int id) {
	auto mit = clients.find(id);
	if (mit == clients.end()) {
		ERR("Cannot remove client that does not exist.");
		return false;
	}
	boost::system::error_code ec;
	auto tcp_endpoint = mit->second->getTcpSocket().remote_endpoint(ec);
	if (ec) {
		LOG("Removing [TCP] disconnected client.");
	} else {
		LOG("Removing [TCP] connected client.");
	}
	auto udp_endpoint = mit->second->getUdpEndpoint();
	if (map_udp_endpoint.erase(udp_endpoint) != 1) {
		LOG("UDP endpoint mapping does not exist for removing client. Continue..");
	}
	clients.erase(mit);
	LOG("Client removed, id: " + std::to_string(id));
	return true;
}

