/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "Client.hpp"
#include "packet_header.h"

Client::Client(boost::asio::io_service& io_service, ClientController& _controller) :
		controller(_controller), tcp_socket(io_service),
		udp_socket(io_service, ip::udp::endpoint(ip::udp::v4(), 0)),
		input_buffer(MAX_UP_SIZE),
		stdin(io_service, dup(STDIN_FILENO)), stdout(io_service, dup(STDOUT_FILENO)),
		keepalive_timer(io_service), activity_timer(io_service) {

	INFO("Client started.");

	data_count = max_seen_data = upload_num = available_win = expected_data_num = 0;
	is_active = true;
	//todo first_msg =
	reading = false;

	// Resolving endpoints (throwable)
	ip::tcp::resolver tcp_resolver(io_service);
	ip::tcp::resolver::query query_tcp(_controller.host, _(_controller.port),
			ip::resolver_query_base::flags());
	tcp_server_endpoint = *(tcp_resolver.resolve(query_tcp));

	ip::udp::resolver udp_resolver(io_service);
	ip::udp::resolver::query query_udp(_controller.host, _(_controller.port),
			ip::resolver_query_base::flags());
	udp_server_endpoint = *(udp_resolver.resolve(query_udp));

	LOG("Endpoints resolved, UDP: " + endpointToString(udp_server_endpoint) + ", TCP: " + endpointToString(tcp_server_endpoint));

	// TCP connect
	tcp_socket.async_connect(tcp_server_endpoint,
			std::bind(&Client::connect, this, std::placeholders::_1));
}

void Client::connect(const boost::system::error_code& ec) {
	if (!ec) {
		INFO("TCP connected.");

		receiveID(); //blocking
		sendKeepalive();
		readDatagram();
		checkActivity();

	} else {
		ERR(ec);
		throw TroublesomeConnection();
	}
}


void Client::receiveID() {
	LOG("Waiting for client id from server via TCP.");

	tcp_socket.receive(boost::asio::buffer(buffer_chararray, 20)); // throwable
	size_t client_id;
	if ( !parser.matches_client_id(buffer_chararray, client_id) ) {
		ERR("Wrong CLIENT datagram schema.")
		throw TroublesomeConnection();
	}
	size_t s = strlen(buffer_chararray);
	INFO("My id: " + _(client_id) + ". Re-send to server my id via UDP.");
	udp_socket.send_to(boost::asio::buffer(buffer_chararray, s), udp_server_endpoint);
}


void Client::sendKeepalive() {
	udp_socket.send_to(boost::asio::buffer("KEEPALIVE\n"), udp_server_endpoint);
	LOG("KEEPALIVE sent.");

	keepalive_timer.expires_from_now(boost::posix_time::milliseconds(KEEPALIVE_INTERVAL_MS));
	keepalive_timer.async_wait(
			[this](boost::system::error_code ec) {
					if (ec) {
						ERR(ec);
					}
					sendKeepalive();
					});
}

void Client::checkActivity() {
	LOG("Activity check: " + _(is_active));
	if (!is_active && !IS_DEB) {
		WARN("No activity was observed. Stopping.")
		throw TroublesomeConnection();
	}

	is_active = false;
	activity_timer.expires_from_now(
			boost::posix_time::seconds(ACTIVITY_CHECK_INTERVAL_S));
	activity_timer.async_wait(
			[this](boost::system::error_code ec) {
				if (ec) {
					ERR(ec);
				}
				checkActivity();
			});
}

void Client::readDatagram() {
	buffer_boostarray.assign(0);
	udp_socket.async_receive_from(boost::asio::buffer(buffer_boostarray),
			udp_recv_endpoint,
			std::bind(&Client::readDatagramHandler, this, std::placeholders::_1,
					std::placeholders::_2));
}

void Client::sendDatagram(std::vector<char> msg) {
	bool can_send = pending_datagrams.empty();
	pending_datagrams.push_front(std::move(msg));

	if (can_send) {
		udp_socket.async_send_to(boost::asio::buffer(pending_datagrams.front()),
				udp_server_endpoint,
				std::bind(&Client::on_datagram_sent, this,
						std::placeholders::_1, std::placeholders::_2));
	}
}

