/*
 * wrapper.cpp
 *
 *  Created on: Sep 14, 2016
 *      Author: vermosen
 */

#include <stdio.h>			// added by me
#include "EPosixClientSocket.h"
#include "EPosixClientSocket.cpp"
#include "EPosixClientSocketPlatform.h"
#include "EClientSocketBase.cpp"
#include "Contract.h"
#include "Order.h"

#include "interactiveBroker/wrapper/wrapper.hpp"
#include "application/logger/logger.hpp"

namespace wotan
{
	namespace ib
	{
	const int PING_DEADLINE = 2; 		// seconds
		const int SLEEP_BETWEEN_PINGS = 30; // seconds

		///////////////////////////////////////////////////////////
		// member funcs
		wrapper::wrapper()
			: m_pClient(new EPosixClientSocket(this))
			, m_state(ST_CONNECT)
			, m_sleepDeadline(0)
			, m_orderId(0) {}

		wrapper::~wrapper() {}

		bool wrapper::connect(const char *host, unsigned int port, int clientId)
		{
			// trying to connect
			LOG_INFO() << "Connecting to host";

			bool bRes = m_pClient->eConnect( host, port, clientId, /* extraAuth */ false);

			if (bRes)
			{
				LOG_INFO() << "connection successfull";
			}
			else
				LOG_WARNING() << "connection successfull";

			return bRes;
		}

		void wrapper::disconnect() const
		{
			m_pClient->eDisconnect();
			LOG_INFO() << "disconnected";
		}

		bool wrapper::isConnected() const
		{
			return m_pClient->isConnected();
		}

		void wrapper::processMessages()
		{
			fd_set readSet, writeSet, errorSet;

			struct timeval tval;
			tval.tv_usec = 0;
			tval.tv_sec = 0;

			time_t now = time(NULL);

			switch (m_state) {
				case ST_PLACEORDER:
					placeOrder();
					break;
				case ST_PLACEORDER_ACK:
					break;
				case ST_CANCELORDER:
					cancelOrder();
					break;
				case ST_CANCELORDER_ACK:
					break;
				case ST_PING:
					reqCurrentTime();
					break;
				case ST_PING_ACK:
					if( m_sleepDeadline < now) {
						disconnect();
						return;
					}
					break;
				case ST_IDLE:
					if( m_sleepDeadline < now) {
						m_state = ST_PING;
						return;
					}
					break;
			}

			if( m_sleepDeadline > 0) {
				// initialize timeout with m_sleepDeadline - now
				tval.tv_sec = m_sleepDeadline - now;
			}

			if( m_pClient->fd() >= 0 ) {

				FD_ZERO( &readSet);
				errorSet = writeSet = readSet;

				FD_SET( m_pClient->fd(), &readSet);

				if( !m_pClient->isOutBufferEmpty())
					FD_SET( m_pClient->fd(), &writeSet);

				FD_SET( m_pClient->fd(), &errorSet);

				int ret = select( m_pClient->fd() + 1, &readSet, &writeSet, &errorSet, &tval);

				if( ret == 0) { // timeout
					return;
				}

				if( ret < 0) {	// error
					disconnect();
					return;
				}

				if( m_pClient->fd() < 0)
					return;

				if( FD_ISSET( m_pClient->fd(), &errorSet)) {
					// error on socket
					m_pClient->onError();
				}

				if( m_pClient->fd() < 0)
					return;

				if( FD_ISSET( m_pClient->fd(), &writeSet)) {
					// socket is ready for writing
					m_pClient->onSend();
				}

				if( m_pClient->fd() < 0)
					return;

				if( FD_ISSET( m_pClient->fd(), &readSet)) {
					// socket is ready for reading
					m_pClient->onReceive();
				}
			}
		}

		//////////////////////////////////////////////////////////////////
		// methods
		void wrapper::reqCurrentTime()
		{
			LOG_INFO() << "Requesting Current Time";

			// set ping deadline to "now + n seconds"
			m_sleepDeadline = time( NULL) + PING_DEADLINE;

			m_state = ST_PING_ACK;

			m_pClient->reqCurrentTime();
		}

		void wrapper::placeOrder()
		{
			Contract contract;
			Order order;

			contract.symbol = "IBM";
			contract.secType = "STK";
			contract.exchange = "SMART";
			contract.currency = "USD";

			order.action = "BUY";
			order.totalQuantity = 1000;
			order.orderType = "LMT";
			order.lmtPrice = 0.01;

			//printf( "Placing Order %ld: %s %ld %s at %f\n", m_orderId, order.action.c_str(), order.totalQuantity, contract.symbol.c_str(), order.lmtPrice);

			m_state = ST_PLACEORDER_ACK;

			m_pClient->placeOrder( m_orderId, contract, order);
		}

		void wrapper::cancelOrder()
		{
			//printf( "Cancelling Order %ld\n", m_orderId);

			m_state = ST_CANCELORDER_ACK;

			m_pClient->cancelOrder( m_orderId);
		}

