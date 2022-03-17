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
#ifndef FIX8PRO_SIMPLECLISRV_HPP_
#define FIX8PRO_SIMPLECLISRV_HPP_

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
class Application final : public Fix8ProApplication
{
	int _giveupreset, _interval;
	bool _server, _reliable, _hb, _quiet, _show_states, _generate, _summary;
	f8String _libdir, _clcf, _global_logger_name, _sses, _cses, _libpath, _tname, _username, _password, _capfile;
	unsigned _next_send, _next_receive;
	std::mt19937_64 _eng = create_seeded_mersenne_engine(); // provided by fix8pro

	int main(const std::vector<f8String>& args) override; // required
	bool options_setup(cxxopts::Options& ops) override;

	void server_session(SessionInstanceBase_ptr inst, int scnt);
	void client_session(ClientSessionBase_ptr mc);
	MessagePtr generate_order();

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
	Application& _app { Application::get_instance<Application>() };

	// ExecutionReport handler (client)
	bool operator()(const ExecutionReport *msg) override;
	// NewOrderSingle handler (server)
	bool operator()(const NewOrderSingle *msg) override;

	// required
	bool handle_application(unsigned seqnum, MessagePtr& msg) override;

	// optional overrides
	MessagePtr generate_logon(unsigned heartbeat_interval, const f8String davi) override;
	bool authenticate(SessionID& id, const MessagePtr& msg) override;
	void state_change(States::SessionStates before, States::SessionStates after, const char *where=nullptr) override;
	void reliable_state_change(States::ReliableSessionStates before, States::ReliableSessionStates after, std::any closure, const char *where=nullptr) override;
	void print_message(const MessagePtr& msg, std::ostream& os, bool usecolour) const override;
	void on_send_success(const MessagePtr& msg) const override;

	// misc
	void print_summary(const MessagePtr& msg, bool way) const;

public:
	using Session::Session;
	~SimpleSession() = default;
};

//-----------------------------------------------------------------------------------------
namespace
{
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

	void toggle(f8String_view tag, bool& what, std::ostream& os)
	{
		os << tag << ' ' << ((what ^= true) ? "on" : "off") << std::endl;
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
		const int r1 { _sb1->pubsync() }, r2 { _sb2->pubsync() };
		return r1 == 0 && r2 == 0 ? 0 : -1;
	}

	// This tee buffer has no buffer. So every character "overflows"
	// and can be put directly into the teed buffers.
	virtual int overflow(int c) override
	{
		if (c == EOF)
			return !EOF;
		const int r1 { _sb1->sputc(c) }, r2 { _sb2->sputc(c) };
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
class filtercolourteebuf : public teebuf // filters escape sequences
{
	bool ison{};
	virtual int overflow(int c) override
	{
		if (c == EOF)
			return !EOF;
		const int r1 { _sb1->sputc(c) };
		int r2{};
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
	// Construct a streambuf which tees output to both input
	// streambufs.
	filtercolourteebuf(std::streambuf *sb1, std::streambuf *sb2) : teebuf(sb1, sb2) {}
};

//-----------------------------------------------------------------------------------------
class teestream : public std::ostream
{
	filtercolourteebuf tbuf;

public:
// Construct an ostream which tees output to the supplied
// ostreams.
	teestream(std::ostream& o1, std::ostream& o2)
		: std::ostream(&tbuf), tbuf(o1.rdbuf(), o2.rdbuf()) {}
};

//-----------------------------------------------------------------------------------------

#endif // FIX8PRO_SIMPLECLISRV_HPP_

