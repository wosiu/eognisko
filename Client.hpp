/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <list>
#include <string>
#include <chrono>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>
#include "DatagramParser.hpp"
#include "commons.hpp"

namespace ip = boost::asio::ip;

class ClientController {
public:
	ClientController() :
			port(10000 + 336071 % 10000), retransmit_limit(10) {
	}
	std::string host;
	uint16_t port;
	uint16_t retransmit_limit;

};

class TroublesomeConnection: public std::exception
{
	const char* what() const noexcept
	{
		return "Troublesome connection.";
	}
};

class Client
{
public:
	Client(boost::asio::io_service& io_service, ClientController& controller);

private:
	// Constants
	const size_t KEEPALIVE_INTERVAL_MS = (IS_DEB) ? 2500 : 100;
	const size_t ACTIVITY_CHECK_INTERVAL_S = (IS_DEB) ? 5 : 1;

	// Client context
	size_t id;
	ClientController& controller;
	DatagramParser parser;

	// Sockets
	ip::tcp::socket tcp_socket;
	ip::udp::socket udp_socket;
	ip::tcp::endpoint tcp_server_endpoint;
	ip::udp::endpoint udp_server_endpoint, udp_recv_endpoint;

	// Buffers
	boost::asio::streambuf tcp_buffer;
	char buffer_chararray[65535];
	boost::array<char, 65535> buffer_boostarray;
	boost::array<char, 8800> stdin_buffer;
	std::list<std::string> pending_datagrams;

	// Timers
	boost::asio::deadline_timer keepalive_timer;
	boost::asio::deadline_timer activity_timer;

	// std I/O descriptors
	boost::asio::posix::stream_descriptor stdin;
	boost::asio::posix::stream_descriptor stdout;

	size_t expected_data_num, max_seen_data, upload_num, available_win,
			data_count;
	std::string last_upload;
	bool reading;
	bool is_active;

	// Functions:
	void receiveID();
	void readDatagram();
	void cyclicReadReports();
	void cylicSendKeepalive();
	void cylicCheckActivity();
	void sendDatagram(std::string msg);
	void readStdInput();
	bool equal(ip::udp::endpoint a, ip::udp::endpoint b) const;

	// Binded async handlers
	void connect(const boost::system::error_code &ec);
	void processDatagram(const boost::system::error_code &ec,
			size_t bytes_transferred);
	void cyclicDatagramSend(const boost::system::error_code &ec,
			size_t bytes_transferred);
	void sendStdinInput(const boost::system::error_code &ec,
			size_t bytes_transferred);
};

#endif /* CLIENT_HPP_ */