		///////////////////////////////////////////////////////////////////
		// events
		void wrapper::orderStatus( OrderId orderId, const IBString &status, int filled,
			   int remaining, double avgFillPrice, int permId, int parentId,
			   double lastFillPrice, int clientId, const IBString& whyHeld)

		{
			if( orderId == m_orderId) {
				if( m_state == ST_PLACEORDER_ACK && (status == "PreSubmitted" || status == "Submitted"))
					m_state = ST_CANCELORDER;

				if( m_state == ST_CANCELORDER_ACK && status == "Cancelled")
					m_state = ST_PING;

				//printf( "Order: id=%ld, status=%s\n", orderId, status.c_str());
			}
		}

		void wrapper::nextValidId( OrderId orderId)
		{
			m_orderId 	= orderId;
			m_state 	= ST_PLACEORDER;
		}

		void wrapper::currentTime( long time)
		{
			if ( m_state == ST_PING_ACK)
			{
				time_t t = ( time_t)time;
				struct tm * timeinfo = localtime ( &t);
				//printf( "The current date/time is: %s", asctime( timeinfo));

				time_t now = ::time(NULL);
				m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;
				m_state = ST_IDLE;
			}
		}

		void wrapper::error(const int id, const int errorCode, const IBString errorString)
		{
			LOG_ERROR() << "Error id=" << id << ", error code=" << errorCode;
		//	printf( "Error id=%d, errorCode=%d, msg=%s\n", id, errorCode, errorString.c_str());

			if( id == -1 && errorCode == 1100) // if "Connectivity between IB and TWS has been lost"
				disconnect();
		}

		void wrapper::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute) {}
		void wrapper::tickSize( TickerId tickerId, TickType field, int size) {}
		void wrapper::tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
											 double optPrice, double pvDividend,
											 double gamma, double vega, double theta, double undPrice) {}
		void wrapper::tickGeneric(TickerId tickerId, TickType tickType, double value) {}
		void wrapper::tickString(TickerId tickerId, TickType tickType, const IBString& value) {}
		void wrapper::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
									   double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry) {}
		void wrapper::openOrder( OrderId orderId, const Contract&, const Order&, const OrderState& ostate) {}
		void wrapper::openOrderEnd() {}
		void wrapper::winError( const IBString &str, int lastError) {}
		void wrapper::connectionClosed() {}
		void wrapper::updateAccountValue(const IBString& key, const IBString& val,
												  const IBString& currency, const IBString& accountName) {}
		void wrapper::updatePortfolio(const Contract& contract, int position,
				double marketPrice, double marketValue, double averageCost,
				double unrealizedPNL, double realizedPNL, const IBString& accountName){}
		void wrapper::updateAccountTime(const IBString& timeStamp) {}
		void wrapper::accountDownloadEnd(const IBString& accountName) {}
		void wrapper::contractDetails( int reqId, const ContractDetails& contractDetails) {}
		void wrapper::bondContractDetails( int reqId, const ContractDetails& contractDetails) {}
		void wrapper::contractDetailsEnd( int reqId) {}
		void wrapper::execDetails( int reqId, const Contract& contract, const Execution& execution) {}
		void wrapper::execDetailsEnd( int reqId) {}

		void wrapper::updateMktDepth(TickerId id, int position, int operation, int side,
											  double price, int size) {}
		void wrapper::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
												int side, double price, int size) {}
		void wrapper::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch) {}
		void wrapper::managedAccounts( const IBString& accountsList) {}
		void wrapper::receiveFA(faDataType pFaDataType, const IBString& cxml) {}
		void wrapper::historicalData(TickerId reqId, const IBString& date, double open, double high,
											  double low, double close, int volume, int barCount, double WAP, int hasGaps) {}
		void wrapper::scannerParameters(const IBString &xml) {}
		void wrapper::scannerData(int reqId, int rank, const ContractDetails &contractDetails,
			   const IBString &distance, const IBString &benchmark, const IBString &projection,
			   const IBString &legsStr) {}
		void wrapper::scannerDataEnd(int reqId) {}
		void wrapper::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
										   long volume, double wap, int count) {}
		void wrapper::fundamentalData(TickerId reqId, const IBString& data) {}
		void wrapper::deltaNeutralValidation(int reqId, const UnderComp& underComp) {}
		void wrapper::tickSnapshotEnd(int reqId) {}
		void wrapper::marketDataType(TickerId reqId, int marketDataType) {}
		void wrapper::commissionReport( const CommissionReport& commissionReport) {}
		void wrapper::position( const IBString& account, const Contract& contract, int position, double avgCost) {}
		void wrapper::positionEnd() {}
		void wrapper::accountSummary( int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency) {}
		void wrapper::accountSummaryEnd( int reqId) {}
		void wrapper::verifyMessageAPI( const IBString& apiData) {}
		void wrapper::verifyCompleted( bool isSuccessful, const IBString& errorText) {}
		void wrapper::displayGroupList( int reqId, const IBString& groups) {}
		void wrapper::displayGroupUpdated( int reqId, const IBString& contractInfo) {}
	} /* namespace ib */
} /* namespace wotan */
