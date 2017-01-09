
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include "core/event_base.h"
#include "core/server_socket.h"
#include "index_manager.h"
#include "location_application.h"


void stdout_redirect() {
  char path[300];
  snprintf(path, sizeof(path), "logs/%d.out.log", getpid());
  open(path, O_CREAT|O_RDWR, 0644);
  snprintf(path, sizeof(path), "logs/%d.err.log", getpid());
  open(path, O_CREAT|O_RDWR, 0644);
  fprintf(stderr, "%d\n", getpid());
}

void daemonize() {
  for (int i = 0; i < 2; ++ i) {
    int pid = fork();
    if (pid != 0) {
      exit(0);
    } else if (pid < 0) {
      fprintf(stderr, "call fork failed\n");
      exit(1);
    }
    if (i == 0) {
      setsid();
      close(1);
      close(2);
    }
  }
  signal(SIGPIPE, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  stdout_redirect();
}

namespace bpo = boost::program_options;

LocationServiceApplication::LocationServiceApplication() {
  add_options()
    ("db,D", bpo::value<std::string>()->default_value("db"), "/path/to/db")
    ("port,p", bpo::value<int>()->default_value(9010), "which port to listen")
    ("threads,n", bpo::value<int>()->default_value(16), "number of threads")
    ("daemon,d", "run in background");
}

int LocationServiceApplication::Run() {
  std::string dbpath = configuration()["db"].as<std::string>();
  int port = configuration()["port"].as<int>();
  int threads = configuration()["threads"].as<int>();
  if (configuration().count("daemon")) {
    daemonize();
  }
  
  IndexManager::Instance().Init(dbpath);

  std::vector<std::thread> workers;
  for (int i = 0; i < threads; ++ i) {
    workers.emplace_back([port, dbpath, i] {
        EventBase evb;
        ServerSocket sock(&evb);
        sock.Bind("0.0.0.0", port);
        sock.Listen(512);
        fprintf(stderr, "[%d] listen on %d\n", i, port);
        evb.Run();
    });
  }

  for (auto& t: workers) {
    t.join();
  }

  fprintf(stderr, "exit\n");
  return 0;
}
