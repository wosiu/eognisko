/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "Client.hpp"

Client::Client(boost::asio::io_service& io_service, ClientController& _controller) :
		controller(_controller), tcp_socket(io_service),
		udp_socket(io_service, ip::udp::endpoint(ip::udp::v4(), 0)),
		stdin(io_service, dup(STDIN_FILENO)), stdout(io_service, dup(STDOUT_FILENO)),
		keepalive_timer(io_service), activity_timer(io_service) {

	INFO("Client started.");

	data_count = max_seen_data = upload_num = available_win = expected_data_num = 0;
	is_active = true;
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
	tcp_socket.connect(tcp_server_endpoint); //blocking, throwable

	INFO("TCP connected.");

	receiveID(); //blocking, throwable
	cylicSendKeepalive();
	readDatagram();
	cyclicReadReports();
	cylicCheckActivity();
}


void Client::receiveID() {
	LOG("Waiting for client id from server via TCP.");

	buffer_boostarray.assign(0);
	auto s = tcp_socket.receive(boost::asio::buffer(buffer_boostarray, 20)); // throwable
	size_t client_id;
	if ( !parser.matches_client_id(buffer_boostarray.data(), client_id) ) {
		ERR("Wrong CLIENT datagram schema.")
		throw TroublesomeConnection();
	}
	INFO("My id: " + _(client_id) + ". Re-send to server my id via UDP.");
	udp_socket.send_to(boost::asio::buffer(buffer_boostarray, s), udp_server_endpoint); //blocking
}


void Client::cylicSendKeepalive() {
	LOG("KEEPALIVE datagram processing.");

	sendDatagram("KEEPALIVE\n");

	keepalive_timer.expires_from_now(boost::posix_time::milliseconds(KEEPALIVE_INTERVAL_MS));
	keepalive_timer.async_wait(
			[this](boost::system::error_code ec) {
					if (ec) {
						ERR(ec);
					}
					cylicSendKeepalive();
					});
}


void Client::cylicCheckActivity() {
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
				cylicCheckActivity();
			});
}



void Client::sendDatagram(std::string msg) {
	// sync, blocking version:
	// udp_socket.send_to(boost::asio::buffer(std::move(msg)), udp_server_endpoint);

	// async, nonblocking with datagrams ready to send queue:
	bool can_send = pending_datagrams.empty();
	pending_datagrams.push_back(std::move(msg));

	if (can_send) {
		udp_socket.async_send_to(boost::asio::buffer(pending_datagrams.front()),
				udp_server_endpoint,
				std::bind(&Client::cyclicDatagramSend, this,
						std::placeholders::_1, std::placeholders::_2));
	} else {
		LOG("Data queued to send");
	}
}


void Client::cyclicDatagramSend(const boost::system::error_code& ec,
		size_t datagram_size) {

	if (!ec) {
		LOG("Sending data via UDP");
		pending_datagrams.pop_front();
		if (!pending_datagrams.empty()) {
			udp_socket.async_send_to(
					boost::asio::buffer(std::move(pending_datagrams.front())),
					udp_server_endpoint,
					std::bind(&Client::cyclicDatagramSend, this,
							std::placeholders::_1, std::placeholders::_2));
		}
	} else {
		ERR(ec);
	}
}


void Client::readStdInput() {

	if (available_win == 0) {
		LOG("No window available. Abort reading from stdin.");
		return;
	}

	LOG("Reading from stdin.");
	reading = true;
	boost::asio::async_read(
			stdin,
			boost::asio::buffer(stdin_buffer,
					std::min(stdin_buffer.size(), available_win)),
			std::bind(&Client::sendStdinInput, this, std::placeholders::_1,
					std::placeholders::_2));
}


void Client::sendStdinInput(const boost::system::error_code &ec,
		size_t bytes_transferred) {

	if (!ec) {
		LOG("Send stdin");
		auto data = stdin_buffer.data();
		auto datagram = "UPLOAD " + _(upload_num) + "\n" + std::string (data, data + bytes_transferred);
		sendDatagram(datagram);
		upload_num++;
		last_upload = std::move(datagram);
	} else {
		ERR(ec);
	}
	reading = false;
	data_count = 0;
}


void Client::readDatagram() {
	buffer_boostarray.assign(0);
	udp_socket.async_receive_from(boost::asio::buffer(buffer_boostarray),
			udp_recv_endpoint,
			std::bind(&Client::processDatagram, this, std::placeholders::_1,
					std::placeholders::_2));
}


void Client::processDatagram(const boost::system::error_code& ec,
		size_t datagram_size) {

	LOG("Datagram via UDP read");
	size_t ack, nr, new_win, data_size;

	if (ec) {
		ERR(ec);

		//TODO to mala function
	} else if (udp_recv_endpoint != udp_server_endpoint) {
		WARN("Data received from an unknown server: " + endpointToString(udp_recv_endpoint));

	} else if (parser.matches_ack(buffer_boostarray.data(), ack, new_win)) {
		LOG("ACK: ack: " + _(ack) + " win: " + _(new_win));
		is_active = true;
		if (ack == upload_num) {
			data_count = 0;
			available_win = new_win;
			if (!reading) {
				readStdInput();
			}
		}

	} else if (parser.matches_data(buffer_boostarray.data(), datagram_size, nr,
			ack, new_win, buffer_chararray, data_size)) {
		LOG("Data (nr, ack, win) " + _(nr) + " " + _(ack) + " " + _(new_win));
		if (nr == expected_data_num || expected_data_num == 0
				|| nr - controller.retransmit_limit > expected_data_num) {
			data_count++;
			if (expected_data_num == 0 || ack == upload_num) {
				data_count = 0;
				available_win = new_win;
				if (!reading) {
					readStdInput();
				}
			}
			try {
				LOG("Write mixed data to stdout");
				boost::asio::write(stdout,
						boost::asio::buffer(buffer_chararray, data_size));
			} catch (std::exception &e) {
				ERR(e.what());
			}
			expected_data_num = nr + 1;

		} else if (nr > expected_data_num
				&& nr - controller.retransmit_limit <= expected_data_num
				&& nr > max_seen_data) {

			auto datagram = "RETRANSMIT " + _(expected_data_num) + "\n";
			INFO("RETRNASMIT ask to server");
			sendDatagram(datagram);
		}
		if (data_count == 2) {
			INFO("Re-send last DATA datagram");
			sendDatagram(last_upload);
		}
		max_seen_data = std::max(max_seen_data, nr);
	} else {
		WARN("Wrong datagram schema.");
	}

	readDatagram();
}


void Client::cyclicReadReports() {
	boost::asio::async_read_until(tcp_socket, tcp_buffer, '\n',
			[this](boost::system::error_code ec, std::size_t datagram_size) {
				if (ec) {
					ERR(ec);
					throw TroublesomeConnection();
				}
				LOG("Read report from server via TCP.");
				std::istream is(&tcp_buffer);
				std::string s;
				std::getline(is, s);
				std::cerr << "[RAPORT] " << s << std::endl;
				cyclicReadReports();
			});
}