void Client::read_and_send() {
	if (available_win == 0)
		return;
	reading = true;
	boost::asio::async_read(
	stdin,
			boost::asio::buffer(input_buffer,
					std::min(input_buffer.size(), available_win)),
			std::bind(&Client::on_input_read, this, std::placeholders::_1,
					std::placeholders::_2));
}

void Client::on_input_read(const boost::system::error_code &ec,
		size_t bytes_transferred) {
	if (!ec) {
		std::vector<char> msg(bytes_transferred + 100);
		PacketHeader h(PacketHeader::Type::UPLOAD, upload_num);
		h.build(msg.data());
		memcpy(msg.data() + h.get_header_size(), input_buffer.data(),
				bytes_transferred);
		msg.resize(h.get_header_size() + bytes_transferred);
		sendDatagram(msg);
		upload_num++;
		last_upload = std::move(msg);
	} else
		std::cerr << "[Client] " << ec << std::endl;
	reading = false;
	data_count = 0;
}

void Client::readDatagramHandler(const boost::system::error_code& ec,
		size_t datagram_size) {

	size_t ack, nr, new_win, data_size;

	if ( ec ) {
		ERR(ec);

	//TODO to mala function
	} else if( udp_recv_endpoint != udp_server_endpoint) {
		WARN("Data received from an unknown server: " + endpointToString(udp_recv_endpoint) );

	} else if ( parser.matches_ack(buffer_boostarray.data(), ack, new_win) ) {
		LOG("ACK: ack: " + _(ack) + " win: " + _(new_win));
		is_active = true;
		if (ack == upload_num) {
			data_count = 0;
			available_win = new_win;
			if (!reading) {
				read_and_send();
			}
		}

	} else if ( parser.matches_data(buffer_boostarray.data(), datagram_size, nr,  ack, new_win, buffer_chararray, data_size) ) {
			if ( nr == expected_data_num || expected_data_num == 0
					|| nr - controller.retransmit_limit > expected_data_num) {
				data_count++;
				if (expected_data_num == 0 || ack == upload_num) {
					data_count = 0;
					available_win = new_win;
					if (!reading) {
						read_and_send();
					}
				}
				try {
					boost::asio::write(stdout, boost::asio::buffer(buffer_chararray, data_size));
				} catch (std::exception &e) {
					ERR(e.what());
				}
				expected_data_num = nr + 1;

			} else if (nr > expected_data_num
					&& nr - controller.retransmit_limit <= expected_data_num
					&& nr > max_seen_data) {

				std::vector<char> msg(100);
				PacketHeader h(PacketHeader::Type::RETRANSMIT,
						expected_data_num);
				h.build(msg.data());
				msg.resize(h.get_header_size());
				sendDatagram(std::move(msg));
			}
			if (data_count == 2) {
				sendDatagram(last_upload);
			}
			max_seen_data = std::max(max_seen_data, nr);
		} else {
			WARN("Wrong datagram schema.");
			ERR( buffer_boostarray.data() );
			exit(0);
		}

	readDatagram();
}

void Client::on_datagram_sent(const boost::system::error_code& ec,
		size_t datagram_size) {
	if (!ec) {
		pending_datagrams.pop_front();
		if (!pending_datagrams.empty()) {
			udp_socket.async_send_to(
					boost::asio::buffer(pending_datagrams.front()),
					udp_server_endpoint,
					std::bind(&Client::on_datagram_sent, this,
							std::placeholders::_1, std::placeholders::_2));
		}
	} else {
		std::cerr << "[Client] Error " << ec << " when sending datagram"
				<< std::endl;
	}
}

void Client::on_tcp_read(const boost::system::error_code& ec,
		size_t datagram_size) {
	if (!ec) {
		std::istream is(&tcp_buffer);
		std::string s;
		std::getline(is, s);
		std::cerr << s << std::endl;
		boost::asio::async_read_until(tcp_socket, tcp_buffer, '\n',
				std::bind(&Client::on_tcp_read, this, std::placeholders::_1,
						std::placeholders::_2));
	} else {
		throw TroublesomeConnection();
	}
}




