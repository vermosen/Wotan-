//============================================================================
// Name        : Wotan++.cpp
// Author      : Jean-Mathieu Vermosen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "StdAfx.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <fstream>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "application/logger/logger.hpp"
#include "application/configuration/configuration.hpp"
#include "interactiveBroker/client/client.hpp"


wotan::ib::client cli_;

int main(int argc, char** argv)
{
	wotan::logger::setLogger("wotan_%Y%m%d_%3N.log", wotan::logger::Info);

	LOG_INFO() 		<< "this is a information message from parent process";
	LOG_WARNING() 	<< "this is a warning message from parent process";
	LOG_ERROR() 	<< "this is an error message from parent process";

	try
	{
		//if(int err = detachService())
		//{
		//	return err;
		//}

		// write the configuration file
		LOG_INFO() << "creating new configuration file";
		std::ofstream ofs("tmp/configuration.xml");
		wotan::configuration * test = new wotan::configuration("foo", 1, 2);
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(test);
		ofs.close();

		// read the configuration file
	    wotan::configuration * config_ = nullptr;
	    {
	        // create and open an archive for input
	        std::ifstream ifs("tmp/configuration.xml");

	        if (ifs.good() || !ifs.bad() || ifs.is_open())
	        {
		        boost::archive::xml_iarchive ia(ifs);
		        // read class state from archive
		        ia >> boost::serialization::make_nvp("wotan::configuration", config_);
		        // archive and stream closed when destructors are called
	        }
	    }

	    LOG_INFO() << "Successfully retrieved configuration file !";

	    // start the client

		while(/*cli_.isConnected()*/true)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(10000));
			LOG_INFO() << "sleep 10 seconds";
		}
	}
	catch(std::exception & err)
	{
		LOG_ERROR() << err.what();
		exit(EXIT_FAILURE);
	}

	LOG_INFO() << "exiting process";
	exit(EXIT_SUCCESS);
}
