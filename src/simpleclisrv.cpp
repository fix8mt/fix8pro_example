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

#include <fix8pro/utils/colours.hpp>
#define COLOUR(x,y,z) Colours::make_string16({Attribute::x, Colour::y},z,Application::use_colour())

#include <simpleclisrv.hpp>

//-----------------------------------------------------------------------------------------
// We're using Fix8ProApplication so we need to declare the instance
Fix8ProApplicationInstance(Application, "simpleclisrv", Fix8Pro::copyright_string("v2.3"), "Fix8Pro sample client/server");

//-----------------------------------------------------------------------------------------
const Instruments Application::_staticdata
{
	{ "AAPL:NASDAQ",	{ 163.17,	0.,	50 } },	{ "MSFT:NASDAQ",	{ 289.86,	0.,	50 } },
	{ "GOOG:NASDAQ",	{ 2642.44,	0.,	100 } },	{ "AMZN:NASDAQ",	{ 2912.82,	0.,	100 } },
	{ "TSLA:NASDAQ",	{ 838.29,	0.,	120 } },	{ "MMM:NYSE",		{ 149.5,		0.,	120 } },
	{ "FB:NASDAQ",		{ 200.06,	0.,	120 } },	{ "NVDA:NASDAQ",	{ 229.36,	0.,	120 } },
	{ "UNH:NYSE",		{ 498.65,	0.,	120 } },	{ "JNJ:NYSE",		{ 169.48,	0.,	120 } },
	{ "V:NYSE",			{ 200.29,	0.,	120 } },	{ "JPM:NYSE",		{ 134.40,	0.,	300 } },
	{ "WMT:NYSE",		{ 142.82,	0.,	300 } },	{ "PG:NYSE",		{ 155.14,	0.,	300 } },
	{ "XOM:NYSE",		{ 84.09,		0.,	300 } },	{ "HD:NYSE",		{ 324.26,	0.,	300 } },
	{ "BAC:NYSE",		{ 40.95,		0.,	200 } },	{ "MC:NYSE",		{ 330.76,	0.,	200 } },
	{ "CVX:NYSE",		{ 158.65,	0.,	200 } },	{ "PFE:NYSE",		{ 48.65,		0.,	200 } },
};

