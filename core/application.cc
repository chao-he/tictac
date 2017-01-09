#include <iostream>
#include <fstream>
#include <boost/make_shared.hpp>
#include "core/application.h"
#include "core/event_base.h"
#include "core/server_socket.h"
#include "core/uv_error_log.h"

Application::Application()
  : opts_("App options") {
    opts_.add_options()("help,h", "show help message");
  }

void Application::PrintUsage() {
  std::cout << opts_;
}

namespace bpo = boost::program_options;

bpo::options_description_easy_init Application::add_options() {
  return opts_.add_options();
}

void Application::add_positional_options(std::initializer_list<positional_option> options) {
  for (auto&& o : options) {
    opts_.add(boost::make_shared<bpo::option_description>(o.name, o.value_semantic, o.help));
    pos_opts_.add(o.name, o.max_count);
  }
}

bpo::variables_map& Application::configuration() {
  return *configuration_;
}

void Application::Init(int argc, char** argv) {
  // app_name_ = argv[0];
  // int i = 1;
  // for (; i < argc; ++ i) {
  //   std::string arg(argv[i]);
  //   if (arg == "--") break;
  //   if (arg == "--help") {
  //     PrintUsage();
  //     exit(0);
  //   }
  //   if (arg[0] == '-') {
  //     ++ i; 
  //     cmdline_opts_[arg.substr(1)] = argv[i];
  //   } else {
  //     cmdline_args_.emplace_back(argv[i]);
  //   }
  // }
  // for (; i < argc; ++ i) {
  //   cmdline_args_.emplace_back(argv[i]);
  // }
  //

  bpo::variables_map configuration;
  try {
    bpo::store(bpo::command_line_parser(argc, argv)
        .options(opts_)
        .positional(pos_opts_)
        .run(), configuration);
    std::ifstream ifs("conf/server.conf");
    if (ifs) {
      bpo::store(bpo::parse_config_file(ifs, opts_), configuration);
    }
  } catch (bpo::error& e) {
    fprintf(stderr, "error: %s\n\nTry --help.\n", e.what());
    return ;
  }
  bpo::notify(configuration);
  if (configuration.count("help")) {
    std::cout << opts_ << "\n";
    exit(0);
  }
  configuration.emplace("appname", boost::program_options::variable_value(std::string(argv[0]), false));
  configuration_ = {std::move(configuration)};
}

SimpleServer::SimpleServer() {
}


static void connection_cb(uv_stream_t* server, int status) {
  // uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  // uv_tcp_init(server->loop, client);
  // fprintf(stderr, "accepted_fd = %d, listen_fd = %d, %d\n", server->accepted_fd,  server->io_watcher.fd, client->io_watcher.fd);
  // int r = uv_accept(server, (uv_stream_t*)client);
  AsyncIOStream client;
  client.attach(server->loop);
  int r = uv_accept(server, client.stream());
  // uv_tcp_t client;
  // uv_tcp_init(server->loop, &client);
  // int r = uv_accept(server, &client);
  UV_ERROR_LOG(r);
}


int SimpleServer::Run() {

  const char* ip = "0.0.0.0";
  int port = 9014;

  EventBase base;
  // uv_loop_t loop;
  // uv_loop_init(&loop);

  ServerSocket server(&base);
  // AsyncIOStream server(base.loop());
  // uv_tcp_t server;
  // uv_tcp_init(&loop, &server);

  server.Bind(ip, port);
  // struct sockaddr_in addr;
  // uv_ip4_addr(ip, port, &addr);
  // uv_tcp_bind(server.transport(), (struct sockaddr*)&addr, 0);

  server.Listen(512);
  // uv_listen(server.stream(), 512, connection_cb);

  base.Run();
  // uv_run(base.loop(), UV_RUN_DEFAULT);

  // ServerSocket server(&base);
  // server.Bind("0.0.0.0", 9010);
  // server.Listen(512);
  // base.Run();
  return 0;
}
