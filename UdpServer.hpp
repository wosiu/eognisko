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
	// storeMixerOut(void* ...)

private:
	uint32_t mixed_datagrams_counter;
	boost::asio::deadline_timer timer_sound_send, timer_udp_check;
	const static uint16_t UDP_CHECK_INTERVAL_S = 15; //TODO
	ServerController& controller;
	udp::socket socket_server_udp;
    udp::endpoint incoming_client_endpoint;
    boost::array<char, 65535> message_buffer;
	char databuffer[65535];
    void store_mixed_data(uint32_t datagram_nr, std::string mixed_data);
    const std::string& get_stored_mixed_data(uint32_t datagram_nr) const;
	void sendMixed();
	void receiveDatagram();
	void processClientDatagram(size_t message_size);
	void checkUdpConnections(const boost::system::error_code& error);
	std::map<int, std::string> mixed_data_storage;
	DatagramParser parser;
};

class DatagramException: public std::exception
{
  virtual const char* what() const throw(){
    return "Datagram does not exist.";
  }
};

#endif /* UDP_SERVER_HPP_ */
