//-----------------------------------------------------------------------------------------
/*
    ____                      __      ____
   /\  _`\   __             /'_ `\   /\  _`\
   \ \ \L\_\/\_\    __  _  /\ \L\ \  \ \ \L\ \ _ __    ___
    \ \  _\/\/\ \  /\ \/'\ \/_> _ <_  \ \ ,__//\`'__\ / __`\
     \ \ \/  \ \ \ \/>  </   /\ \L\ \  \ \ \/ \ \ \/ /\ \L\ \
      \ \_\   \ \_\ /\_/\_\  \ \____/   \ \_\  \ \_\ \ \____/
       \/_/    \/_/ \//\/_/   \/___/     \/_/   \/_/  \/___/

               Fix8Pro FIX Engine and Framework

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

// f8 headers
#include <fix8pro/f8includes.hpp>
#include <fix8pro/utils/static_application.hpp>

#include <fix8pro/utils/colours.hpp>
#define COLOUR(x,y,z) Colours::make_string16({Attribute::x, Colour::y},z,Application::use_colour())

//-----------------------------------------------------------------------------------------
#include <FIX42_EXAMPLE_types.hpp>
#include <FIX42_EXAMPLE_router.hpp>
#include <FIX42_EXAMPLE_classes.hpp>

//-----------------------------------------------------------------------------------------
using namespace cxxopts;
using namespace FIX8;
using namespace FIX42_EXAMPLE;

//-----------------------------------------------------------------------------------------
class Application final : public Fix8ProApplication
{
	int _giveupreset, _interval;
	bool _server, _reliable, _hb, _quiet, _show_states, _generate;
	f8String _libdir, _clcf, _global_logger_name, _sses, _cses, _libpath, _tname;
	unsigned _next_send, _next_receive;
	std::mt19937_64 _eng = create_seeded_mersenne_engine(); // provided by fix8pro

	int main(const std::vector<f8String>& args) override; // required
	bool options_setup(cxxopts::Options& ops) override;

	void server_session(SessionInstanceBase_ptr inst, int scnt);
	void client_session(ClientSessionBase_ptr mc);

public:
	using Fix8ProApplication::Fix8ProApplication;
	~Application() = default;

	friend class SimpleSession;
};

//-----------------------------------------------------------------------------------------
// We're using Fix8ProApplication so we need to declare the instance
Fix8ProApplicationInstance(Application, "simpleclisrv", Fix8Pro::copyright_string("v2.1"), "Fix8Pro sample client/server");

//-----------------------------------------------------------------------------------------
/// Universal session for client and server
/// Note: inheriting from the compiler generated router requires passing the -U flag to the fix8pro compiler f8pc (see CMakeLists.txt)
class SimpleSession final : public Session, FIX42_EXAMPLE_Router
{
	Application& _app { Fix8ProApplicationInstance::get() };

	// ExecutionReport handler (client)
	bool operator()(const ExecutionReport *msg) override;
	// NewOrderSingle handler (server)
	bool operator()(const NewOrderSingle *msg) override;

	// required
	bool handle_application(unsigned seqnum, MessagePtr& msg) override;

	// optional overrides
	void state_change(States::SessionStates before, States::SessionStates after, const char *where=nullptr) override;
	void reliable_state_change(States::ReliableSessionStates before, States::ReliableSessionStates after, std::any closure, const char *where=nullptr) override;
	void print_message(const MessagePtr& msg, std::ostream& os, bool usecolour) const override;
	void on_send_success(const MessagePtr& msg) const override;
	MessagePtr generate_logon(unsigned heartbeat_interval, const f8String davi) override;

public:
	using Session::Session;
	~SimpleSession() = default;
};

//-----------------------------------------------------------------------------------------
namespace
{
	std::string make_id(int val)
	{
		std::ostringstream ostr;
		ostr << std::setw(8) << std::setfill('0') << val;
		return std::move(ostr.str());
	}

	char get_wait_key(timeval wait={})
	{
		char ch = '\0';
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		if (select(1, &rfds, 0, 0, &wait) > 0)
		{
			std::cin.get(ch);
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		return ch;
	}
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool Application::options_setup(Options& ops)
{
	ops.add_options() // see cxxopts (https://github.com/jarro2783/cxxopts) for info about how Options work
		("c,config", "xml config (default: simple_client.xml or simple_server.xml)", value<f8String>(_clcf))
		("l,log", "global log filename (default: ./run/client_%{DATE}_global.log or ./run/server_%{DATE}_global.log)", value<f8String>(_global_logger_name))
		("V,serversession", "name of server session profile in xml config to use", value<f8String>(_sses)->default_value("SRV"))
		("C,clientsession", "name of client session profile in xml config to use", value<f8String>(_cses)->default_value("CLI"))
		("q,quiet", "do not print fix output", value<bool>(_quiet)->default_value("false"))
		("R,receive", "set next expected receive sequence number", value<unsigned>(_next_receive)->default_value("0"))
		("S,send", "set next expected send sequence number", value<unsigned>(_next_send)->default_value("0"))
		("g,giveupreset", "number of reliable reconnects to try before resetting seqnums", value<int>(_giveupreset)->default_value("10"))
		("r,reliable", "start in reliable mode", value<bool>(_reliable)->default_value("true")->implicit_value("false"))
		("G,generate", "generate NewOrderSingle messages (client)", value<bool>(_generate)->default_value("true")->implicit_value("false"))
		("I,interval", "generate interval (client, msecs)", value<int>(_interval)->default_value("5000"))
		("H,showheartbeats", "show inbound heartbeats", value<bool>(_hb)->default_value("true")->implicit_value("false"))
		("L,libpath", "library path to load Fix8 schema object, default path or LD_LIBRARY_PATH", value<f8String>(_libdir))
		("T,threadname", "prefix thread names with given string", value<f8String>(_tname))
		("t,states", "show session and reliable session thread state changes", value<bool>(_show_states)->default_value("true")->implicit_value("false"))
		("s,server", "run in server mode (default client mode)", value<bool>(_server)->default_value("false"));
	add_postamble(R"(e.g.
  simpleclisrv -c config/simple_server.xml -s
  simpleclisrv -c config/simple_client.xml
  simpleclisrv -015678vhD)");
	return true;
}

//-----------------------------------------------------------------------------------------
int Application::main(const std::vector<f8String>& args)
{
	try
	{
		using namespace std::string_literals;
		const f8String srvtype = _server ? "server" : "client";
		if (!has("config"))
			_clcf = "simple_"s + srvtype + ".xml";
		if (!exist(_clcf))
			throw InvalidConfiguration(_clcf + " not found");
		std::unique_ptr<std::istream> istr = std::make_unique<std::ifstream>(_clcf.c_str()); // our config file as a stream

		void *handle = nullptr;
		f8String errstr;
		auto [ctxfunc, _libpath] = load_cast_ctx_from_so("FIX42", _libdir, handle, errstr); // load our FIX schema .so
		if (!ctxfunc)
			throw BadSharedLibrary(errstr);
		std::cout << "loaded: " << _libpath << std::endl;

		f8String glogname = "./run/" + srvtype;
		if (!has("log"))
			glogname += "_%{DATE}_global.log;latest_" + srvtype + "_global.log"; // logger expands DATE
		else
			glogname += _global_logger_name + ";latest_" + srvtype + '_' + _global_logger_name; // logger adds automatic symlink to latest

		if (has("threadname"))
			Fix8Pro::base_application_thread_name(_tname); // easier to view threads in ps/top/htop etc
		Fix8ProInstance fix8pro_instance (1, glogname.c_str()); // warms up timer, setup logmanager, global logger

		if (_server)
		{
			ServerSessionBase_ptr ms = std::make_unique<ServerSession<SimpleSession>>(ctxfunc(), *istr, _sses);
			constexpr f8String_view prompt {"Waiting for new connection (q=quit)..."};
			std::cout << prompt << std::endl;

			for (unsigned scnt = 0; !term_received && get_wait_key() != 'q';)
			{
				if (ms->poll()) // default timeout 250ms
				{
					// we have a new session
					SessionInstanceBase_ptr srv (ms->create_server_instance());
					server_session(std::move(srv), ++scnt);
					std::cout << "Client session(" << scnt << ") finished. " << prompt << std::endl;
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
	catch (const f8Exception& e)
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
		std::cout << "terminated." << std::endl;
	return 0;
}

//-----------------------------------------------------------------------------------------
void Application::server_session(SessionInstanceBase_ptr inst, int scnt)
{
	auto *ses = inst->session_t_ptr<SimpleSession>(); // obtains a pointer to your session
	if (!_quiet)
		ses->control() |= (_hb ? Session::print : Session::printnohb); // turn on the built-in FIX printer
	std::cout << "Client session(" << scnt << ") connection established." << std::endl;
	inst->start(false, _next_send, _next_receive); // when false, session starts and control returns immediately
	while (!ses->is_shutdown() && ses->get_session_state() != States::st_logoff_sent && ses->get_connection()
		&& ses->get_connection()->is_connected() && !term_received)
	{
		switch (get_wait_key({0, 100 * 1000})) // 100ms
		{
		case 'l':
			ses->logout_and_shutdown("goodbye from server");
			break;
		case 'q':
			ses->control() |= Session::shutdown;
			break;
		case 'x':
			exit(1);
		case 'Q':
			ses->control() ^= (_hb ? Session::print : Session::printnohb);
			std::cout << "quiet " << std::boolalpha << (_quiet ^= true) << std::endl;
			break;
		case '?':
			std::cout << "l - logout\nq - disconnect (no logout)\nx - just exit\nQ - toggle quiet\n? - help" << std::endl;
			break;
		default:
			break;
		}
	}
	ses->request_stop();
}

//-----------------------------------------------------------------------------------------
void Application::client_session(ClientSessionBase_ptr mc)
{
	auto *ses = mc->session_t_ptr<SimpleSession>(); // obtains a pointer to your session
	if (!_quiet)
		ses->control() |= (_hb ? Session::print : Session::printnohb); // turn on the built-in FIX printer
	if (ses->get_login_parameters()._reliable)
		std::cout << "starting reliable client" << std::endl; // reliable is default
	mc->start(false, _next_send, _next_receive, ses->get_login_parameters()._davi); // when false, session starts and control returns immediately
	const timeval wait { _interval / 1000L, (_interval % 1000L) * 1000L };
	for (bool ok = true; ok && !term_received && !mc->has_given_up(); )
	{
		switch (get_wait_key({wait}))
		{
		case 'l':
			ses->logout_and_shutdown("goodbye from client");
			[[fallthrough]];
		case 'q':
			ok = false;
			break;
		case 'x':
			exit(1);
		case 'g':
			std::cout << "generate " << std::boolalpha << (_generate ^= true) << std::endl;
			break;
		case 'Q':
			ses->control() ^= (_hb ? Session::print : Session::printnohb);
			std::cout << "quiet " << std::boolalpha << (_quiet ^= true) << std::endl;
			break;
		case '?':
			std::cout << "l - logout and quit\nq - quit (no logout)\nx - just exit\ng - toggle generate\nQ - toggle quiet\n? - help" << std::endl;
			break;
		default:
			break;
		case 0:
			if (_generate)
			{
				static unsigned oid = 0;
				auto nos = make_message<NewOrderSingle>();
				*nos << nos->make_field<TransactTime>() // defaults to now
					  << nos->make_field<EffectiveTime>(Tickval(2022, 4, 23, 10, 21, 1, 865765000), TimeIndicator::_with_us) // default is _with_ms
					  << nos->make_field<ClOrdID>('C' + tp_to_string(Tickval(true).get_time_point(), R"(%Y%m%d)") + make_id(++oid))
					  << nos->make_field<HandlInst>(HandlInst::AutomatedExecutionNoIntervention)
					  << nos->make_field<OrderQty>(std::uniform_int_distribution<>(1, 100)(_eng)) // random qty
					  << nos->make_field<Price>(std::cauchy_distribution(121.78, 0.1)(_eng), 3)	// using more realistic cauchy 'fat tail'; 3 decimal places if necessary
					  << nos->make_field<Symbol>("NYSE::IBM")
					  << nos->make_field<OrdType>(OrdType::Limit)
					  << nos->make_field<Side>(std::bernoulli_distribution()(_eng) ? Side::Buy : Side::Sell) // coin toss side
					  << nos->make_field<TimeInForce>(std::uniform_int_distribution<>(0, TimeInForce::count - 1)(_eng) + '0'); // random tif
				ses->send(std::move(nos));
			}
			break;
		}
	}
	ses->request_stop();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
/// This is way we typically handle application messages
bool SimpleSession::handle_application(unsigned seqnum, MessagePtr& msg)
{
	return enforce(seqnum, msg) || msg->process(*this); // process calls appropriate FIX message handler
}

//-----------------------------------------------------------------------------------------
/// This allows us to specialise the logon (not needed for this example)
MessagePtr SimpleSession::generate_logon(unsigned heartbeat_interval, const f8String davi)
{
	auto msg = Session::generate_logon(heartbeat_interval, davi);
	return msg;
}

//-----------------------------------------------------------------------------------------
void SimpleSession::print_message(const MessagePtr& msg, std::ostream& os, bool usecolour) const
{
	std::cout << "\r-------------------- received --------------------\n";
	Session::print_message(msg, std::cout, Application::use_colour());
}

//-----------------------------------------------------------------------------------------
void SimpleSession::on_send_success(const MessagePtr& msg) const
{
	if ((_control.has(Session::printnohb) && msg->get_msgtype() == MsgType::Heartbeat)
		|| !_control.has(Session::print));
	else
	{
		std::cout << "\r---------------------- sent ----------------------\n";
		Session::print_message(msg, std::cout, Application::use_colour());
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
			std::cout << "   ";
		std::cout << state_colours[before] << " => " << state_colours[after];
	//	if (where)
	//		std::cout << " (" << where << ')';
		std::cout << std::endl;
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
// Called by framework when the ReliableSession state changes - the reliability thread
// Ensures a reliable client stays connected
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
		std::cout << state_colours[before] << " => " << state_colours[after];
		if (closure.has_value())
			std::cout << " (" << std::any_cast<unsigned>(closure) << ") attempt(s)";
	//	if (where)
	//		std::cout << " (" << where << ')';
		std::cout << std::endl;
	}
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::operator()(const ExecutionReport *msg)
{
	// client processing for received ERs
	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSession::operator()(const NewOrderSingle *msg)
{
	// server processing for received NOSs
	static unsigned oid = 0, eoid = 0;
	OrderQty::this_type qty = msg->get<OrderQty>()->get();
	Price::this_type price = msg->get<Price>()->get();
	auto er = make_message<ExecutionReport>();
	MessageBasePtr erb = detail::static_pointer_cast(er);
	msg->copy_legal(erb); // copy all fields legal for ER from NOS
	const auto tpstr = tp_to_string(Tickval(true).get_time_point(), R"(%Y%m%d)");
	const auto oidstr = 'O' + tpstr + make_id(++oid);

	*er << er->make_field<OrderID>(oidstr)
		 << er->make_field<ExecTransType>(ExecTransType::New)
		 << er->make_field<ExecID>('E' + tpstr + make_id(++eoid))
		 << er->make_field<CumQty>(0.)
		 << er->make_field<AvgPx>(0.)
		 << er->make_field<LastCapacity>(LastCapacity::Principal);

	if (std::bernoulli_distribution(0.80)(_app._eng)) // 80% accepted, 20% rejected
	{
		*er << er->make_field<OrdStatus>(OrdStatus::New)
			 << er->make_field<LeavesQty>(qty)
			 << er->make_field<ExecType>(ExecType::New);
		send(move(er));

		for (OrderQty::this_type remaining_qty = qty, cum_qty = 0; remaining_qty > 0;)
		{
			auto trdqty = std::uniform_int_distribution<>(1, remaining_qty)(_app._eng);
			er = make_message<ExecutionReport>();
			erb = detail::static_pointer_cast(er);
			msg->copy_legal(erb);
			cum_qty += trdqty;
			*er << er->make_field<OrderID>(oidstr)
				 << er->make_field<ExecID>('F' + tpstr + make_id(++eoid))
				 << er->make_field<ExecType>(remaining_qty == trdqty ? ExecType::Fill : ExecType::PartialFill)
				 << er->make_field<OrdStatus>(remaining_qty == trdqty ? OrdStatus::Filled : OrdStatus::PartiallyFilled)
				 << er->make_field<LeavesQty>(remaining_qty - trdqty)
				 << er->make_field<ExecTransType>(ExecTransType::New)
				 << er->make_field<CumQty>(cum_qty)
				 << er->make_field<LastShares>(trdqty)
				 << er->make_field<AvgPx>(price, 3); // to 3 decimal places

			// add some repeating groups
			const auto& nocb = er->find_group<ExecutionReport::NoContraBrokers>();
			int ncnt = 0;
			for (auto trdqtycb = trdqty; trdqtycb > 0; ++ncnt)
			{
				constexpr const std::array broker_nms { "BRTS", "KLMR", "NYVY", "NXPR", "SIMM", "STANS", "AVR1", "AVR2", "HULV", "HULZ", "CAMS", "ORTA" };
				auto gr1 = nocb->create_group();
				auto retrdqtycb = std::uniform_int_distribution<>(1, trdqtycb)(_app._eng);
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
	else
	{
		*er << er->make_field<OrdStatus>(OrdStatus::Rejected)
			 << er->make_field<LeavesQty>(0)
			 << er->make_field<OrdRejReason>(std::uniform_int_distribution<>(0, OrdRejReason::count - 1)(_app._eng)) // random orr
			 << er->make_field<ExecType>(ExecType::Rejected);
		send(move(er));
	}

	return true;
}

