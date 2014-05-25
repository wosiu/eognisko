/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "UdpServer.hpp"

UdpServer::UdpServer(boost::asio::io_service& io_service,
		ServerController& _controller) :
		controller(_controller), timer_sound_send(io_service,
				boost::posix_time::millisec(_controller.tx_interval)), timer_udp_check(
				io_service, boost::posix_time::seconds(UDP_CHECK_INTERVAL_S)), socket_server_udp(
				io_service, udp::endpoint(udp::v6(), _controller.port)) {
	LOG("UDP server started.");
	mixed_datagrams_counter = 0;
	// start receiving
	receiveDatagram();
	// start sending data to clients
	timer_sound_send.async_wait(boost::bind(&UdpServer::sendMixed, this));
	// start checking UDP clients' connections
	timer_udp_check.async_wait(boost::bind(&UdpServer::checkUdpConnections, this, boost::asio::placeholders::error));
}

void UdpServer::sendMixed() {
	// Note, that sending reports will start when first client come
	if (controller.clients.empty()) {
		LOG("No clients to send sound.");
	} else {
		LOG("Send sound");
		//TODO mix
		//auto mixed_data = controller.getMixedData();
		auto mixed_data = "3=> \n3==> \n3===> \n";

		for (auto it = controller.map_udp_endpoint.begin();
				it != controller.map_udp_endpoint.end(); it++) {
	        //TODO
			//auto header = "DATA " + std::to_string(mixed_datagrams_counter)
	        //                + get_ack_win(from_pars(endpoint)) + "\n";
			auto header = "DATA " + std::to_string(mixed_datagrams_counter) + "1" + "\n";
			auto endpoint = it->first;
			socket_server_udp.send_to(boost::asio::buffer(header + mixed_data), endpoint);
		}
		//TODO push datagram to storage
		mixed_datagrams_counter++;
	}
	timer_sound_send.expires_from_now( boost::posix_time::millisec(controller.tx_interval));
	timer_sound_send.async_wait(boost::bind(&UdpServer::sendMixed, this));
}

void UdpServer::receiveDatagram() {
	socket_server_udp.async_receive_from(boost::asio::buffer(message_buffer),
			new_client_endpoint,
			[this](boost::system::error_code ec, std::size_t size)
			{
				if (!ec) {
					LOG("Receive datagram from client.");
					processClientDatagram(size);
				} else {
					ERR(ec);
				}

				receiveDatagram();
			});
}

void UdpServer::checkUdpConnections(const boost::system::error_code& error) {
	if ( error ) {
		ERR(error);
	} else {
		LOG("Checking UDP connections.");
		for (auto it = controller.map_udp_endpoint.begin();
				it != controller.map_udp_endpoint.end();) {
			if ( !it->second->isActiveUDP() ) {
				LOG("Inactive client UDP connection detected. Removing.");
				auto id_to_remove = it->second->getId();
				it++;
				controller.removeClient(id_to_remove);
			} else {
				it++;
			}
		}
	}

	timer_udp_check.expires_from_now( boost::posix_time::seconds(UDP_CHECK_INTERVAL_S) );
	timer_udp_check.async_wait(boost::bind(&UdpServer::checkUdpConnections, this, boost::asio::placeholders::error));
}

// parsing and processing user datagram
void UdpServer::processClientDatagram(size_t message_size) {
	LOG("Parsing user datagram.");
}