//-----------------------------------------------------------------------------------------
/// Fix8ProApplication CLI options setup
bool Application::options_setup()
{
	add_options() // see cxxopts (https://github.com/jarro2783/cxxopts) for info about how Options work
		("b,brownopts", "set the Brownian options (drift,volume,lpf) parameters", value<std::vector<double>>(_brown_opts)->default_value("0.01,20.0,0.025"))
		("c,config", "xml config (default: simple_client.xml or simple_server.xml)", value<f8String>(_clcf))
		("d,depth", "use with market data mode, set maximum depth to request on subscription", value<int>(_depth)->default_value("10"))
		("f,refdata", "specify alternate security reference data", value<f8String>(_reffile))
		("g,giveupreset", "number of reliable reconnects to try before resetting seqnums", value<int>(_giveupreset)->default_value("10"))
		("k,capture", "capture all screen output to specified file", value<f8String>(_capfile))
		("l,log", "global log filename (default: ./run/client_%{DATE}_global.log or ./run/server_%{DATE}_global.log)", value<f8String>(_global_logger_name))
		("m,marketdata", "run in marketdata mode (default order mode)", value<bool>(_mdata)->default_value("false"))
		("n,numsec", "maximum number of securities to use (default no limit)", value<int>(_maxsec)->default_value("0"))
		("q,quiet", "do not print fix output", value<bool>(_quiet)->default_value("false"))
		("r,reliable", "start in reliable mode", value<bool>(_reliable)->default_value("true")->implicit_value("false"))
		("s,server", "run in server mode (default client mode)", value<bool>(_server)->default_value("false"))
		("t,states", "show session and reliable session thread state changes", value<bool>(_show_states)->default_value("true")->implicit_value("false"))
		("u,summary", "run in summary display mode", value<bool>(_summary)->default_value("false"))
		("C,clientsession", "name of client session profile in xml config to use", value<f8String>(_cses)->default_value("CLI"))
		("G,generate", "generate NewOrderSingle(client) or market data(server) messages", value<bool>(_generate)->default_value("true")->implicit_value("false"))
		("H,showheartbeats", "show inbound heartbeats", value<bool>(_hb)->default_value("true")->implicit_value("false"))
		("I,interval", "generation interval (msecs); if -ve choose random interval between 0 and -(n)", value<int>(_interval)->default_value("5000"))
		("K,tickcapture", "capture all trade ticks to specified file", value<f8String>(_tickfile))
		("L,libpath", "library path to load Fix8 schema object, default path or LD_LIBRARY_PATH", value<f8String>(_libdir))
		("P,password", "FIX password used in logon (cleartext)", value<f8String>(_password)->default_value("password"))
		("R,receive", "set next expected receive sequence number", value<unsigned>(_next_receive)->default_value("0"))
		("S,send", "set next expected send sequence number", value<unsigned>(_next_send)->default_value("0"))
		("T,threadname", "prefix thread names with given string", value<f8String>(_tname))
		("U,username", "FIX username used in logon", value<f8String>(_username)->default_value("testuser"))
		("V,serversession", "name of server session profile in xml config to use", value<f8String>(_sses)->default_value("SRV"));

	add_postamble(R"(examples:
cli/srv pair:
   simpleclisrv -c config/simple_server.xml -s
   simpleclisrv -c config/simple_client.xml
cli/srv pair with supplied hash pw, random generation interval (~1s), base thread named, run server in summary mode:
   simpleclisrv -sc ../config/simple_server.xml -P 5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8 -u -T clisrv
   simpleclisrv -c ../config/simple_client.xml -I -1000 -P 5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8 -u -T clisrv
cli/srv pair running in market data mode, load refdata from file, random generation interval (~1s), client depth 30 levels:
   simpleclisrv -sc ../config/simple_server.xml -u -T clisrv -f ../config/sample_ref_data.csv
   simpleclisrv -c ../config/simple_client.xml -I -1000 -u -T clisrv -d 30)");

	return true;
}

//-----------------------------------------------------------------------------------------
/// Fix8ProApplication entry point
int Application::main(const std::vector<f8String>& args)
{
	try
	{
		// optional teestream (capture screen to file)
		setup_capture();

		// load config
		const f8String srvtype = _server ? "server" : "client";
		if (!has("config"))
			_clcf = "simple_"s + srvtype + ".xml";
		if (!exist(_clcf))
			throw InvalidConfiguration(_clcf + " not found");
		std::unique_ptr<std::istream> istr = std::make_unique<std::ifstream>(_clcf.c_str()); // our config file as a stream

		// load our FIX schema .so
		void *handle = nullptr;
		f8String errstr;
		auto [ctxfunc, _libpath] = load_cast_ctx_from_so("FIX44_EXAMPLE", _libdir, handle, errstr);
		if (!ctxfunc)
			throw BadSharedLibrary(errstr);
		cout() << "loaded: " << _libpath << std::endl;

		// setup lognames
		f8String glogname = "./run/" + srvtype;
		if (!has("log"))
			glogname += "_%{DATE}_global.log;latest_" + srvtype + "_global.log"; // logger expands DATE
		else
			glogname += _global_logger_name + ";latest_" + srvtype + '_' + _global_logger_name; // logger adds automatic symlink to latest

		// easier to view threads in ps/top/htop etc
		if (has("threadname"))
			Fix8Pro::base_application_thread_name(_tname);

		// warms up timer, setup logmanager, global logger
		Fix8ProInstance fix8pro_instance (1, glogname.c_str());
		glout_info << "Command line was: \"" << get_cmdline() << '"';

		// load securities
		cout() << "loaded: " << load_refdata() << " securities" << std::endl;

		// set Brownian parameters
		_br = std::make_unique<Brownian>(_brown_opts[volume], _brown_opts[lpf]);

		if (_server)
		{
			ServerSessionBase_ptr ms = std::make_unique<ServerSession<SimpleSession>>(ctxfunc(), *istr, _sses);
			constexpr f8String_view prompt = "Waiting for new connection (q=quit)...";
			cout() << prompt << std::endl;

			for (unsigned scnt = 0; !term_received && get_wait_key() != 'q';)
			{
				if (ms->poll()) // default timeout 250ms
				{
					std::thread worker([&]() // run acceptor in new thread (not required, just for example)
					{
						// we have a new session
						SessionInstanceBase_ptr srv (ms->create_server_instance());
						server_session(std::move(srv), scnt++);
						cout() << "Client session(" << scnt << ") finished. " << prompt << std::endl;
					});
					if (worker.joinable())
						worker.join(); // for multiple concurrent clients, we wouldn't block here
				}
			}
		}
		else
		{
			ClientSessionBase_ptr cli = _reliable	? std::make_unique<ReliableClientSession<SimpleSession>>(ctxfunc(), *istr, _cses, _giveupreset)
																: std::make_unique<ClientSession<SimpleSession>>(ctxfunc(), *istr, _cses);
			client_session(std::move(cli));
		}
	}
	catch (const f8Exception& e) // catch fix8pro framework exceptions
	{
		std::cerr << "f8Exception: " << COLOUR(Bold, Red, e.what()) << std::endl;
		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "std::exception: " << COLOUR(Bold, Red, e.what()) << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << "unknown exception" << std::endl;
		return 1;
	}

	if (term_received)
		cout() << "terminated." << std::endl;
	return 0;
}

