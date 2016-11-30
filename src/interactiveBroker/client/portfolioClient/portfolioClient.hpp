/*
 * portfolioClient.hpp
 *
 *  Created on: Oct 1, 2016
 *      Author: vermosen
 */

#ifndef INTERACTIVEBROKER_CLIENT_PORTFOLIOCLIENT_HPP_
#define INTERACTIVEBROKER_CLIENT_PORTFOLIOCLIENT_HPP_

#include <EPosixClientSocket.h>
#include "interactiveBroker/wrapper/wrapper.hpp"

namespace wotan
{
	namespace ib
	{
		class portfolioClient: public EPosixClientSocket
		{
		public:
			portfolioClient(wrapper * ptr);
			virtual ~portfolioClient();
		};
	} /* namespace ib */
} /* namespace wotan */

#endif /* INTERACTIVEBROKER_CLIENT_PORTFOLIOCLIENT_HPP_ */
