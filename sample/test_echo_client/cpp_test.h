#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sstream>


#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN32

#include <core/application.h>
#include <core/event.h>
#include <core/dynamic_module_factory.h>

#include <network/acceptor.h>
#include <network/connector.h>

#include <log/log.h>

#include <random>
#include <lua_proxy/lua_proxy.h>

#include "addressbook.pb.h"

class Client
	: public gsf::Module
	, public gsf::IEvent
{
public:
	Client()
		: Module("Client")
	{}

	void before_init() override
	{
		dispatch(eid::app_id, eid::new_dynamic_module, gsf::Args("ConnectorModule"), [&](const gsf::Args &args) {
			connector_id_ = args.pop_int32(0);

			listen(connector_id_, eid::network::connector_init, std::bind(&Client::create_connector_succ, this, std::placeholders::_1));
		});

	}

	void create_connector_succ(const gsf::Args &args)
	{
		dispatch(connector_id_, eid::network::send_remote_callback, gsf::Args(), [&](const gsf::Args &args) {
			send_ = args.pop_remote_callback(0);
		});

		listen(this, eid::network::new_connect, [&](const gsf::Args &args, gsf::CallbackFunc callback) {
			fd_ = args.pop_int32(0);

			tutorial::Person _p;
			_p.set_name("jack");
			_p.set_id(1000);
			_p.set_email("127.0.0.1");
			std::string _msg = "";
			
			if (_p.SerializeToString(&_msg)) {
				send_(fd_, 1001, _msg);
			}

		});
		
		/*
		dispatch(connector_id_, eid::network::recv_remote_callback, gsf::Args(
			get_module_id()
			, 1002), std::bind(&Client::msg_handler, this, std::placeholders::_1));
		*/
		dispatch(connector_id_, eid::network::make_connector
			, gsf::Args(get_module_id(), "127.0.0.1", 8001));
	}

	void init() override
	{

	}

	void msg_handler(const gsf::Args &args)
	{
		send_(fd_, 1001, "hello");
	}

private:
	gsf::ModuleID connector_id_ = gsf::ModuleNil;
	gsf::SessionID fd_ = gsf::SessionNil;

	gsf::RemoteFunc send_;
};