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
#ifndef FIX8PRO_SIMPLECLISRV_HPP_
#define FIX8PRO_SIMPLECLISRV_HPP_

//-----------------------------------------------------------------------------------------
#if defined FIX8PRO_MAGIC_NUM && FIX8PRO_MAGIC_NUM < 220300L
#error Fix8Pro version 22.03 or greater required to biuld this application.
#endif

//-----------------------------------------------------------------------------------------
#include <FIX44_EXAMPLE_types.hpp>
#include <FIX44_EXAMPLE_router.hpp>
#include <FIX44_EXAMPLE_classes.hpp>

//-----------------------------------------------------------------------------------------
using namespace cxxopts;
using namespace FIX8;
using namespace FIX44_EXAMPLE;
using namespace std::string_literals;

//-----------------------------------------------------------------------------------------
struct Detail
{
	double _ref, _lastrv;
	uint32_t _qty;

	Detail(double ref = 0., double lastrv = 0., uint32_t qty = 0) : _ref(ref), _lastrv(lastrv), _qty(qty) {}
};
using Instruments = std::map<f8String, Detail>;
using BidsAggregated = std::map<double, std::tuple<uint32_t, uint32_t>, std::greater<double>>; // price: total qty, total size
using AsksAggregated = std::map<double, std::tuple<uint32_t, uint32_t>>; // price: total qty, total size
using OrderBook = std::map<f8String, std::tuple<BidsAggregated, AsksAggregated>>;
using TickBar = std::map<Tickval, std::tuple<double, uint32_t>>;
struct History
{
	TickBar _ticks; // tick history - time:price/qty
	double open = 0., high = 0., low = 0., close = 0., tpv = 0.;
	uint32_t tvol = 0;
};
using Histories = std::map<f8String, History>;
//-----------------------------------------------------------------------------------------
class Brownian final
{
	const double _volume, _lpf;

public:
	Brownian(double volume, double lpf) : _volume(volume), _lpf(lpf) {}
	constexpr double generate(double last, double rndv) const
	{
		return last - _lpf * (last - (2.0 * rndv - 1.0) / _volume); // rc filter
	}
};

enum BrownParams { drift, volume, lpf };

//-----------------------------------------------------------------------------------------
class Application final : public Fix8ProApplication
{
	int _giveupreset, _interval, _depth, _maxsec;
	bool _server, _reliable, _hb, _quiet, _show_states, _generate, _summary, _mdata;
	std::vector<double> _brown_opts;
	f8String _libdir, _clcf, _global_logger_name, _sses, _cses, _libpath, _tname, _username, _password, _capfile, _reffile, _tickfile = "ticks.dat";
	unsigned _next_send, _next_receive;
	std::mt19937_64 _eng = create_seeded_mersenne_engine(); // provided by fix8pro; you can optionally supply an implementation-defined token
	std::unique_ptr<std::ostream> _ofptr;
	std::unique_ptr<std::ostream, f8_deleter> _cofs;
	std::unique_ptr<Brownian> _br;

	static const Instruments _staticdata;
	Instruments _refdata;

	int main(const std::vector<f8String>& args) override; // required
	bool options_setup() override; // calls Fix8ProApplication::add_options()

	void server_session(SessionInstanceBase_ptr inst, int scnt);
	void client_session(ClientSessionBase_ptr mc);
	bool setup_capture();
	int load_refdata();
	std::ostream& cout() const { return *_cofs.get(); }

public:
	using Fix8ProApplication::Fix8ProApplication;
	~Application() = default;

	friend class SimpleSession;
};

//-----------------------------------------------------------------------------------------
/// Universal session for client and server
/// Note: inheriting from the compiler generated router requires passing the -U flag to the fix8pro compiler f8pc (see CMakeLists.txt)
class SimpleSession final : public Session, FIX44_EXAMPLE_Router
{
	Application& _app { Fix8ProApplication::get_instance<Application>() };
	std::set<f8String> _subscriptions; // marketdata mode
	std::vector<Instruments::value_type> _subscriptions_vec;
	mutable f8_spin_lock _spin_lock;
	f8_atomic<bool> _pause_md { false };
	OrderBook _orderbook;
	Histories _histories;
	std::unique_ptr<std::ofstream> _tofs;

	// required
	bool handle_application(unsigned seqnum, MessagePtr& msg) override;

	// optional overrides
	MessagePtr generate_logon(unsigned heartbeat_interval, const f8String davi) override;
	bool authenticate(SessionID& id, const MessagePtr& msg) override;
	void state_change(States::SessionStates before, States::SessionStates after, const char *where=nullptr) override;
	void reliable_state_change(States::ReliableSessionStates before, States::ReliableSessionStates after, std::any closure, const char *where=nullptr) override;
	void print_message(const MessagePtr& msg, std::ostream& os, bool usecolour) const override;
	void on_send_success(const MessagePtr& msg) const override;

	// order mode
	// ExecutionReport handler (client, order mode)
	bool operator()(const ExecutionReport *msg) override;
	// NewOrderSingle handler (server, order mode), process an order
	bool operator()(const NewOrderSingle *msg) override;

