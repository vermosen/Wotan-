/*
 * client.cpp
 *
 *  Created on: Sep 14, 2016
 *      Author: vermosen
 */

#include "StdAfx.h"

#ifdef _WIN32
#include <Windows.h>
#ifdef TWSAPIDLL
#ifndef TWSAPIDLLEXP
#define TWSAPIDLLEXP __declspec(dllexport)
#endif
#endif
#else
#include <unistd.h>
#endif

#include "interactiveBroker/client/client.hpp"

#include "EClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"
#include "OrderState.h"
#include "Execution.h"
#include "CommissionReport.h"
#include "ScannerSubscription.h"
#include "executioncondition.h"
#include "PriceCondition.h"
#include "MarginCondition.h"
#include "PercentChangeCondition.h"
#include "TimeCondition.h"
#include "VolumeCondition.h"
#include "CommonDefs.h"

#include "application/logger/logger.hpp"

namespace wotan
{
	namespace ib
	{
	const int PING_DEADLINE = 2; 		// seconds
		const int SLEEP_BETWEEN_PINGS = 30; // seconds

		///////////////////////////////////////////////////////////
		// member funcs
		client::client() :
			  m_osSignal(2000)//2-seconds timeout
			, m_pClient(new EClientSocket(this, &m_osSignal))
			, m_state(ST_CONNECT)
			, m_sleepDeadline(0)
			, m_orderId(0)
			, m_pReader(0)
			, m_extraAuth(false) {}

		//! [socket_init]
		client::~client()
		{
			if (m_pReader)
				delete m_pReader;

			delete m_pClient;
		}

