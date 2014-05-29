/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include <boost/program_options.hpp>
#include "Client.hpp"
#include "commons.hpp"

using namespace boost::program_options;


int main(int argc, char** argv)
{
	ClientController controller;

	// Parsing arguments
	options_description desc("Allowed options");
	desc.add_options()("help", "help view")
			(",p",value<uint16_t>(&controller.port)->default_value(controller.port), "port number")
			(",s", value<std::string> (&controller.host)->required(), "server name/address")
			(",X", value<uint16_t>(&controller.retransmit_limit)->default_value(controller.retransmit_limit), "retransmission limit");

	variables_map vm;
	try {
		store(parse_command_line(argc, argv, desc), vm);
		notify(vm);
	} catch (const std::exception& e) {
		std::cout << "Abort, " << e.what() << "\n";
		return 0;
	}

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 0;
	}

	// Run client
	while(true)
	{
		try
		{
			boost::asio::io_service io_service;
			Client c(io_service, controller);
			io_service.run();
		}
		catch(std::exception &e)
		{
			WARN(e.what());
			INFO("Reconnecting in 1 sec..");
		}
		sleep(1);
	}
	return 0;
}

