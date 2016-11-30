/*
 * configuration.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: vermosen
 */

#ifndef APPLICATION_CONFIGURATION_HPP_
#define APPLICATION_CONFIGURATION_HPP_

#include <fstream>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/export.hpp>

namespace wotan
{
	class configuration
	{
	private:
		friend class boost::serialization::access;
		template<class Archive> void serialize(
				Archive & ar,
				const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(gatewayLocation_);
			ar & BOOST_SERIALIZATION_NVP(b);
			ar & BOOST_SERIALIZATION_NVP(c);
		}

		std::string gatewayLocation_;
		int b;
		int c;
	public:
		configuration();
		virtual ~configuration();
		configuration(std::string gatewayLocation, int bb, int cc);

		std::string gatewayLocation() { return gatewayLocation_; }
	};
} /* namespace wotan */

#endif /* APPLICATION_CONFIGURATION_HPP_ */