		bool client::connect(const char *host, unsigned int port, int clientId)
		{
			// trying to connect
			printf("Connecting to %s:%d clientId:%d\n", !(host && *host) ? "127.0.0.1" : host, port, clientId);

			bool bRes = m_pClient->eConnect(host, port, clientId, m_extraAuth);

			if (bRes) {
				printf("Connected to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);

				m_pReader = new EReader(m_pClient, &m_osSignal);

				m_pReader->start();
			}
			else
				printf("Cannot connect to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);

			return bRes;
		}

		void client::disconnect() const
		{
			m_pClient->eDisconnect();

			printf("Disconnected\n");
		}

		bool client::isConnected() const
		{
			return m_pClient->isConnected();
		}

		void client::setConnectOptions(const std::string& connectOptions)
		{
			m_pClient->setConnectOptions(connectOptions);
		}

		void client::processMessages() {
			fd_set readSet, writeSet, errorSet;

			struct timeval tval;
			tval.tv_usec = 0;
			tval.tv_sec = 0;

			time_t now = time(NULL);

			/*****************************************************************/
			/* Below are few quick-to-test examples on the IB API functions grouped by functionality. Uncomment the relevant methods. */
			/*****************************************************************/
			switch (m_state) {
			case ST_TICKDATAOPERATION:
				//tickDataOperation();
				break;
			case ST_TICKDATAOPERATION_ACK:
				break;
			case ST_MARKETDEPTHOPERATION:
				//marketDepthOperations();
				break;
			case ST_MARKETDEPTHOPERATION_ACK:
				break;
			case ST_REALTIMEBARS:
				//realTimeBars();
				break;
			case ST_REALTIMEBARS_ACK:
				break;
			case ST_MARKETDATATYPE:
				//marketDataType();
				break;
			case ST_MARKETDATATYPE_ACK:
				break;
			case ST_HISTORICALDATAREQUESTS:
				//historicalDataRequests();
				break;
			case ST_HISTORICALDATAREQUESTS_ACK:
				break;
			case ST_OPTIONSOPERATIONS:
				//optionsOperations();
				break;
			case ST_OPTIONSOPERATIONS_ACK:
				break;
			case ST_CONTRACTOPERATION:
				//contractOperations();
				break;
			case ST_CONTRACTOPERATION_ACK:
				break;
			case ST_MARKETSCANNERS:
				//marketScanners();
				break;
			case ST_MARKETSCANNERS_ACK:
				break;
			case ST_REUTERSFUNDAMENTALS:
				//reutersFundamentals();
				break;
			case ST_REUTERSFUNDAMENTALS_ACK:
				break;
			case ST_BULLETINS:
				//bulletins();
				break;
			case ST_BULLETINS_ACK:
				break;
			case ST_ACCOUNTOPERATIONS:
				//accountOperations();
				break;
			case ST_ACCOUNTOPERATIONS_ACK:
				break;
			case ST_ORDEROPERATIONS:
				//orderOperations();
				break;
			case ST_ORDEROPERATIONS_ACK:
				break;
			case ST_OCASAMPLES:
				//ocaSamples();
				break;
			case ST_OCASAMPLES_ACK:
				break;
			case ST_CONDITIONSAMPLES:
				//conditionSamples();
				break;
			case ST_CONDITIONSAMPLES_ACK:
				break;
			case ST_BRACKETSAMPLES:
				//bracketSample();
				break;
			case ST_BRACKETSAMPLES_ACK:
				break;
			case ST_HEDGESAMPLES:
				//hedgeSample();
				break;
			case ST_HEDGESAMPLES_ACK:
				break;
			case ST_TESTALGOSAMPLES:
				//testAlgoSamples();
				break;
			case ST_TESTALGOSAMPLES_ACK:
				break;
			case ST_FAORDERSAMPLES:
				//financialAdvisorOrderSamples();
				break;
			case ST_FAORDERSAMPLES_ACK:
				break;
			case ST_FAOPERATIONS:
				//financialAdvisorOperations();
				break;
			case ST_FAOPERATIONS_ACK:
				break;
			case ST_DISPLAYGROUPS:
				//testDisplayGroups();
				break;
			case ST_DISPLAYGROUPS_ACK:
				break;
			case ST_MISCELANEOUS:
				//miscelaneous();
				break;
			case ST_MISCELANEOUS_ACK:
				break;
			case ST_PING:
				reqCurrentTime();
				break;
			case ST_PING_ACK:
				if (m_sleepDeadline < now) {
					disconnect();
					return;
				}
				break;
			case ST_IDLE:
				if (m_sleepDeadline < now) {
					m_state = ST_PING;
					return;
				}
				break;
			}

			if (m_sleepDeadline > 0) {
				// initialize timeout with m_sleepDeadline - now
				tval.tv_sec = m_sleepDeadline - now;
			}

			m_pReader->checkClient();
			m_osSignal.waitForSignal();
			m_pReader->processMsgs();
		}

		//////////////////////////////////////////////////////////////////
		// methods
		//! [connectack]
		void client::connectAck() {
			if (!m_extraAuth && m_pClient->asyncEConnect())
				m_pClient->startApi();
		}
		//! [connectack]

		void client::reqCurrentTime()
		{
			printf("Requesting Current Time\n");

			// set ping deadline to "now + n seconds"
			m_sleepDeadline = time(NULL) + PING_DEADLINE;

			m_state = ST_PING_ACK;

			m_pClient->reqCurrentTime();
		}

		void client::currentTime(long time)
		{
			if (m_state == ST_PING_ACK) {
				time_t t = (time_t)time;
				struct tm * timeinfo = localtime(&t);
				printf("The current date/time is: %s", asctime(timeinfo));

				time_t now = ::time(NULL);
				m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;

				m_state = ST_PING_ACK;
			}
		}

		//! [error]
		void client::error(const int id, const int errorCode, const std::string errorString)
		{
			//printf("Error. Id: %d, Code: %d, Msg: %s\n", id, errorCode, errorString.c_str());
		}
		//! [error]

		//! [tickprice]
		void client::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute) {
			//printf("Tick Price. Ticker Id: %ld, Field: %d, Price: %g, CanAutoExecute: %d\n", tickerId, (int)field, price, canAutoExecute);
		}
		//! [tickprice]

		//! [ticksize]
		void client::tickSize(TickerId tickerId, TickType field, int size) {
			//printf("Tick Size. Ticker Id: %ld, Field: %d, Size: %d\n", tickerId, (int)field, size);
		}
		//! [ticksize]

		//! [tickoptioncomputation]
		void client::tickOptionComputation(TickerId tickerId, TickType tickType, double impliedVol, double delta,
			double optPrice, double pvDividend,
			double gamma, double vega, double theta, double undPrice) {
			//printf("TickOptionComputation. Ticker Id: %ld, Type: %d, ImpliedVolatility: %g, Delta: %g, OptionPrice: %g, pvDividend: %g, Gamma: %g, Vega: %g, Theta: %g, Underlying Price: %g\n", tickerId, (int)tickType, impliedVol, delta, optPrice, pvDividend, gamma, vega, theta, undPrice);
		}
		//! [tickoptioncomputation]

		//! [tickgeneric]
		void client::tickGeneric(TickerId tickerId, TickType tickType, double value) {
			//printf("Tick Generic. Ticker Id: %ld, Type: %d, Value: %g\n", tickerId, (int)tickType, value);
		}
		//! [tickgeneric]

		//! [tickstring]
		void client::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
			//printf("Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
		}
		//! [tickstring]

