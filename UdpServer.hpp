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
#include "commons.hpp"

using boost::asio::ip::udp;

class UdpServer {
public:
	UdpServer(boost::asio::io_service& _io_service, ServerController& _controller);
	// storeMixerOut(void* ...)

private:
	uint32_t mixed_datagrams_counter;
	boost::asio::deadline_timer timer_sound_send, timer_udp_check;
	const static uint16_t UDP_CHECK_INTERVAL_S = 1;
	ServerController& controller;
	udp::socket socket_server_udp;
    udp::endpoint new_client_endpoint;
    boost::array<char, 65535> message_buffer;
	void sendMixed();
	void receiveDatagram();
	void processClientDatagram(size_t message_size);
	void checkUdpConnections(const boost::system::error_code& error);
	// mixerout_storage deque

};



#endif /* UDP_SERVER_HPP_ */
