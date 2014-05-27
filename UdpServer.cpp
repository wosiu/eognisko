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
				io_service, udp::endpoint(udp::v4(), _controller.port)) {
	INFO("UDP server started.");
	mixed_datagrams_counter = 0;
	controller.turnOnUdpSever();
	// start receiving
	receiveDatagram();
	// start sending data to clients
	timer_sound_send.async_wait(boost::bind(&UdpServer::sendMixed, this));
	// start checking UDP clients' connections
	// TODO uncomment
	/*timer_udp_check.async_wait(
			boost::bind(&UdpServer::checkUdpConnections, this,
					boost::asio::placeholders::error));
	*/
}



// run mixing and sending datagram DATA with mixed data to clients
void UdpServer::sendMixed() {
	if (controller.map_udp_endpoint.empty()) {
		LOG("No ready clients to send mixed data.");
	} else {
		LOG("Prepare and send mixed data.");
		auto mixed_data = controller.mix();
		LOG("Mixed size: " + _(mixed_data.size()));

		int nr = mixed_datagrams_counter, ack, win;

		for (auto it = controller.map_udp_endpoint.begin();
				it != controller.map_udp_endpoint.end(); it++) {

			ack = it->second->getExpectedAck();
			win = it->second->getAllowedWin();

			auto header = "DATA " + _(nr) + " " + _(ack) + " " + _(win) + "\n";
			LOG("Header (nr, ack, win): " + header);

			header += mixed_data;
			auto endpoint = it->first;
			socket_server_udp.send_to(boost::asio::buffer(std::move(header)), endpoint);
		}

		store_mixed_data(mixed_datagrams_counter, mixed_data);
		mixed_datagrams_counter++;
	}
	timer_sound_send.expires_from_now(
			boost::posix_time::millisec(controller.tx_interval));
	timer_sound_send.async_wait(boost::bind(&UdpServer::sendMixed, this));
}

void UdpServer::receiveDatagram() {
	socket_server_udp.async_receive_from(boost::asio::buffer(message_buffer),
			incoming_client_endpoint,
			[this](boost::system::error_code ec, std::size_t size)
			{
				if (!ec) {
					LOG("Receive datagram from client.");
					processClientDatagram(size);
				} else {
					ERR(ec);
				}
				message_buffer.assign(0);
				receiveDatagram();
			});
}

// store mixed by server data needed to retransmissions
void UdpServer::store_mixed_data(uint32_t datagram_nr, std::string mixed_data) {
	mixed_data_storage.insert(mixed_data_storage.end(),
			std::make_pair(datagram_nr, mixed_data));
	if (mixed_data_storage.size() > controller.buffer_len) {
		auto it = mixed_data_storage.begin();
		mixed_data_storage.erase(it);
	}
}

// get stored mixed data during retransmissions
const std::string& UdpServer::get_stored_mixed_data(
		uint32_t datagram_nr) const {
	auto it = mixed_data_storage.find(datagram_nr);
	if (it == mixed_data_storage.end()) {
		ERR("Datagram nr " + _(datagram_nr)+ " does not exists (anymore)");
		throw new DatagramException();
	}
	return it->second;
}

void UdpServer::checkUdpConnections(const boost::system::error_code& error) {
	if (error) {
		ERR(error);
	} else {
		LOG("Checking UDP connections.");
		for (auto it = controller.map_udp_endpoint.begin();
				it != controller.map_udp_endpoint.end();) {
			if (!it->second->isActiveUDP()) {
				INFO("Inactive client UDP connection detected. Removing.");
				auto id_to_remove = it->second->getId();
				it++;
				controller.removeClient(id_to_remove);
			} else {
				it++;
			}
		}
	}

	timer_udp_check.expires_from_now(
			boost::posix_time::seconds(UDP_CHECK_INTERVAL_S));
	timer_udp_check.async_wait(
			boost::bind(&UdpServer::checkUdpConnections, this,
					boost::asio::placeholders::error));
}

