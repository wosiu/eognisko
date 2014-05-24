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

	// Sprawdza czy client o podanym endpoincie juz nie istnieje
	const tcp::endpoint& endpoint = tcp_socket.remote_endpoint(); //throwable!
	auto mit = map_endpoint.find(endpoint);
	if (mit != map_endpoint.end()) {
		LOG("Client already exists.");
		return mit->second;
	}
	std::shared_ptr<ClientContext> cc(
			new ClientContext(next_id, std::move(tcp_socket)));
	clients.insert(std::make_pair(next_id, cc));
	next_id++;
	map_endpoint.insert(
			std::make_pair(endpoint, cc));
	LOG("Client added");
	return cc;
}

bool ServerController::removeClient(int id) {
	auto mit = clients.find(id);
	if (mit == clients.end()) {
		LOG("Cannot remove client that does not exist.");
		return false;
	}
	boost::system::error_code ec;
	auto endpoint = mit->second->getTcpSocket().remote_endpoint(ec);
	if (ec) {
		LOG("Removing [TCP] disconnected client.");
		endpoint = mit->second->getTcpEndpointCopy();
	} else {
		LOG("Removing [TCP] connected client.");
	}
	if (map_endpoint.erase(endpoint) != 1) {
		ERR("Endpoint mapping does not exist for existing client. Continue removing..");
	}
	clients.erase(mit);
	LOG("Client removed, id: " + std::to_string(id));
	return true;
}

