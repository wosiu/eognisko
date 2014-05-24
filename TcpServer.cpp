/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "TcpServer.hpp"

TcpServer::TcpServer(boost::asio::io_service& io_service,
		ServerController& _controller) :
		controller(_controller), acceptor_(io_service,
				tcp::endpoint(tcp::v6(), _controller.port)), socket_(
				io_service), timer_(io_service, boost::posix_time::seconds(1)) {
	LOG("TCP server started.");
	do_accept();
	timer_.expires_from_now(boost::posix_time::seconds(REPORT_INTERVAL_S));
	timer_.async_wait(boost::bind(&TcpServer::send_reports_datagrams, this));

}

void TcpServer::send_id_datagram(std::shared_ptr<ClientContext> cc) {
	auto id = cc->getId();
	auto datagram = "CLIENT " + std::to_string(id) + "\n";
	write_or_remove(cc, datagram);
}

void TcpServer::do_accept() {
	acceptor_.async_accept(socket_, [this](boost::system::error_code ec)
	{
		if (!ec) {
			// dodaje kleinta, zwraca referencje na kontekst z nim zwiazany
			try {
				auto cc = controller.addClient(std::move(socket_)); //throwable
				send_id_datagram(cc);
				LOG("Client accepted, id: " + std::to_string(cc->getId()) );
				//TODO bad idea, beacouse of come and quit many times in small period of time
				// then many send reports event would be queued in to short time
				//remeber last report time and something..
				//if it is a first client, start sending reports
				/*if ( controller.clients.size() == 1 ) {
				 LOG("First client. Start sending reports..");
				 timer_.expires_from_now(boost::posix_time::seconds(REPORT_INTERVAL_S))
				 timer_.async_wait(boost::bind(&TcpServer::send_reports_datagrams, this));
				 }*/
			} catch (const std::exception &e) {
				LOG(e.what());
			}
		} else {
			ERR(ec);
		}

		do_accept();
	});
}

void TcpServer::send_reports_datagrams() {
	// Note, that sending reports will start when first client come
	if (controller.clients.empty()) {
		LOG("No clients to send report.");
	} else {

		std::stringstream report("\n");
		boost::system::error_code ec;
		auto fifo_size = boost::lexical_cast<std::string>(controller.fifo_size);

		for (auto it = controller.clients.begin();
				it != controller.clients.end(); ) {
			auto endpoint = it->second->getTcpSocket().remote_endpoint(ec);
			if (ec) {
				LOG(ec);
				auto id_to_rm = it->second->getId();
				it++;
				controller.removeClient(id_to_rm);
				continue;
			}
			report << boost::lexical_cast<std::string>(endpoint) << " FIFO: "
					<< it->second->getFIFOSize() << "/" << fifo_size << "(min. "
					<< it->second->last_min << ", max. " << it->second->last_max
					<< ")\n";
			it++;
		}

		std::string rendered = report.str();
		//LOG("Sending report to clients: " + rendered);

		for (auto it = controller.clients.begin();
				it != controller.clients.end(); it++) {
			write_or_remove(it->second, rendered);
		}
	}
	timer_.expires_from_now(boost::posix_time::seconds(REPORT_INTERVAL_S));
	timer_.async_wait([this](boost::system::error_code ec) {
		if (ec) {
			ERR(ec);
		}
		// No matter what - repeat
			send_reports_datagrams();
		});

	//boost::bind(&TcpServer::send_reports_datagrams, this));
}

void TcpServer::write_or_remove(std::shared_ptr<ClientContext> cc,
		const std::string& datagram) {
	auto id = cc->getId();
	boost::asio::async_write(cc->getTcpSocket(), boost::asio::buffer(datagram),
			[this, id](boost::system::error_code ec, std::size_t transferred) {
				if (ec) {
					LOG(ec);
					controller.removeClient(id);
				} else {
					LOG("Datagram sent to client, id: " + std::to_string(id) + ".");
					// + ((datagram.size() < 100) ? datagram : (std::string(datagram, 100)) + "...")
				}
			});
}
