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
#include <vector>
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
private:
	const unsigned BUFFER_SIZE = 66000;
	const unsigned MAX_UP_SIZE = 880 * 10;
	const unsigned KEEPALIVE_INTERVAL_MS = (IS_DEB) ? 2500 : 100;
	const size_t ACTIVITY_CHECK_INTERVAL_S = (IS_DEB) ? 5 : 1;

private:
	unsigned id;
	ClientController& controller;
	ip::tcp::socket tcp_socket;
	ip::udp::socket udp_socket;
	ip::tcp::endpoint tcp_server_endpoint;
	ip::udp::endpoint udp_server_endpoint, udp_recv_endpoint;

	DatagramParser parser;

	boost::asio::streambuf tcp_buffer;
	char buffer_chararray[65535];
	boost::array<char, 65535> buffer_boostarray;
	std::vector<char> input_buffer;
	std::vector<char> last_upload;
	std::list<std::vector<char> > pending_datagrams;

	boost::asio::posix::stream_descriptor stdin;
	boost::asio::posix::stream_descriptor stdout;

	size_t expected_data_num, max_seen_data, upload_num, available_win, data_count;
	bool reading;
	bool is_active;
	//bool first_msg;

private:
	boost::asio::deadline_timer keepalive_timer, activity_timer;
	// boost::asio::steady_timer //if something wrong try steady instead deadline..

private:
	void receiveID();
	void readDatagram();
	void sendDatagram(std::vector<char> msg);
	void read_and_send();

private:
	void connect(const boost::system::error_code &ec);
	void on_tcp_read(const boost::system::error_code &ec,
			size_t bytes_transferred);
	void readDatagramHandler(const boost::system::error_code &ec,
			size_t bytes_transferred);
	void on_datagram_sent(const boost::system::error_code &ec,
			size_t bytes_transferred);
	void on_input_read(const boost::system::error_code &ec,
			size_t bytes_transferred);
	void sendKeepalive();
	void checkActivity();

public:
	Client(boost::asio::io_service& io_service, ClientController& controller);
};

#endif /* CLIENT_HPP_ */
