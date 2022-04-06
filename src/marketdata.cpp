//-----------------------------------------------------------------------------------------
/*
    ____                      __      ____
   /\  _`\   __             /'_ `\   /\  _`\
   \ \ \L\_\/\_\    __  _  /\ \L\ \  \ \ \L\ \ _ __    ___
    \ \  _\/\/\ \  /\ \/'\ \/_> _ <_  \ \ ,__//\`'__\ / __`\
     \ \ \/  \ \ \ \/>  </   /\ \L\ \  \ \ \/ \ \ \/ /\ \L\ \
      \ \_\   \ \_\ /\_/\_\  \ \____/   \ \_\  \ \_\ \ \____/
       \/_/    \/_/ \//\/_/   \/___/     \/_/   \/_/  \/___/

                Fix8Pro Example Client Server

Copyright (C) 2010-22 Fix8 Market Technologies Pty Ltd (ABN 29 167 027 198)
ALL RIGHTS RESERVED  https://www.fix8mt.com  heretohelp@fix8mt.com  @fix8mt

This  file is released  under the  GNU LESSER  GENERAL PUBLIC  LICENSE  Version 3.  You can
redistribute  it  and / or modify  it under the  terms of  the  GNU Lesser  General  Public
License as  published  by  the Free  Software Foundation,  either version 3 of the License,
or (at your option) any later version.

This file is distributed in the hope that it will be useful, but  WITHOUT ANY WARRANTY  and
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of  the GNU Lesser General Public  License along with this
file. If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <memory>
#include <vector>
#include <iterator>
#include <typeinfo>
#include <random>
#include <chrono>
#include <poll.h>

// f8 headers
#include <fix8pro/f8includes.hpp>
#include <fix8pro/utils/static_application.hpp>

#include <simpleclisrv.hpp>

//-----------------------------------------------------------------------------------------
/// Match an order against the opposite aggregated book side; trade price generation only
template<typename T>
int SimpleSession::match(double lprice, uint32_t& lqty, T& book, const f8String& sym, MessagePtr& mdir)
{
	int cnt = 0;
	const auto& grnomden = mdir->find_group<MarketDataIncrementalRefresh::NoMDEntries>();
   for (auto itr = book.begin(); itr != book.end() && lqty > 0; ++cnt) // first is top of book
   {
		auto pitr = itr++; // avoid invalidated iterator if erased
		auto& [rqty, rcnt] = pitr->second;
		const auto& rprice = pitr->first;

		// compiler will optimise out the following ternary
		if (std::is_same_v<T, BidsAggregated> ? lprice > rprice : lprice < rprice)
			break;
		auto gr1 = grnomden->create_group();
		*gr1	<< mdir->make_field<MDUpdateAction>(MDUpdateAction::New)
				<< mdir->make_field<MDEntryType>(MDEntryType::Trade)
				<< mdir->make_field<MDEntrySize>(lqty)
				<< mdir->make_field<MDEntryPx>(std::is_same_v<T, BidsAggregated> ? lprice : rprice)
				<< mdir->make_field<Symbol>(sym);
		*grnomden << std::move(gr1);
		update_daily(std::is_same_v<T, BidsAggregated> ? lprice : rprice, lqty, sym);
		if (lqty <= rqty)
		{
			rqty -= lqty;
			lqty = 0;
		}
		else
		{
			lqty -= rqty;
			rqty = 0;
		}
		if (rcnt > 0 && --rcnt == 0 || rqty == 0)
			book.erase(pitr); // remove price level
   }
	return cnt;
}

//-----------------------------------------------------------------------------------------
// insert into aggregated book
template<typename T>
bool SimpleSession::insert_update(double price, uint32_t qty, T& book)
{
	if (auto itr = book.find(price); itr != book.end())
	{
		auto& [lqty, lcnt] = itr->second;
		lqty += qty;
		++lcnt;
		// std::cout << "updated " << price << std::endl;
		return true;
	}
	auto result = book.emplace(price, std::make_tuple(qty, 1)); // add price level
	// std::cout << "added " << price << std::endl;
	return result.second;
}

//-----------------------------------------------------------------------------------------
// remove from aggregated book
template<typename T>
bool SimpleSession::remove(double price, uint32_t qty, T& book)
{
	if (auto itr = book.find(price); itr != book.end())
	{
		auto& [lqty, lcnt] = itr->second;
		if (lqty >= qty)
			lqty -= qty;
		else
			lqty = 0;
		if (lcnt > 0 && --lcnt == 0 || lqty == 0)
			book.erase(itr); // remove price level
		// std::cout << "removed " << price << std::endl;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
template<typename T>
int SimpleSession::generate_delta(const T& obook, const T& nbook, const f8String& sym, MessagePtr& mdir) const
{
	int cnt = 0, level = 1;
	constexpr auto side = std::is_same_v<T, BidsAggregated> ? MDEntryType::Bid : MDEntryType::Offer;
	const auto& grnomden = mdir->find_group<MarketDataIncrementalRefresh::NoMDEntries>();
	for (auto oitr = obook.cbegin(), nitr = nbook.cbegin(); nitr != nbook.cend() && oitr != obook.cend(); ++level)
	{
		const auto& [oqty, osz] = oitr->second;
		const auto& oprice = oitr->first;
		const auto& [nqty, nsz] = nitr->second;
		const auto& nprice = nitr->first;

		if (nprice == oprice)
		{
			if (nqty != oqty || nsz != osz)
			{
				auto gr1 = grnomden->create_group();
				*gr1	<< mdir->make_field<MDUpdateAction>(MDUpdateAction::Change)
						<< mdir->make_field<MDEntryPositionNo>(level)
						<< mdir->make_field<MDEntrySize>(nqty)
						<< mdir->make_field<NumberOfOrders>(nsz)
						<< mdir->make_field<MDEntryType>(side)
						<< mdir->make_field<Symbol>(sym);
				*grnomden << std::move(gr1);
				++cnt;
			}
			++nitr, ++oitr;
			continue;
		}

		// compiler will optimise out the following ternary
		if (std::is_same_v<T, BidsAggregated> ? nprice > oprice : nprice < oprice)
		{
			// insert
			auto gr1 = grnomden->create_group();
			*gr1	<< mdir->make_field<MDUpdateAction>(MDUpdateAction::New)
					<< mdir->make_field<MDEntryType>(side)
					<< mdir->make_field<MDEntryPositionNo>(level)
					<< mdir->make_field<NumberOfOrders>(nsz)
					<< mdir->make_field<MDEntrySize>(nqty)
					<< mdir->make_field<MDEntryPx>(nprice)
					<< mdir->make_field<Symbol>(sym);
			*grnomden << std::move(gr1);
			++nitr;
		}
		else
		{
			// delete
			auto gr1 = grnomden->create_group();
			*gr1	<< mdir->make_field<MDUpdateAction>(MDUpdateAction::Delete)
					<< mdir->make_field<MDEntryType>(side)
					<< mdir->make_field<MDEntryPositionNo>(level)
					<< mdir->make_field<Symbol>(sym);
			*grnomden << std::move(gr1);
			++oitr;
		}
		++cnt;
	}
	return cnt;
}

//-----------------------------------------------------------------------------------------
template<typename T>
int SimpleSession::generate_obsnapshot(const T& book, MessagePtr& mdsfr) const
{
	int cnt = 0, level = 0;
	constexpr auto side = std::is_same_v<T, BidsAggregated> ? MDEntryType::Bid : MDEntryType::Offer;
	const auto& grnomden = mdsfr->find_group<MarketDataSnapshotFullRefresh::NoMDEntries>();
	for (const auto& [nprice, values] : book)
	{
		const auto& [nqty, nsz] = values;
		auto gr1 = grnomden->create_group();
		*gr1	<< mdsfr->make_field<MDEntryPositionNo>(++level)
				<< mdsfr->make_field<NumberOfOrders>(nsz)
				<< mdsfr->make_field<MDEntrySize>(nqty)
				<< mdsfr->make_field<MDEntryType>(side)
				<< mdsfr->make_field<MDEntryPx>(nprice);
		*grnomden << std::move(gr1);
		++cnt;
		if (level >= _app._depth)
			break;
	}
	return cnt;
}

//-----------------------------------------------------------------------------------------
template<typename T>
int SimpleSession::populate_mdentry(char tag, T what, MessagePtr& mdsfr) const
{
	if (!what)
		return 0;
	const auto& grnomden = mdsfr->find_group<MarketDataSnapshotFullRefresh::NoMDEntries>();
	auto gr1 = grnomden->create_group();
	*gr1 << mdsfr->make_field<MDEntryType>(tag);
	if constexpr (std::is_same_v<T, uint32_t>)
		*gr1 << mdsfr->make_field<MDEntrySize>(what);
	else
		*gr1 << mdsfr->make_field<MDEntryPx>(what);
	*grnomden << std::move(gr1);
	return 1;
}

//-----------------------------------------------------------------------------------------
/// Populates tob side, returns qty and count added
template<typename T>
std::pair<uint32_t, int> SimpleSession::populate_tob(T& book, MessagePtr& mdsfr) const
{
	if (book.empty())
		return {};
	const auto& grnomden = mdsfr->find_group<MarketDataSnapshotFullRefresh::NoMDEntries>();
	constexpr auto side = std::is_same_v<T, BidsAggregated> ? MDEntryType::Bid : MDEntryType::Offer;
	const auto& [qty, sz] = book.cbegin()->second;
	const auto& price = book.cbegin()->first;
	auto gr1 = grnomden->create_group();
	*gr1	<< mdsfr->make_field<MDEntryType>(side)
			<< mdsfr->make_field<MDEntrySize>(qty)
			<< mdsfr->make_field<NumberOfOrders>(sz)
			<< mdsfr->make_field<MDEntryPx>(price);
	*grnomden << std::move(gr1);
	return { qty, 1 };
}

//-----------------------------------------------------------------------------------------
int SimpleSession::populate_trade(double price, uint32_t qty, MessagePtr& mdsfr) const
{
	if (price <= 0.)
		return 0;
	const auto& grnomden = mdsfr->find_group<MarketDataSnapshotFullRefresh::NoMDEntries>();
	auto gr1 = grnomden->create_group();
	*gr1	<< mdsfr->make_field<MDEntryType>(MDEntryType::Trade)
			<< mdsfr->make_field<MDEntrySize>(qty)
			<< mdsfr->make_field<MDEntryPx>(price);
	*grnomden << std::move(gr1);
	return 1;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::update_daily(double price, uint32_t qty, const f8String& sym)
{
	auto ditr = _dailies.find(sym);
	auto& rec = ditr == _dailies.end() ? _dailies.emplace(sym, Daily()).first->second : ditr->second;
	rec.last = price;
	rec.lvol = qty;
	rec.tvol += qty;
	rec.tpv += price * qty;
	if (!rec.open)
		rec.open = price;
	if (!rec.low || price < rec.low)
		rec.low = price;
	if (!rec.high || price > rec.high)
		rec.high = price;
	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::create_mdsnapshot(const f8String& sym)
{
	const auto ditr = _dailies.find(sym);
	if (ditr == _dailies.cend())
		return false;
	const auto& rec = ditr->second;
	auto mdsfr = make_message<MarketDataSnapshotFullRefresh>();
	int cnt = 0;
	if (const auto obitr = _orderbook.find(sym); obitr != _orderbook.cend())
	{
		auto& [nbids, nasks] = obitr->second;
		auto [lv, cnt0] = populate_tob(nbids, mdsfr);
		auto [rv, cnt1] = populate_tob(nasks, mdsfr);
		cnt += cnt0 + cnt1;
		if (lv + rv > 0)
			cnt += populate_mdentry(MDEntryType::Imbalance,
				roundtoplaces<2>((static_cast<double>(lv) - rv) / (static_cast<double>(lv) + rv)), mdsfr);
	}
	cnt += populate_trade(rec.last, rec.lvol, mdsfr);
	cnt += populate_mdentry(MDEntryType::TradingSessionHighPrice, rec.high, mdsfr);
	cnt += populate_mdentry(MDEntryType::TradingSessionLowPrice, rec.low, mdsfr);
	cnt += populate_mdentry(MDEntryType::OpeningPrice, rec.open, mdsfr);
	cnt += populate_mdentry(MDEntryType::ClosingPrice, rec.close, mdsfr);
	cnt += populate_mdentry(MDEntryType::TradeVolume, rec.tvol,  mdsfr);
	if (rec.tvol)
		cnt += populate_mdentry(MDEntryType::TradingSessionVWAPPrice, roundtoplaces<2>(rec.tpv / rec.tvol),  mdsfr);
	if (!cnt)
		return false;
	*mdsfr << mdsfr->make_field<Symbol>(sym);
	*mdsfr << mdsfr->make_field<NoMDEntries>(cnt);
	send(std::move(mdsfr));
	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::create_obsnapshot(const f8String& sym)
{
	const auto obitr = _orderbook.find(sym);
	if (obitr == _orderbook.cend())
		return false;
	const auto& [nbids, nasks] = obitr->second;
	auto mdsfr = make_message<MarketDataSnapshotFullRefresh>();
	*mdsfr << mdsfr->make_field<Symbol>(sym);
	*mdsfr << mdsfr->make_field<NoMDEntries>(generate_obsnapshot(nbids, mdsfr) + generate_obsnapshot(nasks, mdsfr));
	send(std::move(mdsfr));
	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::create_snapshots()
{
	f8_scoped_spin_lock slck(_spin_lock);
	// more efficient to just copy the subscription map each time
	const auto wsubs = _subscriptions_vec;
	slck.release();
	auto numsyms = std::uniform_int_distribution<>(1, wsubs.size())(_app._eng);
	for (int ii = 0; ii < numsyms; ++ii)
		// 50% depth snapshot, 50% tob/ohlc snapshot
		std::bernoulli_distribution()(_app._eng) ? create_obsnapshot(wsubs[ii].first) : create_mdsnapshot(wsubs[ii].first);
	return true;
}

//-----------------------------------------------------------------------------------------
/// Prefill the orderbooks
bool SimpleSession::pregenerate_send_marketdata()
{
	f8_scoped_spin_lock slck(_spin_lock);
	const auto wsubs = _subscriptions_vec;
	slck.release();
	for (const auto& [sym, rec] : wsubs)
	{
		const auto& [rprice, rqty] = rec;
		const auto gencnt = std::uniform_int_distribution<>(1, 20)(_app._eng); // random gen count
		for (int jj = 0; jj < gencnt; ++jj)
		{
			auto price = roundtoplaces<2>(std::cauchy_distribution<>(rprice, _app._cauchy_scale)(_app._eng));
			auto qty = std::uniform_int_distribution<>(1, rqty)(_app._eng); // random qty
			auto obitr = _orderbook.find(sym);
			if (obitr == _orderbook.end())
				obitr = _orderbook.emplace(sym, make_tuple(BidsAggregated(), AsksAggregated())).first;
			auto& [nbids, nasks] = obitr->second;
			if (std::bernoulli_distribution()(_app._eng)) // coin toss side
				insert_update(price, qty, nbids);
			else
				insert_update(price, qty, nasks);
		}
		create_obsnapshot(sym);
	}
	return true;
}

//-----------------------------------------------------------------------------------------
/// Generate marketdata randomly
bool SimpleSession::generate_send_marketdata()
{
	if (std::bernoulli_distribution(0.1)(_app._eng)) // 10% snapshots
		return create_snapshots();

	f8_scoped_spin_lock slck(_spin_lock);
	const auto wsubs = _subscriptions_vec;
	slck.release();
	auto numsyms = std::uniform_int_distribution<>(1, wsubs.size())(_app._eng); // generate random number of symbols
	auto mdir = make_message<MarketDataIncrementalRefresh>();
	int cnt = 0;
	for (int ii = 0; ii < numsyms; ++ii)
	{
		const auto& [sym, rec] = wsubs[std::uniform_int_distribution<>(0, wsubs.size() - 1)(_app._eng)]; // generate random symbol from set
		const auto& [rprice, rqty] = rec;
		auto side = std::bernoulli_distribution()(_app._eng); // coin toss side
		auto price = roundtoplaces<2>(std::cauchy_distribution<>(rprice, _app._cauchy_scale)(_app._eng));
		auto qty = std::uniform_int_distribution<uint32_t>(1, rqty)(_app._eng); // random qty
		auto obitr = _orderbook.find(sym);
		bool cancel, inserted = false;
		if (obitr == _orderbook.end())
		{
			obitr = _orderbook.emplace(sym, make_tuple(BidsAggregated(), AsksAggregated())).first;
			inserted = true;
			cancel = false;
		}
		BidsAggregated obids;
		AsksAggregated oasks;
		auto& [nbids, nasks] = obitr->second;

		if (!inserted)
		{
			cancel = std::bernoulli_distribution(0.25)(_app._eng); // 25% cancel
			obids = nbids; // save current to calculate delta
			oasks = nasks;
		}

		if (cancel)
		{
			if (side)
				remove(price, qty, nbids);
			else
				remove(price, qty, nasks);
		}
		else if (side)
		{
			cnt += match(price, qty, nasks, sym, mdir);
			if (qty) // insert remaining
				insert_update(price, qty, nbids);
		}
		else
		{
			cnt += match(price, qty, nbids, sym, mdir);
			if (qty) // insert remaining
				insert_update(price, qty, nasks);
		}
		cnt += side ? generate_delta(obids, nbids, sym, mdir) : generate_delta(oasks, nasks, sym, mdir);
	}
	if (cnt)
	{
		*mdir << mdir->make_field<NoMDEntries>(cnt);
		send(std::move(mdir));
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::send_security_list_request()
{
	static unsigned rid = 0;
	auto slr = make_message<SecurityListRequest>();
	*slr << slr->make_field<SecurityReqID>(make_id(++rid))
		  << slr->make_field<SecurityListRequestType>(SecurityListRequestType::AllSecurities);
	send(std::move(slr));
	return true;
}

//-----------------------------------------------------------------------------------------
/// SecurityList handler (client, market data mode), handle security list result, send subscriptions (MarketDataRequest)
bool SimpleSession::operator()(const SecurityList *msg)
{
	std::vector<f8String> avail_subscriptions;
	if (const auto& grnors = msg->find_group<SecurityList::NoRelatedSym>(); grnors)
		for (const auto& pp : *grnors)
			avail_subscriptions.push_back(pp->get<Symbol>()->get().c_str()); // force conversion from f8LocalString to f8String

	if (!avail_subscriptions.empty())
	{
		auto mdr = make_message<MarketDataRequest>();
		const auto& grnors = mdr->find_group<MarketDataRequest::NoRelatedSym>();
		int cnt = 0;
		for (size_t ii = 0; ii < avail_subscriptions.size(); ++ii)
		{
			const auto& sym = avail_subscriptions[std::uniform_int_distribution<>(0, avail_subscriptions.size() - 1)(_app._eng)]; // choose instr
			f8_scoped_spin_lock slck(_spin_lock);
			if (_subscriptions.count(sym) == 0)
			{
				auto gr1 = grnors->create_group();
				*gr1 << mdr->make_field<Symbol>(sym)
					  << mdr->make_field<SecurityType>(SecurityType::CommonStock);
				*grnors << std::move(gr1);
				_subscriptions.emplace(sym);
				++cnt;
			}
		}

		const auto& grnomdet = mdr->find_group<MarketDataRequest::NoMDEntryTypes>();
		constexpr const std::array mdtypes { MDEntryType::Bid, MDEntryType::Offer, MDEntryType::Trade, MDEntryType::OpeningPrice,
			MDEntryType::ClosingPrice, MDEntryType::TradeVolume, MDEntryType::TradingSessionHighPrice, MDEntryType::TradingSessionLowPrice,
			MDEntryType::TradingSessionVWAPPrice };
		for (const auto pp : mdtypes)
		{
			auto gr1 = grnomdet->create_group();
			*gr1 << mdr->make_field<MDEntryType>(pp);
			*grnomdet << std::move(gr1);
		}

		static unsigned rid = 0;
		*mdr << mdr->make_field<MarketDepth>(_app._depth)
			  << mdr->make_field<NoRelatedSym>(cnt)
			  << mdr->make_field<NoMDEntryTypes>(mdtypes.size())
			  << mdr->make_field<MDUpdateType>(MDUpdateType::IncrementalRefresh)
			  << mdr->make_field<MDReqID>(make_id(++rid))
			  << mdr->make_field<SubscriptionRequestType>(SubscriptionRequestType::SnapshotAndUpdates);
		send(std::move(mdr));
	}
	else
		std::cerr << "No securities found to subscribe to" << std::endl;

	return true;
}

//-----------------------------------------------------------------------------------------
/// SecurityListRequest handler (server, marketdata mode), process security download request
bool SimpleSession::operator()(const SecurityListRequest *msg)
{
	static unsigned rid = 0;
	auto sl = make_message<SecurityList>();
	*sl << sl->make_field<SecurityReqID>(msg->get<SecurityReqID>()->get())
		 << sl->make_field<SecurityResponseID>(make_id(++rid));

	if (msg->get<SecurityListRequestType>()->get() != SecurityListRequestType::AllSecurities)
		*sl << sl->make_field<SecurityRequestResult>(SecurityRequestResult::InvalidOrUnsupportedRequest);
	else if (const auto& grnors = sl->find_group<SecurityList::NoRelatedSym>(); grnors)
	{
		for (const auto& pp : _app._refdata)
		{
			auto gr1 = grnors->create_group();
			*gr1 << sl->make_field<Symbol>(pp.first)
				  << sl->make_field<SecurityType>(SecurityType::CommonStock);
			*grnors << std::move(gr1);
		}
		*sl << sl->make_field<NoRelatedSym>(_app._refdata.size())
			 << sl->make_field<SecurityRequestResult>(SecurityRequestResult::ValidRequest);
	}

	send(std::move(sl));
	return true;
}

//-----------------------------------------------------------------------------------------
/// MarketDataRequest handler (server, marketdata mode), process security data request
bool SimpleSession::operator()(const MarketDataRequest *msg)
{
	unsubscribe();

	switch(msg->get<SubscriptionRequestType>()->get())
	{
	case SubscriptionRequestType::SnapshotAndUpdates:
	case SubscriptionRequestType::Snapshot:
		if (const auto& grnors = msg->find_group<MarketDataRequest::NoRelatedSym>(); grnors)
		{
			f8_scoped_spin_lock slck(_spin_lock);
			for (const auto& pp : *grnors)
			{
				const auto& sym = pp->get<Symbol>()->get();
				if (const auto iitr = _app._refdata.find(sym.c_str()); iitr != _app._refdata.cend())
				{
					if (_subscriptions.count(sym.c_str()) == 0)
					{
						_subscriptions.emplace(iitr->first);
						_subscriptions_vec.emplace_back(iitr->first, iitr->second);
					}
				}
				else
					std::cerr << sym << " not valid" << std::endl;
			}
		}
		_app._depth = msg->get<MarketDepth>()->get();
		pregenerate_send_marketdata();
		break;
	case SubscriptionRequestType::DisablePreviousSnapshot:
		break;
	}
	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::operator()(const MarketDataSnapshotFullRefresh *msg)
{
	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::operator()(const MarketDataIncrementalRefresh *msg)
{
	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::operator()(const MarketDataRequestReject *msg)
{
	return true;
}

//-----------------------------------------------------------------------------------------
/// See if we have any subscribers
bool SimpleSession::has_subscriptions() const
{
	f8_scoped_spin_lock slck(_spin_lock);
	return !_subscriptions.empty();
}

//-----------------------------------------------------------------------------------------
void SimpleSession::unsubscribe()
{
	f8_scoped_spin_lock slck(_spin_lock);
	_subscriptions.clear();
	_subscriptions_vec.clear();
	_orderbook.clear();
}

