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

Copyright (C) 2010-21 Fix8 Market Technologies Pty Ltd (ABN 29 167 027 198)
All Rights Reserved. [http://www.fix8mt.com] <heretohelp@fix8mt.com>

This  file is released  under the  GNU LESSER  GENERAL PUBLIC  LICENSE  Version 2.  You can
redistribute  it  and / or modify  it under the  terms of  the  GNU Lesser  General  Public
License as  published  by  the Free  Software Foundation,  either version 2 of the License,
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
using namespace std;
using namespace cxxopts;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
class SimpleSession;

/// universal inbound message router for client and server
class SimpleSessionRouter final : public FIX42_EXAMPLE::FIX42_EXAMPLE_Router
{
	SimpleSession& _session;

public:
	/*! Ctor.
	    \param session client session */
	SimpleSessionRouter(SimpleSession& session) : _session(session) {}

	/*! Execution report handler. For client
	    \param msg Execution report message session */
	bool operator()(const FIX42_EXAMPLE::ExecutionReport *msg) const override;

	/*! NewOrderSingle message handler. For server
	    \param msg NewOrderSingle message */
	bool operator()(const FIX42_EXAMPLE::NewOrderSingle *msg) const override;
};

//-----------------------------------------------------------------------------------------
/// universal session for client and server
class SimpleSession final : public Session
{
	SimpleSessionRouter _router;

public:
	SimpleSession(const F8MetaCntx& ctx, const SessionID& sid, PersisterPtr persist=PersisterPtr(), // client
		LoggerPtr logger=LoggerPtr(), LoggerPtr plogger=LoggerPtr(), SupplementalsPtr&& supplementals=SupplementalsPtr())
		: Session(ctx, sid, persist, logger, plogger, move(supplementals)), _router(*this) {}

	SimpleSession(const F8MetaCntx& ctx, int session_count, const f8String& sci, PersisterPtr persist=PersisterPtr(), // server
		LoggerPtr logger=LoggerPtr(), LoggerPtr plogger=LoggerPtr(), SupplementalsPtr&& supplementals=SupplementalsPtr())
		: Session(ctx, session_count, sci, persist, logger, plogger, move(supplementals)), _router(*this) {}

	bool handle_application(unsigned seqnum, MessagePtr& msg) override;
	void state_change(States::SessionStates before, States::SessionStates after, const char *where=nullptr) override;
	void print_message(const MessagePtr& msg, ostream& os, bool usecolour) const override;
	void on_send_success(const MessagePtr& msg) const override;
	MessagePtr generate_logon(unsigned heartbeat_interval, const f8String davi) override;
};

//-----------------------------------------------------------------------------------------
class Application final : public Fix8ProApplication
{
	f8String libdir;
	int giveupreset;
	bool server, reliable, hb;
	f8String clcf, global_logger_name, sses, cses, libpath;
	unsigned next_send, next_receive;
	static mt19937_64 eng;

	static bool quiet;

public:
	using Fix8ProApplication::Fix8ProApplication;
	~Application() = default;

	int main(const vector<f8String>& args) override;
	bool options_setup(cxxopts::Options& ops) override;

	friend class SimpleSession;
	friend class SimpleSessionRouter;
};

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Fix8ProApplicationInstance(Application, "simpleclisrv", "(your copyright string)", "Fix8Pro sample client/server");
bool Application::quiet;
mt19937_64 Application::eng(time(0));

//-----------------------------------------------------------------------------------------
bool Application::options_setup(Options& ops)
{
	ops.add_options()
		("c,config", "xml config (default: simple_client.xml or simple_server.xml)", value<f8String>(clcf))
		("l,log", "global log filename", value<f8String>(global_logger_name)->default_value("global.log"))
		("V,serversession", "name of server session profile to use", value<f8String>(sses)->default_value("SRV"))
		("C,clientsession", "name of client session profile to use", value<f8String>(cses)->default_value("CLI"))
		("q,quiet", "do not print fix output", value<bool>(quiet)->default_value("false"))
		("R,receive", "set next expected receive sequence number", value<unsigned>(next_receive)->default_value("0"))
		("S,send", "set next expected send sequence number", value<unsigned>(next_send)->default_value("0"))
		("g,giveupreset", "number of reliable reconnects to try before resetting seqnums", value<int>(giveupreset)->default_value("10"))
		("r,reliable", "start in reliable mode", value<bool>(reliable)->default_value("true")->implicit_value("false"))
		("H,showheartbeats", "show inbound heartbeats", value<bool>(hb)->default_value("true")->implicit_value("false"))
		("L,libpath", "library path to load Fix8 schema object, default path or LD_LIBRARY_PATH", value<f8String>(libdir))
		("s,server", "run in server mode (default client mode)", value<bool>(server)->default_value("false"));
	add_postamble(R"(e.g.
  simpleclisrv -c config/simple_client.xml -r
  simpleclisrv -c config/simple_server.xml -s)");
	return true;
}

//-----------------------------------------------------------------------------------------
int Application::main(const vector<f8String>& args)
{
	try
	{
		if (clcf.empty())
			clcf = server ? "simple_client.xml" : "simple_server.xml";
		if (!exist(clcf))
		{
			cerr << "configuration file " << clcf << " does not exist" << endl;
			return 1;
		}
		unique_ptr<istream> istr { make_unique<ifstream>(clcf.c_str()) };

		void *handle{};
		f8String errstr;
		auto [ctxfunc, libp] { load_cast_ctx_from_so("FIX42", libdir, handle, errstr) };
		if (!ctxfunc)
		{
			cerr << errstr << endl;
			return 1;
		}
		cout << "loaded: " << (libpath = libp) << endl;

		Fix8Pro::base_application_thread_name("clisrv");
		const f8String gtype { server ? "server" : "client" }, glogname { "./run/" + gtype + "_%{DATE}_" + global_logger_name + ";latest_" + gtype + '_' + global_logger_name };
		Fix8ProInstance instance{1, glogname.c_str()}; // warms up timer, logmanager

		if (server)
		{
			unique_ptr<ServerSessionBase> ms(make_unique<ServerSession<SimpleSession>>(ctxfunc(), *istr, sses));

			for (unsigned scnt{}; !term_received; )
			{
				if (!ms->poll())
					continue;
				unique_ptr<SessionInstanceBase> inst(ms->create_server_instance());
				auto *ses { static_cast<SimpleSession*>(inst->session_ptr()) };
				if (!quiet)
					ses->control() |= (hb ? Session::print : Session::printnohb);
				cout << "client(" << ++scnt << ") connection established." << endl;
				inst->start(false, next_send, next_receive);
				while (!ses->is_shutdown() && ses->get_session_state() != States::st_logoff_sent && ses->get_connection()
					&& ses->get_connection()->is_connected() && !term_received)
						hypersleep(1s);
				ses->request_stop();
				cout << "Session(" << scnt << ") finished. Waiting for new connection..." << endl;
			}
		}
		else
		{
			unique_ptr<ClientSessionBase> mc(reliable ? make_unique<ReliableClientSession<SimpleSession>>(ctxfunc(), *istr, cses, giveupreset)
																	: make_unique<ClientSession<SimpleSession>>(ctxfunc(), *istr, cses));
			if (!quiet)
				mc->session_ptr()->control() |= (hb ? Session::print : Session::printnohb);

			const LoginParameters& lparam(mc->session_ptr()->get_login_parameters());
			if (!reliable)
				mc->start(false, next_send, next_receive, lparam._davi);
			else
			{
				cout << "starting reliable client" << endl;
				mc->start(false, next_send, next_receive, lparam._davi);
			}
			cout << "Press 'l' to logout and quit, 'q' to quit (no logout), 'x' to just exit" << endl;
			char ch;
			while (!term_received)
			{
				timeval tv{5};
				fd_set rfds;
				FD_ZERO(&rfds);
				FD_SET(0, &rfds);
				if (select(1, &rfds, 0, 0, &tv) > 0 && read(0, &ch, 1) > 0)
				{
					if (ch == 'l')
					{
						mc->session_ptr()->logout_and_shutdown("goodbye");
						break;
					}
					else if (ch == 'q')
						break;
					else if (ch == 'x')
						exit(1);
				}
				static unsigned oid{};
				auto nos(make_message<FIX42_EXAMPLE::NewOrderSingle>());
				*nos << nos->make_field<FIX42_EXAMPLE::TransactTime>()
					  << nos->make_field<FIX42_EXAMPLE::ClOrdID>("ord" + to_string(++oid))
					  << nos->make_field<FIX42_EXAMPLE::HandlInst>(FIX42_EXAMPLE::HandlInst_AutomatedExecutionNoIntervention)
					  << nos->make_field<FIX42_EXAMPLE::OrderQty>(uniform_int_distribution<int>(1, 50)(eng))
					  << nos->make_field<FIX42_EXAMPLE::Price>(uniform_real_distribution<double>(119.0, 123.)(eng), 3)	// 3 decimal places if necessary
					  << nos->make_field<FIX42_EXAMPLE::Symbol>("NYSE::IBM")
					  << nos->make_field<FIX42_EXAMPLE::OrdType>(FIX42_EXAMPLE::OrdType_Limit)
					  << nos->make_field<FIX42_EXAMPLE::Side>(uniform_int_distribution<int>(0, 1)(eng) ? FIX42_EXAMPLE::Side_Buy : FIX42_EXAMPLE::Side_Sell)
					  << nos->make_field<FIX42_EXAMPLE::TimeInForce>(FIX42_EXAMPLE::TimeInForce_FillOrKill);
				mc->session_ptr()->send(move(nos));
			}

			mc->session_ptr()->request_stop();
		}
	}
	catch (const f8Exception& e)
	{
		cerr << "f8Exception: " << COLOUR(Bold, Red, e.what()) << endl;
		return 1;
	}
	catch (const exception& e)
	{
		cerr << "std::exception: " << COLOUR(Bold, Red, e.what()) << endl;
		return 1;
	}
	catch (...)
	{
		cerr << "unknown exception" << endl;
		return 1;
	}

	if (term_received)
		cout << "terminated." << endl;
	return 0;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool SimpleSession::handle_application(unsigned seqnum, MessagePtr& msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

//-----------------------------------------------------------------------------------------
MessagePtr SimpleSession::generate_logon(unsigned heartbeat_interval, const f8String davi)
{
	auto msg { Session::generate_logon(heartbeat_interval, davi) };
	return msg;
}

//-----------------------------------------------------------------------------------------
void SimpleSession::print_message(const MessagePtr& msg, ostream& os, bool usecolour) const
{
	if (!Application::quiet)
	{
		static const f8String rule { f8String(20, '-') + " received " + f8String(20, '-') };
		cout << '\r' << rule << endl;
		Session::print_message(msg, cout, Application::use_colour());
	}
}

//-----------------------------------------------------------------------------------------
void SimpleSession::on_send_success(const MessagePtr& msg) const
{
	if (!Application::quiet)
	{
		if (_control & Session::printnohb && msg->get_msgtype() == F8FIX(MsgType_HEARTBEAT))
			;
		else
		{
			static const f8String rule { f8String(22, '-') + " sent " + f8String(22, '-') };
			cout << '\r' << rule << endl;
			Session::print_message(msg, cout, Application::use_colour());
		}
	}
}

//-----------------------------------------------------------------------------------------
void SimpleSession::state_change(States::SessionStates before, States::SessionStates after, const char *where)
{
	static const array state_colours
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
	cout << state_colours[before] << " => " << state_colours[after];
	if (where)
		cout << " (" << where << ')';
	cout << endl;
	if (before == States::st_logon_sent && after == States::st_logoff_received) // force reliable client to try again even though normal exit was detected
		set_exit_state(false);
	else if (before == States::st_logoff_received && after == States::st_logoff_sent_and_received && _connection_role == ConnectionRole::cn_acceptor)
		logout_and_shutdown("goodbye");
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool SimpleSessionRouter::operator()(const FIX42_EXAMPLE::ExecutionReport *msg) const
{
	return true;
}

//-----------------------------------------------------------------------------------------
bool SimpleSessionRouter::operator()(const FIX42_EXAMPLE::NewOrderSingle *msg) const
{
	static unsigned oid{}, eoid{};
	FIX42_EXAMPLE::OrderQty::this_type qty { msg->get<FIX42_EXAMPLE::OrderQty>()->get() };
	FIX42_EXAMPLE::Price::this_type price { msg->get<FIX42_EXAMPLE::Price>()->get() };
	auto er{make_message<FIX42_EXAMPLE::ExecutionReport>()};
	MessageBasePtr erb{detail::static_pointer_cast(er)};
	msg->copy_legal(erb);

	*er << er->make_field<FIX42_EXAMPLE::OrderID>("ord" + to_string(++oid))
		 << er->make_field<FIX42_EXAMPLE::ExecID>("exec" + to_string(++eoid))
		 << er->make_field<FIX42_EXAMPLE::ExecType>(FIX42_EXAMPLE::ExecType_New)
		 << er->make_field<FIX42_EXAMPLE::OrdStatus>(FIX42_EXAMPLE::OrdStatus_New)
		 << er->make_field<FIX42_EXAMPLE::ExecTransType>(FIX42_EXAMPLE::ExecTransType_New)
		 << er->make_field<FIX42_EXAMPLE::LeavesQty>(qty)
		 << er->make_field<FIX42_EXAMPLE::CumQty>(0.)
		 << er->make_field<FIX42_EXAMPLE::AvgPx>(0.)
		 << er->make_field<FIX42_EXAMPLE::LastCapacity>('4');
	_session.send(move(er));

	FIX42_EXAMPLE::OrderQty::this_type remaining_qty{qty}, cum_qty{};
	while (remaining_qty > 0)
	{
		auto trdqty{uniform_int_distribution<int>(1, remaining_qty)(Application::eng)};
		er = make_message<FIX42_EXAMPLE::ExecutionReport>();
		MessageBasePtr erb{detail::static_pointer_cast(er)};
		msg->copy_legal(erb);
		cum_qty += trdqty;
		*er << er->make_field<FIX42_EXAMPLE::OrderID>("ord" + to_string(oid))
			 << er->make_field<FIX42_EXAMPLE::ExecID>("exec" + to_string(++eoid))
			 << er->make_field<FIX42_EXAMPLE::ExecType>(FIX42_EXAMPLE::ExecType_New)
			 << er->make_field<FIX42_EXAMPLE::OrdStatus>(remaining_qty == trdqty ? FIX42_EXAMPLE::OrdStatus_Filled : FIX42_EXAMPLE::OrdStatus_PartiallyFilled)
			 << er->make_field<FIX42_EXAMPLE::LeavesQty>(remaining_qty - trdqty)
			 << er->make_field<FIX42_EXAMPLE::ExecTransType>(FIX42_EXAMPLE::ExecTransType_New)
			 << er->make_field<FIX42_EXAMPLE::CumQty>(cum_qty)
			 << er->make_field<FIX42_EXAMPLE::LastShares>(trdqty)
			 << er->make_field<FIX42_EXAMPLE::AvgPx>(price);
		_session.send(move(er));
		remaining_qty -= trdqty;
	}
	return true;
}

