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
	tx_interval = (IS_DEB) ? 1000 : 5; // ms
	is_tcp_server_on = is_udp_server_on = false;

	memset(mixer_buffer, 0, sizeof(mixer_buffer));
}

std::shared_ptr<ClientContext> ServerController::addClient(
		tcp::socket tcp_socket) {
	std::shared_ptr<ClientContext> cc(
			new ClientContext(next_id, std::move(tcp_socket), fifo_size, low_mark, high_mark));
	clients.insert(std::make_pair(next_id, cc));
	INFO("Client added, id: " + _(next_id));
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
	INFO("Client removed, id: " + _(id));
	return true;
}

std::string ServerController::mix() {
	size_t n = 0;
	size_t size = 0;

	if ( map_udp_endpoint.empty() ) {
		ERR("Co clients to make mix for them!");
	}

	for (auto it = map_udp_endpoint.begin(); it != map_udp_endpoint.end();
			it++) {
		if (it->second->data_fifo_state
				== ClientContext::DataFifoState::ACTIVE) {
			mixer_inputs_buffer[n] = it->second->mix;
			active_ptr_buffer[n] = it->second;
			n++;
		}
	}
	LOG("Active buffors no: " + _(n));
	DEB("before mixer: out size " + _(size) + ", mixer_buffer size " + _(strlen(mixer_buffer)));

	mixer(mixer_inputs_buffer, n, mixer_buffer, &size, tx_interval);

	DEB("after mixer: out size " + _(size) + ", mixer_buffer size " + _(strlen(mixer_buffer)));
	/*if ( size > 516 ) {
		WARN("Big mixer output size: " + _(size));
	}*/

	for ( size_t i = 0; i < n; i++ ) {
		active_ptr_buffer[i]->mix.consumed = mixer_inputs_buffer[i].consumed;
		active_ptr_buffer[i]->mix.len = mixer_inputs_buffer[i].len;
		active_ptr_buffer[i]->consumeData();
	}

	std::string result(mixer_buffer, mixer_buffer + size);
	//std::fill(mixer_buffer, mixer_buffer + size + 1, 0);
	memset(mixer_buffer, 0, size + 1);

	DEB("Result size: " + _(result.size()));
	return result;
}

void ServerController::turnOnTcpSever() {
	is_tcp_server_on = true;
}

void ServerController::turnOnUdpSever() {
	is_udp_server_on = true;
}
