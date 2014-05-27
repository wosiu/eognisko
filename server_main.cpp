/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include <memory>
#include <boost/program_options.hpp>
#include "TcpServer.hpp"
#include "UdpServer.hpp"
#include "ServerController.hpp"
#include "commons.hpp"

using namespace boost::program_options;

int main(int argc, char **argv) {

	ServerController controller;
	boost::asio::io_service io_service;

	//Parsowanie argumentow
	options_description desc("Allowed options");
	desc.add_options()("help", "help view")(",p",
			value<uint16_t>(&controller.port)->default_value(
					controller.port), "port number")(",F",
			value<uint16_t>(&controller.fifo_size)->default_value(controller.fifo_size),
			"FIFO size for client")(",L",
			value<uint16_t>(&controller.low_mark)->default_value(controller.low_mark),
			"low watermark")(",H", value<uint16_t>(), "high watermark")(",X",
			value<uint16_t>(&controller.buffer_len)->default_value(controller.buffer_len),
			"mixed data buffer length - retransmission limit")(",i",
			value<uint16_t>(&controller.tx_interval)->default_value(controller.tx_interval),
			"tx interval");

	variables_map vm;
	try {
		store(parse_command_line(argc, argv, desc), vm);
	} catch (boost::program_options::unknown_option& e) {
		std::cout << "Abort, " << e.what() << "\n";
		return 0;
	}
	notify(vm);

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 0;
	}

	if (vm.count("-H"))
		controller.high_mark = vm["-H"].as<uint16_t>();
	else
		controller.high_mark = controller.fifo_size;

	if (IS_DEB) {
		controller.high_mark = 4;
	}

	//Praca
	TcpServer tcp_server(io_service, controller);
	UdpServer udp_server(io_service, controller);

	io_service.run();

	return 0;
}