//-----------------------------------------------------------------------------------------
/// Run a server session
void Application::server_session(SessionInstanceBase_ptr inst, int scnt)
{
	set_application_thread_name("worker/" + std::to_string(scnt));
	auto *ses = inst->session_t_ptr<SimpleSession>(); // obtains a pointer to your session
	if (!_quiet)
		ses->control() |= (_hb ? Session::print : Session::printnohb); // turn on the built-in FIX printer
	cout() << "Client session(" << scnt << ") connection established." << std::endl;
	inst->start(false, _next_send, _next_receive); // when false, session starts and control returns immediately
	while (!ses->is_shutdown() && ses->get_session_state() != States::st_logoff_sent && ses->get_connection()
		&& ses->get_connection()->is_connected() && !term_received)
	{
		auto ival = _interval < 0 ? std::uniform_int_distribution<>(0, -_interval)(_eng) : _interval;
		switch (auto ch = get_wait_key(ival); ch)
		{
		case 'l':
			ses->logout_and_shutdown("goodbye from server");
			break;
		case 's':
			toggle("summary", _summary, cout());
			break;
		case 'g':
			if (_mdata)
				toggle("generate", _generate, cout());
			break;
		case 'S':
			toggle("states", _show_states, cout());
			break;
		case 'q':
			ses->control() |= Session::shutdown;
			break;
		case 'x':
			exit(1);
		case 'Q':
			ses->set_mprint();
			toggle("quiet", _quiet, cout());
			break;
		default:
			cout() << COLOUR(Bold, Red, "unknown cmd: ") << (isprint(ch) ? ch : ' ') << std::endl;
			[[fallthrough]];
		case '?':
			cout() <<
R"(l - logout
s - toggle summary
q - disconnect (no logout)
x - just exit
Q - toggle quiet
S - toggle states
? - help)" << std::endl;
			if (_mdata)
				cout() <<
R"(g - toggle generate)" << std::endl;
			break;
		case 0:
			if (_mdata && _generate && ses->get_session_state() == States::st_continuous && ses->has_subscriptions())
				ses->generate_send_marketdata();  // marketdata mode
			break;
		}
	}
	ses->request_stop();
}

