/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef COMMONS_HPP_
#define COMMONS_HPP_

#include <iostream>
#include <boost/asio.hpp>

#define ERR(ec) std::cerr << "[Error] " << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ <<  ": " << ec << std::endl
#define LOG(log) std::cerr << "[Log] " << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ <<  ": " << log << std::endl

#endif /* COMMONS_HPP_ */
