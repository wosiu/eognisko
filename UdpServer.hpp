/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef UDP_SERVER_HPP_
#define UDP_SERVER_HPP_

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "ServerController.hpp"
#include "ClientContext.hpp"
#include "DatagramParser.hpp"
#include "commons.hpp"

using boost::asio::ip::udp;

class UdpServer {
public:
	UdpServer(boost::asio::io_service& _io_service, ServerController& _controller);
	typedef std::pair<udp::endpoint, std::string > message;

private:
	ServerController& controller;
	boost::asio::deadline_timer timer_sound_send;
	boost::asio::deadline_timer timer_udp_check;
	udp::socket socket_server_udp;
	udp::endpoint incoming_client_endpoint;

	const uint16_t UDP_CHECK_INTERVAL_S = 1; //unused in debug mode

	size_t mixed_datagrams_counter;

    char databuffer[65535];
    boost::array<char, 65535> message_buffer;
	std::map<size_t, std::string> mixed_data_storage;
	std::list<message> pending_datagrams;

	DatagramParser parser;

    const std::string& get_stored_mixed_data(uint32_t datagram_nr) const;

    void store_mixed_data(uint32_t datagram_nr, std::string mixed_data);
	void processingMixed();
	void receiveDatagram();
	void processClientDatagram(size_t message_size);
	void checkUdpConnections(const boost::system::error_code& error);

	void sendDatagram(std::string msg, udp::endpoint& udp_client_endpoint);
	void cyclicDatagramSend(const boost::system::error_code& ec, size_t datagram_size);
};


class DatagramException: public std::exception
{
  virtual const char* what() const throw(){
    return "Datagram does not exist.";
  }
};

#endif /* UDP_SERVER_HPP_ */
