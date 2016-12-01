#ifndef APPLICATION_SERVICE_HPP_
#define APPLICATION_SERVICE_HPP_

#ifdef _WIN32
#include <Windows.h>
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

namespace wotan
{
	#ifdef _WIN32
	// win32 service
	#include "service\winService.hpp"
	#else
	#endif
}
#endif
