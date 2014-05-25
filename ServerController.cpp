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
	tx_interval = 1000; // ms //TODO 5

	is_tcp_server_on = is_udp_server_on = false;
}

std::shared_ptr<ClientContext> ServerController::addClient(
		tcp::socket tcp_socket) {
	std::shared_ptr<ClientContext> cc(
			new ClientContext(next_id, std::move(tcp_socket), fifo_size, low_mark, high_mark));
	clients.insert(std::make_pair(next_id, cc));
	INFO("Client added, id: " + std::to_string(next_id));
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
		INFO("Removing [TCP] disconnected client.");
	} else {
		INFO("Removing [TCP] connected client.");
	}
	auto udp_endpoint = mit->second->getUdpEndpoint();
	if (map_udp_endpoint.erase(udp_endpoint) != 1) {
		INFO("UDP endpoint mapping does not exist for removing client. Continue..");
	}
	clients.erase(mit);
	INFO("Client removed, id: " + std::to_string(id));
	return true;
}

std::string ServerController::mix() {
	std::vector<std::shared_ptr<ClientContext>>active;

	for (auto it = map_udp_endpoint.begin(); it != map_udp_endpoint.end();
			it++) {
		if (it->second->data_fifo_state
				== ClientContext::DataFifoState::ACTIVE) {
			active.push_back(it->second);
		}

	}
	size_t n = active.size();
	mixer_input *inputs = new mixer_input[n];

	for ( size_t i = 0; i < n; i++ ) {
		inputs[i] = active[i]->mix;
	}

	size_t *size = new size_t( 175 * tx_interval);
	char *output = new char[*size];

	mixer(inputs, n, output, size, tx_interval);

	for ( size_t i = 0; i < active.size(); i++ ) {
		active[i]->mix.consumed = inputs[i].consumed;
		active[i]->mix.len = inputs[i].len;
		active[i]->consumeData();
	}

	std::string result(output, output + *size);

	delete output;
	delete size;
	delete inputs;

	LOG("Result size: " + _(result.size()) + ", active clients: " + _(n) );
	return result;
}

void ServerController::turnOnTcpSever() {
	is_tcp_server_on = true;
}

void ServerController::turnOnUdpSever() {
	is_udp_server_on = true;
}