// Parsing and processing user datagram. Using global message_buffer and new_client_endpoint
void UdpServer::processClientDatagram(size_t message_size) {

	LOG("Parsing datagram, endpoint: " + endpointToString(incoming_client_endpoint) + ", size: " + _(message_size));
	//LOG("dupa" + incoming_client_endpoint.data()->sa_data );
	char* datagram = message_buffer.c_array();

	size_t nr, client_id;

	if (parser.matches_upload(datagram, message_size, nr, databuffer)) {

		int data_len = strlen(databuffer);
		LOG("UPLOAD nr " + _(nr) + ", data size: " + _(data_len));

		auto uit = controller.map_udp_endpoint.find(incoming_client_endpoint);
		if( uit == controller.map_udp_endpoint.end() ) {
			WARN("No UDP endpoint mapping. Aborting.");
			return;
		}
		auto client = uit->second;

		if( nr != client->getExpectedAck() ) {
			WARN("Wrong datagram nr: " + _(nr) + ", expected:" + _(client->getExpectedAck()) + ". Abort.");
			return;
		}

		if ( message_size > client->getAllowedWin() ) {
			WARN("Too big message size: " + _(message_size) + ", allowed window:" + _(client->getAllowedWin()) + ". Removing client.");
			controller.removeClient(client->getId());
			return;
		}

		client->addData(databuffer, data_len); //also ack++
		//send ACK
		auto datagram = "ACK " + _(client->getExpectedAck()) + " " + _(client->getAllowedWin()) + "\n";
		LOG("ACK send :" + datagram);
		socket_server_udp.send_to(boost::asio::buffer(std::move(datagram)), incoming_client_endpoint);


	} else if (parser.matches_keepalive(datagram)) {

		LOG("KEEPALIVE");
		auto uit = controller.map_udp_endpoint.find(incoming_client_endpoint);
		if (uit == controller.map_udp_endpoint.end()) {
			WARN("No UDP endpoint mapping. Aborting.");
			return;
		}
		uit->second->correctlastUdpTime();


	} else if (parser.matches_retransmit(datagram, nr)) {

		LOG("RETRANSMITE nr >= " + _(nr));
		auto uit = controller.map_udp_endpoint.find(incoming_client_endpoint);
		if ( uit == controller.map_udp_endpoint.end() ) {
			WARN("UDP endpoint mapping exists. Aborting.");
			return;
		}

		auto dit = mixed_data_storage.find(nr);
		if ( dit == mixed_data_storage.end() ) {
			WARN("Client ask to retransmit too old datagram. Aborting");
			return;
		}

		auto ack_win_str = " " + _(uit->second->getExpectedAck()) + " " + _(uit->second->getAllowedWin()) + "\n";

		while ( dit != mixed_data_storage.end() ) {
			auto to_send = "DATA " + _(dit->first) + ack_win_str + dit->second;
			socket_server_udp.send_to(boost::asio::buffer(std::move(to_send)), incoming_client_endpoint);
			dit++;
		}

	} else if (parser.matches_client_id(datagram, client_id)) {

		LOG("CLIENT " + _(client_id));
		// `CLIENT` datagram means, that tcp connection must be already created
		auto tit = controller.clients.find(client_id);
		if (tit == controller.clients.end()) {
			WARN("No TCP id mapping. Aborting.");
			return;
		}

		auto client_ptr = tit->second;
		auto uit = controller.map_udp_endpoint.find(incoming_client_endpoint);

		// check if this is first connection via UDP for client with endpoint
		if( client_ptr->getUdpEndpoint() == incoming_client_endpoint) {
			WARN("Already connected via UDP, endpoint is correct. Aborting.");
			return;
		}

		// check if endpoint mapping does not exist for some other client (with different id)
		if ( uit != controller.map_udp_endpoint.end() ) {
			WARN("UDP endpoint mapping exists. Aborting.");
			return;
		}

		// must be first connection, so no udp_endpoint set required
		/*if( client_ptr->getUdpEndpoint() != nullptr ) {
			WARN("Someone with different endpoint is trying to connect as existing client using the same id. Aborting.");
			return;
		}*/

		controller.map_udp_endpoint.insert(make_pair(incoming_client_endpoint, client_ptr));
		client_ptr->setUdpEndpoint(std::move(incoming_client_endpoint));


	} else {

		WARN("Incorrect datagram schema.");
	}
}