		void client::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
			double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
			//printf("TickEFP. %ld, Type: %d, BasisPoints: %g, FormattedBasisPoints: %s, Total Dividends: %g, HoldDays: %d, Future Last Trade Date: %s, Dividend Impact: %g, Dividends To Last Trade Date: %g\n", tickerId, (int)tickType, basisPoints, formattedBasisPoints.c_str(), totalDividends, holdDays, futureLastTradeDate.c_str(), dividendImpact, dividendsToLastTradeDate);
		}

		//! [orderstatus]
		void client::orderStatus(OrderId orderId, const std::string& status, double filled,
			double remaining, double avgFillPrice, int permId, int parentId,
			double lastFillPrice, int clientId, const std::string& whyHeld) {
			//printf("OrderStatus. Id: %ld, Status: %s, Filled: %g, Remaining: %g, AvgFillPrice: %g, PermId: %d, LastFillPrice: %g, ClientId: %d, WhyHeld: %s\n", orderId, status.c_str(), filled, remaining, avgFillPrice, permId, lastFillPrice, clientId, whyHeld.c_str());
		}
		//! [orderstatus]

		//! [openorder]
		void client::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& ostate) {
			//printf("OpenOrder. ID: %ld, %s, %s @ %s: %s, %s, %g, %s\n", orderId, contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(), order.action.c_str(), order.orderType.c_str(), order.totalQuantity, ostate.status.c_str());
		}
		//! [openorder]

		//! [openorderend]
		void client::openOrderEnd() {
			//printf("OpenOrderEnd\n");
		}
		//! [openorderend]

		void client::winError(const std::string& str, int lastError) {}
		void client::connectionClosed() {
			//printf("Connection Closed\n");
		}

		//! [updateaccountvalue]
		void client::updateAccountValue(const std::string& key, const std::string& val,
			const std::string& currency, const std::string& accountName) {
			//printf("UpdateAccountValue. Key: %s, Value: %s, Currency: %s, Account Name: %s\n", key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());
		}
		//! [updateaccountvalue]

		//! [updateportfolio]
		void client::updatePortfolio(const Contract& contract, double position,
			double marketPrice, double marketValue, double averageCost,
			double unrealizedPNL, double realizedPNL, const std::string& accountName) {
			//printf("UpdatePortfolio. %s, %s @ %s: Position: %g, MarketPrice: %g, MarketValue: %g, AverageCost: %g, UnrealisedPNL: %g, RealisedPNL: %g, AccountName: %s\n", (contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), position, marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL, accountName.c_str());
		}
		//! [updateportfolio]

		//! [updateaccounttime]
		void client::updateAccountTime(const std::string& timeStamp) {
			//printf("UpdateAccountTime. Time: %s\n", timeStamp.c_str());
		}
		//! [updateaccounttime]

		//! [accountdownloadend]
		void client::accountDownloadEnd(const std::string& accountName) {
			//printf("Account download finished: %s\n", accountName.c_str());
		}
		//! [accountdownloadend]

		//! [contractdetails]
		void client::contractDetails(int reqId, const ContractDetails& contractDetails) {
			//printf("ContractDetails. ReqId: %d - %s, %s, ConId: %ld @ %s, Trading Hours: %s, Liquidation Hours: %s\n", reqId, contractDetails.summary.symbol.c_str(), contractDetails.summary.secType.c_str(), contractDetails.summary.conId, contractDetails.summary.exchange.c_str(), contractDetails.tradingHours.c_str(), contractDetails.liquidHours.c_str());
		}
		//! [contractdetails]

		void client::bondContractDetails(int reqId, const ContractDetails& contractDetails) {
			//printf("Bond. ReqId: %d, Symbol: %s, Security Type: %s, Currency: %s, Trading Hours: %s, Liquidation Hours: %s\n", reqId, contractDetails.summary.symbol.c_str(), contractDetails.summary.secType.c_str(), contractDetails.summary.currency.c_str(), contractDetails.tradingHours.c_str(), contractDetails.liquidHours.c_str());
		}

		//! [contractdetailsend]
		void client::contractDetailsEnd(int reqId) {
			//printf("ContractDetailsEnd. %d\n", reqId);
		}
		//! [contractdetailsend]

		//! [execdetails]
		void client::execDetails(int reqId, const Contract& contract, const Execution& execution) {
			//printf("ExecDetails. ReqId: %d - %s, %s, %s - %s, %ld, %g\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), execution.orderId, execution.shares);
		}
		//! [execdetails]

		//! [execdetailsend]
		void client::execDetailsEnd(int reqId) {
			//printf("ExecDetailsEnd. %d\n", reqId);
		}
		//! [execdetailsend]

		//! [updatemktdepth]
		void client::updateMktDepth(TickerId id, int position, int operation, int side,
			double price, int size) {
			//printf("UpdateMarketDepth. %ld - Position: %d, Operation: %d, Side: %d, Price: %g, Size: %d\n", id, position, operation, side, price, size);
		}
		//! [updatemktdepth]

		void client::updateMktDepthL2(TickerId id, int position, std::string marketMaker, int operation,
			int side, double price, int size) {
			//printf("UpdateMarketDepthL2. %ld - Position: %d, Operation: %d, Side: %d, Price: %g, Size: %d\n", id, position, operation, side, price, size);
		}

		//! [updatenewsbulletin]
		void client::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {
			//printf("News Bulletins. %d - Type: %d, Message: %s, Exchange of Origin: %s\n", msgId, msgType, newsMessage.c_str(), originExch.c_str());
		}
		//! [updatenewsbulletin]

		//! [managedaccounts]
		void client::managedAccounts(const std::string& accountsList) {
			//printf("Account List: %s\n", accountsList.c_str());
		}
		//! [managedaccounts]

		//! [receivefa]
		void client::receiveFA(faDataType pFaDataType, const std::string& cxml) {
			//std::cout << "Receiving FA: " << (int)pFaDataType << std::endl << cxml << std::endl;
		}
		//! [receivefa]

		//! [historicaldata]
		void client::historicalData(TickerId reqId, const std::string& date, double open, double high,
			double low, double close, int volume, int barCount, double WAP, int hasGaps) {
			//printf("HistoricalData. ReqId: %ld - Date: %s, Open: %g, High: %g, Low: %g, Close: %g, Volume: %d, Count: %d, WAP: %g, HasGaps: %d\n", reqId, date.c_str(), open, high, low, close, volume, barCount, WAP, hasGaps);
		}
		//! [historicaldata]

		//! [scannerparameters]
		void client::scannerParameters(const std::string& xml) {
			//printf("ScannerParameters. %s\n", xml.c_str());
		}
		//! [scannerparameters]

		//! [scannerdata]
		void client::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
			const std::string& distance, const std::string& benchmark, const std::string& projection,
			const std::string& legsStr) {
			//printf("ScannerData. %d - Rank: %d, Symbol: %s, SecType: %s, Currency: %s, Distance: %s, Benchmark: %s, Projection: %s, Legs String: %s\n", reqId, rank, contractDetails.summary.symbol.c_str(), contractDetails.summary.secType.c_str(), contractDetails.summary.currency.c_str(), distance.c_str(), benchmark.c_str(), projection.c_str(), legsStr.c_str());
		}
		//! [scannerdata]

		//! [scannerdataend]
		void client::scannerDataEnd(int reqId) {
			//printf("ScannerDataEnd. %d\n", reqId);
		}
		//! [scannerdataend]

		//! [realtimebar]
		void client::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
			long volume, double wap, int count) {
			//printf("RealTimeBars. %ld - Time: %ld, Open: %g, High: %g, Low: %g, Close: %g, Volume: %ld, Count: %d, WAP: %g\n", reqId, time, open, high, low, close, volume, count, wap);
		}
		//! [realtimebar]

		//! [fundamentaldata]
		void client::fundamentalData(TickerId reqId, const std::string& data) {
			//printf("FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
		}
		//! [fundamentaldata]

		void client::deltaNeutralValidation(int reqId, const UnderComp& underComp) {
			//printf("DeltaNeutralValidation. %d, ConId: %ld, Delta: %g, Price: %g\n", reqId, underComp.conId, underComp.delta, underComp.price);
		}

		//! [ticksnapshotend]
		void client::tickSnapshotEnd(int reqId) {
			//printf("TickSnapshotEnd: %d\n", reqId);
		}
		//! [ticksnapshotend]

		//! [marketdatatype]
		void client::marketDataType(TickerId reqId, int marketDataType) {
			//printf("MarketDataType. ReqId: %ld, Type: %d\n", reqId, marketDataType);
		}
		//! [marketdatatype]

		//! [commissionreport]
		void client::commissionReport(const CommissionReport& commissionReport) {
			//printf("CommissionReport. %s - %g %s RPNL %g\n", commissionReport.execId.c_str(), commissionReport.commission, commissionReport.currency.c_str(), commissionReport.realizedPNL);
		}
		//! [commissionreport]

		//! [position]
		void client::position(const std::string& account, const Contract& contract, double position, double avgCost) {
			//printf("Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %g, Avg Cost: %g\n", account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), position, avgCost);
		}
		//! [position]

		//! [positionend]
		void client::positionEnd() {
			//printf("PositionEnd\n");
		}
		//! [positionend]

		//! [accountsummary]
		void client::accountSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
			//printf("Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, Currency: %s\n", reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
		}
		//! [accountsummary]

		//! [accountsummaryend]
		void client::accountSummaryEnd(int reqId) {
			//printf("AccountSummaryEnd. Req Id: %d\n", reqId);
		}
		//! [accountsummaryend]

		void client::verifyMessageAPI(const std::string& apiData) {
			//printf("verifyMessageAPI: %s\b", apiData.c_str());
		}

		void client::verifyCompleted(bool isSuccessful, const std::string& errorText) {
			//printf("verifyCompleted. IsSuccessfule: %d - Error: %s\n", isSuccessful, errorText.c_str());
		}

		void client::verifyAndAuthMessageAPI(const std::string& apiDatai, const std::string& xyzChallenge) {
			//printf("verifyAndAuthMessageAPI: %s %s\n", apiDatai.c_str(), xyzChallenge.c_str());
		}

		void client::verifyAndAuthCompleted(bool isSuccessful, const std::string& errorText) {
			//printf("verifyAndAuthCompleted. IsSuccessful: %d - Error: %s\n", isSuccessful, errorText.c_str());
			if (isSuccessful)
				m_pClient->startApi();
		}

		//! [displaygrouplist]
		void client::displayGroupList(int reqId, const std::string& groups) {
			//printf("Display Group List. ReqId: %d, Groups: %s\n", reqId, groups.c_str());
		}
		//! [displaygrouplist]

		//! [displaygroupupdated]
		void client::displayGroupUpdated(int reqId, const std::string& contractInfo) {
			//std::cout << "Display Group Updated. ReqId: " << reqId << ", Contract Info: " << contractInfo << std::endl;
		}
		//! [displaygroupupdated]

		//! [positionmulti]
		void client::positionMulti(int reqId, const std::string& account, const std::string& modelCode, const Contract& contract, double pos, double avgCost) {
			//printf("Position Multi. Request: %d, Account: %s, ModelCode: %s, Symbol: %s, SecType: %s, Currency: %s, Position: %g, Avg Cost: %g\n", reqId, account.c_str(), modelCode.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), pos, avgCost);
		}
		//! [positionmulti]

		//! [positionmultiend]
		void client::positionMultiEnd(int reqId) {
			//printf("Position Multi End. Request: %d\n", reqId);
		}
		//! [positionmultiend]

		//! [accountupdatemulti]
		void client::accountUpdateMulti(int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {
			//printf("AccountUpdate Multi. Request: %d, Account: %s, ModelCode: %s, Key, %s, Value: %s, Currency: %s\n", reqId, account.c_str(), modelCode.c_str(), key.c_str(), value.c_str(), currency.c_str());
		}
		//! [accountupdatemulti]

		//! [accountupdatemultiend]
		void client::accountUpdateMultiEnd(int reqId) {
			//printf("Account Update Multi End. Request: %d\n", reqId);
		}
		//! [accountupdatemultiend]

		//! [securityDefinitionOptionParameter]
		void client::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass, const std::string& multiplier, std::set<std::string> expirations, std::set<double> strikes) {
			//printf("Security Definition Optional Parameter. Request: %d, Trading Class: %s, Multiplier: %s\n", reqId, tradingClass.c_str(), multiplier.c_str());
		}
		//! [securityDefinitionOptionParameter]

		//! [securityDefinitionOptionParameterEnd]
		void client::securityDefinitionOptionalParameterEnd(int reqId) {
			//printf("Security Definition Optional Parameter End. Request: %d\n", reqId);
		}
		//! [securityDefinitionOptionParameterEnd]

		//! [softDollarTiers]
		void client::softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) {
			//printf("Soft dollar tiers (%d):", tiers.size());

			for (int i = 0; i < tiers.size(); i++) {
				//printf("%s\n", tiers[0].displayName());
			}
		}

		//! [nextvalidid]
		void client::nextValidId(OrderId orderId)
		{
			//printf("Next Valid Id: %ld\n", orderId);
			m_orderId = orderId;
			//! [nextvalidid]

			//m_state = ST_TICKDATAOPERATION;
			//m_state = ST_MARKETDEPTHOPERATION;
			//m_state = ST_REALTIMEBARS;
			//m_state = ST_MARKETDATATYPE;
			//m_state = ST_HISTORICALDATAREQUESTS;
			//m_state = ST_CONTRACTOPERATION;
			//m_state = ST_MARKETSCANNERS;
			//m_state = ST_REUTERSFUNDAMENTALS;
			//m_state = ST_BULLETINS;
			m_state = ST_ACCOUNTOPERATIONS;
			//m_state = ST_ORDEROPERATIONS;
			//m_state = ST_OCASAMPLES;
			//m_state = ST_CONDITIONSAMPLES;
			//m_state = ST_BRACKETSAMPLES;
			//m_state = ST_HEDGESAMPLES;
			//m_state = ST_TESTALGOSAMPLES;
			//m_state = ST_FAORDERSAMPLES;
			//m_state = ST_FAOPERATIONS;
			//m_state = ST_DISPLAYGROUPS;
			//m_state = ST_MISCELANEOUS;
			//m_state = ST_PING;
		}
	} /* namespace ib */
} /* namespace wotan */
