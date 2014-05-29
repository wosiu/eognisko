/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "TcpServer.hpp"

TcpServer::TcpServer(boost::asio::io_service& io_service,
		ServerController& _controller) :
			controller(_controller),
			acceptor_(io_service, tcp::endpoint(tcp::v6(), _controller.port)),
			socket_(io_service),
			timer_(io_service, boost::posix_time::seconds(REPORT_INTERVAL_S)) {

	INFO("TCP server started.");
	controller.turnOnTcpSever();
	acceptClient();
	timer_.async_wait(boost::bind(&TcpServer::sendReports, this));
}

void TcpServer::sendId(std::shared_ptr<ClientContext> cc) {
	auto id = cc->getId();
	auto datagram = "CLIENT " + _(id) + "\n";
	writeOrRemove(cc, datagram);
}

void TcpServer::acceptClient() {
	acceptor_.async_accept(socket_, [this](boost::system::error_code ec)
	{
		if (!ec) {
			if ( controller.clients.size() > controller.MAX_CLIENT_NO ) {
				socket_.close(ec);
				if (!ec) {
					WARN("Client disconnected by server - too much clients.");
				} else {
					ERR(ec);
				}
			} else {
				// dodaje kleinta, zwraca referencje na kontekst z nim zwiazany
				auto cc = controller.addClient(std::move(socket_));
				sendId(cc);
				INFO("Client joined, sending id: " + _(cc->getId()) );
			}
		} else {
			ERR(ec);
		}

		acceptClient();
	});
}

void TcpServer::sendReports() {
	// Note, that sending reports will start when first client come
	if (controller.map_udp_endpoint.empty()) {
		LOG("No clients to send report.");
	} else {

		std::stringstream report("\n");
		boost::system::error_code ec;
		auto fifo_size = boost::lexical_cast<std::string>(controller.fifo_size);

		for (auto it = controller.clients.begin();
				it != controller.clients.end(); ) {
			auto endpoint = it->second->getTcpSocket().remote_endpoint(ec);
			if (ec) {
				INFO(ec);
				auto id_to_rm = it->second->getId();
				it++;
				controller.removeClient(id_to_rm);
				continue;
			}
			report << boost::lexical_cast<std::string>(endpoint) << " FIFO: "
					<< it->second->getFIFOSize() << "/" << fifo_size << " (min. "
					<< it->second->getMixFifoLastMin() << ", max. " << it->second->getMixFifoLastMax()
					<< ")\n";
			it++;
		}

		std::string rendered = report.str();
		LOG("Sending report: " + rendered);

		for (auto it = controller.clients.begin();
				it != controller.clients.end(); it++) {
			it->second->resetDataStatus();
			writeOrRemove(it->second, rendered);
		}
	}
	timer_.expires_from_now(boost::posix_time::seconds(REPORT_INTERVAL_S));
	timer_.async_wait([this](boost::system::error_code ec) {
		if (ec) {
			ERR(ec);
		}
		// No matter what - repeat
			sendReports();
		});

	//boost::bind(&TcpServer::send_reports_datagrams, this));
}

void TcpServer::writeOrRemove(std::shared_ptr<ClientContext> cc,
		const std::string& datagram) {
	auto id = cc->getId();
	boost::asio::async_write(cc->getTcpSocket(), boost::asio::buffer(datagram),
			[this, id](boost::system::error_code ec, std::size_t transferred) {
				if (ec) {
					INFO(ec);
					controller.removeClient(id);
				} else {
					LOG("Datagram sent to client, id: " + _(id));
				}
			});
}