//-----------------------------------------------------------------------------------------
/// Run a client session
void Application::client_session(ClientSessionBase_ptr mc)
{
	auto *ses = mc->session_t_ptr<SimpleSession>(); // obtains a pointer to your session
	if (!_quiet)
		ses->control() |= (_hb ? Session::print : Session::printnohb); // turn on the built-in FIX printer
	if (ses->get_login_parameters()._reliable)
		cout() << "starting reliable client" << std::endl; // reliable is default
	mc->start(false, _next_send, _next_receive, ses->get_login_parameters()._davi); // when false, session starts and control returns immediately
	for (auto ok = true; ok && !term_received && !mc->has_given_up(); )
	{
		auto ival = _interval < 0 ? std::uniform_int_distribution<>(0, -_interval)(_eng) : _interval;
		switch (auto ch = get_wait_key(ival); ch)
		{
		case 'l':
			ses->logout_and_shutdown("goodbye from client");
			[[fallthrough]];
		case 'q':
			ok = false;
			break;
		case 'x':
			exit(1);
		case 's':
			toggle("summary", _summary, cout());
			break;
		case 'S':
			toggle("states", _show_states, cout());
			break;
		case 'Q':
			ses->set_mprint();
			toggle("quiet", _quiet, cout());
			break;
		case 'g':
			if (!_mdata)
				toggle("generate", _generate, cout());
			break;
		default:
			cout() << COLOUR(Bold, Red, "unknown cmd: ") << (isprint(ch) ? ch : ' ') << std::endl;
			[[fallthrough]];
		case '?':
			cout() <<
R"(l - logout and quit
q - quit (no logout)
x - just exit
s - toggle summary
Q - toggle quiet
S - toggle states
? - help)" << std::endl;
			if (_mdata)
				cout() <<
R"(G - resubscribe
u - unsubscribe
m - request history)" << std::endl;
			else
				cout() <<
R"(g - toggle generate)" << std::endl;
			break;
		case 'm':
			if (_mdata)
				ses->request_history();
			break;
		case 'u':
		case 'G':
			if (!_mdata)
				break;
			ses->unsubscribe();
			if (ch == 'u')
				break;
			[[fallthrough]];
		case 0:
			if (_generate && ses->get_session_state() == States::st_continuous)
			{
				if (_mdata)
				{
					if (!ses->has_subscriptions())
						ses->send_security_list_request(); // initiate subscriptions
				}
				else
					ses->generate_send_order();  // order mode
			}
			break;
		}
	}
	ses->request_stop();
}

//-----------------------------------------------------------------------------------------
/// Setup optional capture screen to disk
bool Application::setup_capture()
{
	if (has("capture") && (_ofptr = std::unique_ptr<std::ostream>(new std::ofstream(_capfile.c_str(), std::ios::trunc))))
	{
		if (use_colour()) // filter colourised output
			_cofs = std::unique_ptr<std::ostream, f8_deleter>(new teestream<filtercolourteebuf>(std::cout, *_ofptr), f8_deleter(true));
		else
			_cofs = std::unique_ptr<std::ostream, f8_deleter>(new teestream<teebuf>(std::cout, *_ofptr), f8_deleter(true));
		if (_cofs)
			*_cofs << "capturing screen output to " << _capfile << std::endl;
	}
	else
		_cofs = std::unique_ptr<std::ostream, f8_deleter>(&std::cout, f8_deleter(false));
	return static_cast<bool>(_cofs);
}

