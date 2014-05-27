/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef COMMONS_HPP_
#define COMMONS_HPP_

#include <string>
#include <iostream>
#include <boost/asio.hpp>

#define IS_ERR 1
#define IS_WARN 1
#define IS_INFO 1
#define IS_LOG 0
#define IS_DEB 0

// Problem with connection or something wrong happen in code
#define ERR(ec) if(IS_ERR){ std::cerr << "[Error] " << __FILE__ << ", " << __FUNCTION__ << ", at " << __LINE__ <<  ": " << ec << std::endl; }
// Warnings, e.g. tricky clients connections
#define WARN(ec) if(IS_WARN){ std::cerr << "[Warning] " << __FILE__ << ", " << __FUNCTION__ << ", at " << __LINE__ <<  ": " << ec << std::endl; }
// Info, mostly related with client events
#define INFO(log) if(IS_INFO){ std::cerr << "[Info] " << __FILE__ << ", " << __FUNCTION__ << ", at " << __LINE__ <<  ": " << log << std::endl; }
// Others events with the lowest priority, which produce logs very fast, e.g. datagrams debug
#define LOG(log) if(IS_LOG){ std::cerr << "[Log] " << __FILE__ << ", " << __FUNCTION__ << ", at " << __LINE__ <<  ": " << log << std::endl; }
// Debug
#define DEB(var) if(IS_DEB){ std::cerr << "[Debug] " << __FILE__ << ", " << __FUNCTION__ << ", at " << __LINE__ <<  ": " << #var << ": " << var << std::endl; }


#define _(var) std::to_string(var)

std::string endpointToString(boost::asio::ip::udp::endpoint udp_endpoint);
std::string endpointToString(boost::asio::ip::tcp::endpoint udp_endpoint);



#endif /* COMMONS_HPP_ */
