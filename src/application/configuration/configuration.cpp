/*
 * configuration.cpp
 *
 *  Created on: Sep 28, 2016
 *      Author: vermosen
 */
#include "StdAfx.h"

#include <application/configuration/configuration.hpp>

namespace wotan
{
	configuration::configuration()
	{
		// TODO Auto-generated constructor stub

	}

	configuration::configuration(std::string gatewayLocation, int bb, int cc)
		: gatewayLocation_(gatewayLocation), b(bb), c(cc)
	{
		// TODO Auto-generated constructor stub
	}

	configuration::~configuration()
	{
		// TODO Auto-generated destructor stub
	}
} /* namespace wotan */