//-----------------------------------------------------------------------------------------
/// Read security reference data from CSV or load static data
int Application::load_refdata()
{
	if (has("refdata"))
	{
		// # security, refprice, max order qty
		if (std::ifstream ifs(_reffile.c_str()); ifs)
		{
			f8String line;
			int lcnt = 0;
			while (std::getline(ifs, line))
			{
				++lcnt;
				if (strip(line)[0] == '#') // comment
					continue;
				f8String buf;
				std::stringstream pstr;
				for (std::istringstream istr(line); std::getline(istr, buf, ','); pstr << ' ' << strip(buf));
				double price = 0.;
				uint32_t qty = 0;
				pstr >> buf >> price >> qty;
				if (!buf.empty() && price && qty)
				{
					if (!_refdata.emplace(buf, Detail(price, 0., qty)).second)
						std::cerr << "Failed to insert (" << lcnt << ") " << buf << " (duplicate?) - ignoring" << std::endl;
				}
				else
					std::cerr << "Invalid ref record (" << lcnt << ") - ignoring" << std::endl;
			}
			return _refdata.size();
		}
		else
			std::cerr << "Unable to read from ref file \"" << _reffile << "\" - loading static data instead..." << std::endl;
	}
	for (const auto& pp : _staticdata) // use static data
		_refdata.emplace(pp.first, pp.second);
	return _refdata.size();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
/// This is way we typically handle application messages
bool SimpleSession::handle_application(unsigned seqnum, MessagePtr& msg)
{
	return enforce(seqnum, msg) || msg->process(*this); // process calls appropriate FIX message handler
}

//-----------------------------------------------------------------------------------------
/// This allows us to specialise the logon
MessagePtr SimpleSession::generate_logon(unsigned heartbeat_interval, const f8String davi)
{
	auto msg = Session::generate_logon(heartbeat_interval, davi);
	*msg	<< msg->make_field<Username>(_app._username)
			<< msg->make_field<Password>(_app._password);
	return msg;
}

//-----------------------------------------------------------------------------------------
/// Intercept server authentication
bool SimpleSession::authenticate(SessionID& id, const MessagePtr& msg)
{
	return msg->has<Username>() && msg->has<Password>()
		&& msg->get<Username>()->get() == _app._username && msg->get<Password>()->get() == _app._password;
}

//-----------------------------------------------------------------------------------------
/// Generate an order randomly from a list of instruments
bool SimpleSession::generate_send_order()
{
	static unsigned oid = 0;
	auto itr = std::next(_app._refdata.begin(), std::uniform_int_distribution<>(0, _app._refdata.size() - 1)(_app._eng));

	itr->second._lastrv = _app._br->generate(itr->second._lastrv, std::uniform_real_distribution()(_app._eng));
	itr->second._ref = roundtoplaces<2>(itr->second._ref + itr->second._ref * _app._brown_opts[drift] * itr->second._lastrv);

	auto nos = make_message<NewOrderSingle>();
	*nos << nos->make_field<TransactTime>() // defaults to now
		  << nos->make_field<Symbol>(itr->first)
		  << nos->make_field<ClOrdID>('C' + tp_to_string(Tickval(true).get_time_point(), R"(%Y%m%d)") + make_id(++oid))
		  << nos->make_field<HandlInst>(std::uniform_int_distribution<>(1, HandlInst::count)(_app._eng) + '0') // random hi
		  << nos->make_field<OrderQty>(std::uniform_int_distribution<>(1, itr->second._qty)(_app._eng)) // random qty
		  << nos->make_field<Price>(itr->second._ref, 2)
		  << nos->make_field<OrdType>(OrdType::Limit)
		  << nos->make_field<Side>(std::bernoulli_distribution()(_app._eng) ? Side::Buy : Side::Sell) // coin toss side
		  << nos->make_field<TimeInForce>(std::uniform_int_distribution<>(0, TimeInForce::count - 1)(_app._eng) + '0'); // random tif
	return send(std::move(nos));
}

//-----------------------------------------------------------------------------------------
/*
Recv 2022-03-17 08:33:22.250000000 ExecutionReport (8) seq=2      FB    S 9 @ 200.14 New:NASDAQ
Sent 2022-04-06 08:54:53.946632855 MarketDataIncrementalRefresh(X) seq=22510
   INTC:NASDAQ  Change  Offer                    386 pos=1 4
   JPM:NYSE     Change  Offer                    557 pos=1 4
   TMO:NASDAQ   Change  Bid                      316 pos=2 2
   AMD:NASDAQ   Change  Offer                    238 pos=1 4
   GE:NASDAQ    New     Trade                     32 @ 92.02
   MDT:NASDAQ   Change  Offer                    434 pos=1 3
   AMD:NASDAQ   New     Trade                      3 @ 110.53
   AMZN:NASDAQ  New     Trade                     85 @ 2912.82
   INTU:NASDAQ  New     Trade                    172 @ 505.65
   UNH:NYSE     Delete  Bid                      pos=1
   CHTR:NASDAQ  Change  Offer                    1564 pos=1 8
   SYK:NASDAQ   Change  Bid                      370 pos=1 7
   INTU:NASDAQ  New     Trade                     97 @ 505.65
   INTU:NASDAQ  Change  Bid                      307 pos=1 3
   EL:NASDAQ    New     Offer                     62 @ 278.63 pos=2 1
   NKE:NASDAQ   New     Trade                     17 @ 134.34
   EL:NASDAQ    New     Trade                    176 @ 278.62
   BAC:NYSE     Change  Bid                      625 pos=1 7
 */
void SimpleSession::print_summary(const MessagePtr& msg, bool way) const
{
	static const f8String send = COLOUR(Bold, Magenta, "Sent"), recv = COLOUR(Bold, Cyan, "Recv");
	_app.cout() << (way ? send : recv) << ' ' << msg->Header()->get<SendingTime>()->get()
					<< ' ' << std::left << std::setw(16) << _ctx._bme.find(msg->get_msgtype())->second._name // query the metadata
					<< '(' << msg->get_msgtype() << ") seq=" << std::setw(6) << msg->Header()->get<MsgSeqNum>()->get();
	if (msg->has<Symbol>())
		_app.cout() << ' ' << std::setw(12) << msg->get<Symbol>()->get();
	if (msg->has<Side>())
		_app.cout() << ' ' << (msg->get<Side>()->get() == Side::Buy ? 'B' : 'S');
	if (msg->has<LastQty>())
		_app.cout() << ' ' << msg->get<LastQty>()->get();
	else if (msg->has<OrderQty>())
		_app.cout() << ' ' << msg->get<OrderQty>()->get();
	if (msg->has<Price>())
		_app.cout() << " @ " << msg->get<Price>()->get();
	if (msg->has<OrdStatus>())
		_app.cout() << ' ' << msg->get_field_domain_description<OrdStatus>();
	_app.cout() << std::endl;
	if (const auto& grnors = msg->find_group<MarketDataSnapshotFullRefresh::NoMDEntries>(); grnors) // also finds MarketDataIncrementalRefresh::NoMDEntries
	{
		for (const auto& pp : *grnors)
		{
			bool prevwasvol = false;
			_app.cout() << Application::spacer();
			if (pp->has<Symbol>())
				_app.cout() << std::setw(12) << pp->get<Symbol>()->get() << ' ';
			if (pp->has<MDUpdateAction>())
				_app.cout() << std::setw(7) << pp->get_field_domain_description<MDUpdateAction>() << ' ';
			if (pp->has<MDEntryType>())
				_app.cout() << std::setw(24) << pp->get_field_domain_description<MDEntryType>() << ' ';
			if (pp->has<MDEntrySize>())
			{
				_app.cout() << std::right << std::setw(3) << pp->get<MDEntrySize>()->get() << std::left << ' ';
				prevwasvol = true;
			}
			if (pp->has<MDEntryPx>())
			{
				if (prevwasvol)
					_app.cout() << "@ ";
				_app.cout() << pp->get<MDEntryPx>()->get() << ' ';
			}
			if (pp->has<MDEntryPositionNo>())
				_app.cout() << "pos=" << pp->get<MDEntryPositionNo>()->get() << ' ';
			if (pp->has<NumberOfOrders>())
				_app.cout() << pp->get<NumberOfOrders>()->get() << ' ';
			_app.cout() << std::endl;
		}
	}
}

//-----------------------------------------------------------------------------------------
void SimpleSession::set_mprint(int way)
{
	auto flag = _app._hb ? Session::print : Session::printnohb;
	if (auto curr = _control & flag; way == 0 || (way == 1 && !curr) || (way == 2 && curr))
		_control ^= flag;
}

//-----------------------------------------------------------------------------------------
/// Override Fix8Pro printer
void SimpleSession::print_message(const MessagePtr& msg, std::ostream& os, bool usecolour) const
{
	if (_app._summary)
		print_summary(msg, false);
	else
	{
		_app.cout() << "---------------------- Recv ----------------------\n";
		Session::print_message(msg, _app.cout(), Application::use_colour());
	}
}

//-----------------------------------------------------------------------------------------
/// Override Fix8Pro call to send success method
void SimpleSession::on_send_success(const MessagePtr& msg) const
{
	if ((_control.has(Session::printnohb) && msg->get_msgtype() == MsgType::Heartbeat)
		|| !_control.has(Session::print));
	else if (_app._summary)
		print_summary(msg, true);
	else
	{
		_app.cout() << "---------------------- Sent ----------------------\n";
		Session::print_message(msg, _app.cout(), Application::use_colour());
	}
}

//-----------------------------------------------------------------------------------------
/// Called by framework when the Session state changes
void SimpleSession::state_change(States::SessionStates before, States::SessionStates after, const char *where)
{
	Session::state_change(before, after, where); // In debug logs to session log

	if (_app._show_states)
	{
		static const std::array state_colours
		{
			COLOUR(Bold, Red, get_session_state_string(States::st_none)),
			COLOUR(Bold, Green, get_session_state_string(States::st_continuous)),
			COLOUR(Bold, Red, get_session_state_string(States::st_session_terminated)),
			COLOUR(Bold, Yellow, get_session_state_string(States::st_wait_for_logon)),
			COLOUR(Bold, Cyan, get_session_state_string(States::st_not_logged_in)),
			COLOUR(Bold, Yellow, get_session_state_string(States::st_logon_sent)),
			COLOUR(Bold, Magenta, get_session_state_string(States::st_logon_received)),
			COLOUR(Bold, Magenta, get_session_state_string(States::st_logoff_sent)),
			COLOUR(Bold, Magenta, get_session_state_string(States::st_logoff_received)),
			COLOUR(Bold, Magenta, get_session_state_string(States::st_logoff_sent_and_received)),
			COLOUR(Bold, Yellow, get_session_state_string(States::st_test_request_sent)),
			COLOUR(Bold, Yellow, get_session_state_string(States::st_sequence_reset_sent)),
			COLOUR(Bold, Yellow, get_session_state_string(States::st_sequence_reset_received)),
			COLOUR(Bold, Blue, get_session_state_string(States::st_resend_request_sent)),
			COLOUR(Bold, Blue, get_session_state_string(States::st_resend_request_received))
		};
		if (_loginParameters._reliable) // set by framework if session was started as reliable
			_app.cout() << "   ";
		_app.cout() << state_colours[before] << " => " << state_colours[after];
	//	if (where)
	//		_app.cout() << " (" << where << ')';
		_app.cout() << std::endl;
	}

	// some tweaks to solve typical ReliableClient / Server gripes
	if (before == States::st_logon_sent && after == States::st_logoff_received) // force reliable client to try again even though normal exit was detected
		set_exit_state(false);
	else if (before == States::st_logoff_received && after == States::st_logoff_sent_and_received)
	{
		if (_connection_role == ConnectionRole::cn_acceptor)
			logout_and_shutdown("goodbye");
		else
			set_exit_state(true);
	}
}

//-----------------------------------------------------------------------------------------
/// Called by framework when the ReliableSession state changes - the reliability thread
/// Ensures a reliable client stays connected
void SimpleSession::reliable_state_change(States::ReliableSessionStates before, States::ReliableSessionStates after, std::any closure, const char *where)
{
	Session::reliable_state_change(before, after, closure, where); // In debug logs to session log

	if (_app._show_states)
	{
		static const std::array state_colours
		{
			COLOUR(Reverse, Red, get_reliable_session_state_string(States::rst_none)),
			COLOUR(Reverse, Yellow, get_reliable_session_state_string(States::rst_attempting)),
			COLOUR(Reverse, Red, get_reliable_session_state_string(States::rst_giving_up)),
			COLOUR(Reverse, Red, get_reliable_session_state_string(States::rst_resetting)),
			COLOUR(Reverse, Cyan, get_reliable_session_state_string(States::rst_failover)),
			COLOUR(Reverse, Green, get_reliable_session_state_string(States::rst_connected)),
			COLOUR(Reverse, Cyan, get_reliable_session_state_string(States::rst_disconnected)),
			COLOUR(Reverse, Magenta, get_reliable_session_state_string(States::rst_starting)),
			COLOUR(Reverse, Magenta, get_reliable_session_state_string(States::rst_stopping))
		};
		_app.cout() << state_colours[before] << " => " << state_colours[after];
		if (closure.has_value())
			_app.cout() << " (" << std::any_cast<unsigned>(closure) << ") attempt(s)";
	//	if (where)
	//		_app.cout() << " (" << where << ')';
		_app.cout() << std::endl;
	}
}

//-----------------------------------------------------------------------------------------
/// client processing for received ERs
bool SimpleSession::operator()(const ExecutionReport *msg)
{
	return true;
}

//-----------------------------------------------------------------------------------------
/// server processing for received NOSs
bool SimpleSession::operator()(const NewOrderSingle *msg)
{
	static unsigned oid = 0, eoid = 0;
	OrderQty::this_type qty = msg->get<OrderQty>()->get();
	Price::this_type price = msg->get<Price>()->get();
	auto er = make_message<ExecutionReport>();
	MessageBasePtr erb = detail::static_pointer_cast(er);
	msg->copy_legal(erb); // copy all fields legal for ER from NOS
	const auto tpstr = tp_to_string(Tickval(true).get_time_point(), R"(%Y%m%d)");
	const auto oidstr = 'O' + tpstr + make_id(++oid);

	*er << er->make_field<OrderID>(oidstr)
		 << er->make_field<ExecID>('E' + tpstr + make_id(++eoid))
		 << er->make_field<CumQty>(0.)
		 << er->make_field<AvgPx>(0.)
		 << er->make_field<LastCapacity>(LastCapacity::Principal);

	if (std::bernoulli_distribution(0.80)(_app._eng)) // 80% accepted, 20% rejected
	{
		*er << er->make_field<OrdStatus>(OrdStatus::New)
			 << er->make_field<LeavesQty>(qty)
			 << er->make_field<ExecType>(ExecType::New);
		send(std::move(er));

		if (std::bernoulli_distribution(0.75)(_app._eng)) // 75% trade, 25% rest)
		{
			auto partfill = std::bernoulli_distribution(0.75)(_app._eng); // 75% partially, 25% fully filled
			for (OrderQty::this_type remaining_qty = qty, cum_qty = 0; remaining_qty > 0;)
			{
				auto trdqty = partfill ? std::uniform_int_distribution<>(1, remaining_qty)(_app._eng) : remaining_qty;
				er = make_message<ExecutionReport>();
				erb = detail::static_pointer_cast(er);
				msg->copy_legal(erb);
				cum_qty += trdqty;
				*er << er->make_field<OrderID>(oidstr)
					 << er->make_field<ExecID>('F' + tpstr + make_id(++eoid))
					 << er->make_field<ExecType>(ExecType::Trade)
					 << er->make_field<OrdStatus>(remaining_qty == trdqty ? OrdStatus::Filled : OrdStatus::PartiallyFilled)
					 << er->make_field<LeavesQty>(remaining_qty - trdqty)
					 << er->make_field<CumQty>(cum_qty)
					 << er->make_field<LastQty>(trdqty)
					 << er->make_field<AvgPx>(price, 3); // to 3 decimal places

				// add some repeating groups
				const auto& nocb = er->find_group<ExecutionReport::NoContraBrokers>();
				int ncnt = 0;
				for (auto trdqtycb = trdqty; trdqtycb > 0; ++ncnt)
				{
					constexpr const std::array broker_nms { "BRTS", "KLMR", "NYVY", "NXPR", "SIMM", "STANS", "AVR1", "AVR2", "HULV", "HULZ", "CAMS", "ORTA" };
					auto gr1 = nocb->create_group();
					auto retrdqtycb = std::uniform_int_distribution<>(1, trdqtycb)(_app._eng); // random qty
					*gr1  << er->make_field<ContraBroker>(broker_nms[std::uniform_int_distribution<>(0, broker_nms.size() - 1)(_app._eng)]) // random cb
							<< er->make_field<ContraTradeQty>(retrdqtycb)
							<< er->make_field<ContraTrader>(std::to_string(std::uniform_int_distribution<>(1000, 9999)(_app._eng))); // random ct
					trdqtycb -= retrdqtycb;
					*nocb << std::move(gr1);
				}
				*er << er->make_field<NoContraBrokers>(ncnt);

				send(std::move(er));
				remaining_qty -= trdqty;
			}
		}
	}
	else
	{
		*er << er->make_field<OrdStatus>(OrdStatus::Rejected)
			 << er->make_field<LeavesQty>(0)
			 << er->make_field<OrdRejReason>(std::uniform_int_distribution<>(0, OrdRejReason::count - 1)(_app._eng)) // random orr
			 << er->make_field<ExecType>(ExecType::Rejected);
		send(std::move(er));
	}

	return true;
}

