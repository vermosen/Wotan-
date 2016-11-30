/*
 * historicalClient.hpp
 *
 *  Created on: Sep 30, 2016
 *      Author: vermosen
 */

#ifndef INTERACTIVEBROKER_CLIENT_HISTORICALCLIENT_HISTORICALCLIENT_HPP_
#define INTERACTIVEBROKER_CLIENT_HISTORICALCLIENT_HISTORICALCLIENT_HPP_

#include <EPosixClientSocket.h>
#include "interactiveBroker/wrapper/wrapper.hpp"

namespace wotan
{
	namespace ib
	{
		class historicalClient : public EPosixClientSocket
		{
		public:
			historicalClient(wrapper * ptr);
			virtual ~historicalClient();
		};
	} /* namespace ib */
} /* namespace wotan */

#endif /* INTERACTIVEBROKER_CLIENT_HISTORICALCLIENT_HISTORICALCLIENT_HPP_ */
