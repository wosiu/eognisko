/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#include "ClientContext.hpp"

ClientContext::ClientContext(uint32_t id, tcp::socket tcp_socket) : id(id), tcp_socket(std::move(tcp_socket)){
	last_min = last_max = 0;
	tcp_endpoint_copy = this->tcp_socket.remote_endpoint(); //throwable!
}
