//============================================================================
// Name        : Wotan++.cpp
// Author      : Jean-Mathieu Vermosen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#ifdef _WIN32
	
#else
	#include <signal.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <linux/fs.h>
	#include <syslog.h>
#endif

#include <fstream>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "application/logger/logger.hpp"
#include "application/configuration/configuration.hpp"
#include "interactiveBroker/wrapper/wrapper.hpp"

const unsigned MAX_ATTEMPTS = 50;
const unsigned SLEEP_TIME = 10;

bool running = true;
wotan::ib::wrapper wrapper_;

void termSignalHandler(int arg)						// catch termination signal
{
	running = false;
}

void hangupHandler(int arg)							// catch hangup signal
{
	//
}

int detachService()
{
	pid_t pid; pid = fork ();						// create new process

	if 		(pid < 0) 	{ exit(EXIT_FAILURE); }
	else if (pid > 0) 	{ exit(EXIT_SUCCESS); }		// We got a good pid, Close the Parent Process

	umask(0);										// Change File Mask

	if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }	// if we don't find / dir, failure

	close(STDIN_FILENO);							//Close Standard File Descriptors
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }	// if we don't find / dir, failure

	// set the signal handlers
    signal(SIGCHLD, SIG_IGN); 						// child terminate signal
    signal(SIGHUP, hangupHandler); 					// hangup signal
    signal(SIGTERM, termSignalHandler); 			// software termination signal from kill

	return (setsid() < 0);							// set new process id
}

int main(int argc, char** argv)
{
	wotan::logger::setLogger("wotan_%Y%m%d_%3N.log", wotan::logger::Info);

	LOG_INFO() 		<< "this is a information message from parent process";
	LOG_WARNING() 	<< "this is a warning message from parent process";
	LOG_ERROR() 	<< "this is an error message from parent process";

	try
	{
		if(int err = detachService())
		{
			return err;
		}

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

		while(running)
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
