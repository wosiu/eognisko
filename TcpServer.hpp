/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef TCP_SERVER_HPP_
#define TCP_SERVER_HPP_

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "ServerController.hpp"
#include "ClientContext.hpp"
#include "commons.hpp"

using boost::asio::ip::tcp;

class TcpServer {
public:
	TcpServer(boost::asio::io_service& _io_service, ServerController& _controller);
private:
	ServerController& controller;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	boost::asio::deadline_timer timer_;
	static const uint16_t REPORT_INTERVAL_S = 1;

	void do_accept();
	void send_reports_datagrams();
	void send_id_datagram(std::shared_ptr<ClientContext> cc);
	void write_or_remove(std::shared_ptr<ClientContext> cc, const std::string& datagram);
};

#endif /* TCP_SERVER_HPP_ */