	// market data mode
	// SecurityList handler (client, marketdata mode), handle security list result
	bool operator()(const SecurityList *msg) override;
	// MarketDataSnapshotFullRefresh handler (client, marketdata mode), handle market data
	bool operator()(const MarketDataSnapshotFullRefresh *msg) override;
	// MarketDataIncrementalRefresh handler (client, marketdata mode), handle market data
	bool operator()(const MarketDataIncrementalRefresh *msg) override;
	// MarketDataRequestReject handler (client, marketdata mode), handle market data request reject
	bool operator()(const MarketDataRequestReject *msg) override;
	// SecurityListRequest handler (server, marketdata mode), process security download request
	bool operator()(const SecurityListRequest *msg) override;
	// MarketDataRequest handler (server, marketdata mode), process security data request
	bool operator()(const MarketDataRequest *msg) override;
	// MarketDataHistoryRequest handler (server, marketdata mode), process security history request: User Defined Message
	bool operator()(const MarketDataHistoryRequest *msg) override;
	// MarketDataHistoryFullRefresh handler (server, marketdata mode), process security history: User Defined Message
	bool operator()(const MarketDataHistoryFullRefresh *msg) override;

	template<typename T> bool insert_update(double price, uint32_t qty, T& book);
	template<typename T> bool remove(double price, uint32_t qty, T& book);
	template<typename T> int generate_obsnapshot(const T& book, MessagePtr& mdsfr) const;
	template<typename T> int generate_delta(const T& obook, const T& nbook, const f8String& sym, MessagePtr& mdir) const;
	template<typename T> int match(double price, uint32_t& qty, T& book, const f8String& sym, MessagePtr& mdir);
	template<typename T> int populate_mdentry(char tag, T what, MessagePtr& mdsfr) const;
	template<typename T> std::pair<uint32_t, int> populate_tob(T& book, MessagePtr& mdsfr) const;
	int populate_trade(double price, uint32_t qty, MessagePtr& mdsfr) const;
	bool create_snapshots();
	bool create_mdsnapshot(const f8String& sym);
	bool create_obsnapshot(const f8String& sym);
	bool pregenerate_send_marketdata();
	bool update_history(double price, uint32_t qty, const f8String& sym);

	// misc
	void print_summary(const MessagePtr& msg, bool way) const;

public:
	using Session::Session;
	~SimpleSession() = default;

	bool generate_send_order();
	bool generate_send_marketdata();
	bool send_security_list_request();
	bool has_subscriptions() const;
	bool request_history();
	void set_mprint(int way=0); // 0=toggle 1=on 2=off
	void unsubscribe();
};

//-----------------------------------------------------------------------------------------
namespace
{
	constexpr const int MaxRptElPerMsg = 20;

	f8String make_id(int val)
	{
		std::ostringstream ostr;
		ostr << std::setw(8) << std::setfill('0') << val;
		return std::move(ostr.str());
	}

	char get_wait_key(int waitms=0)
	{
		char ch = '\0';
		struct pollfd pfds { 0, POLLIN };
		if (poll(&pfds, 1, waitms) > 0 && pfds.revents & POLLIN)
		{
			std::cin.get(ch);
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		return ch;
	}

	void toggle(const f8String& tag, bool& what, std::ostream& os)
	{
		os << tag << ' ' << ((what ^= true) ? "on" : "off") << std::endl;
	}

	inline f8String strip(const f8String& from)
	{
		constexpr const char *ws = " \t";
		f8String result;
		if (const size_t bgstr(from.find_first_not_of(ws)); bgstr != f8String::npos)
			result = from.substr(bgstr, from.find_last_not_of(ws) - bgstr + 1);
		return std::move(result);
	}
}

//-----------------------------------------------------------------------------------------
class teebuf : public std::streambuf // courtesy Nicolai M. Josuttis
{
	// This tee buffer has no buffer. So every character "overflows"
	// and can be put directly into the teed buffers.
	// Sync both teed buffers.
	virtual int sync() override
	{
		const int r1 = _sb1->pubsync(), r2 = _sb2->pubsync();
		return r1 == 0 && r2 == 0 ? 0 : -1;
	}

	// This tee buffer has no buffer. So every character "overflows"
	// and can be put directly into the teed buffers.
	virtual int overflow(int c) override
	{
		if (c == EOF)
			return !EOF;
		const int r1 = _sb1->sputc(c), r2 = _sb2->sputc(c);
		return r1 == EOF || r2 == EOF ? EOF : c;
	}

protected:
	std::streambuf *_sb1, *_sb2;

public:
	// Construct a streambuf which tees output to both input
	// streambufs.
	teebuf(std::streambuf *sb1, std::streambuf *sb2) : _sb1(sb1), _sb2(sb2) {}
};

//-----------------------------------------------------------------------------------------
class filtercolourteebuf : public teebuf // filters colour escape sequences
{
	bool ison{};
	int overflow(int c) override
	{
		if (c == EOF)
			return !EOF;
		const int r1 = _sb1->sputc(c);
		int r2 = 0;
		if (ison)
		{
			if ((c & 0xff) == 'm')
				ison = false;
		}
		else if ((c & 0xff) == 0x1b)
			ison = true;
		else
			r2 = _sb2->sputc(c);
		return r1 == EOF || r2 == EOF ? EOF : c;
	}

public:
	using teebuf::teebuf;
};

//-----------------------------------------------------------------------------------------
template<typename T>
class teestream : public std::ostream
{
	T tbuf;

public:
	teestream(std::ostream& o1, std::ostream& o2)
		: std::ostream(&tbuf), tbuf(o1.rdbuf(), o2.rdbuf()) {}
};

//-----------------------------------------------------------------------------------------

#endif // FIX8PRO_SIMPLECLISRV_HPP_

