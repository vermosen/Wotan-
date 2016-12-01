#ifndef APPLICATION_SERVICE_DAEMON_HPP_
#define APPLICATION_SERVICE_DAEMON_HPP_

#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <syslog.h>

namespace wotan
{
	const unsigned MAX_ATTEMPTS = 50;
	const unsigned SLEEP_TIME = 10;

	bool running = true;

	// TODO
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
		pid_t pid; pid = fork();						// create new process

		if (pid < 0) { exit(EXIT_FAILURE); }
		else if (pid > 0) { exit(EXIT_SUCCESS); }		// We got a good pid, Close the Parent Process

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
}
#endif
#endif
