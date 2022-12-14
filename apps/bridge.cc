#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <time.h>
#include <unistd.h>

#include "lib/mars.h"

void collect(const std::string what, int client,
             std::shared_ptr<mars::Temps> temps,
             std::shared_ptr<mars::Transit> transit) {
  std::string temp_filename = temps->temp();
  int fd = open(temp_filename.c_str(), O_WRONLY | O_CREAT, 0666);
  if (fd < 0) {
    std::cerr << what << ":client: " << mars::error_description() << "\n";
    exit(1);
  }
  size_t size = 64 * 1024;
  std::unique_ptr<char[]> buffer = std::unique_ptr<char[]>(new char[size]);
  int n, m;
  while ((n = read(client, buffer.get(), size)) > 0)
    if ((m = write(fd, buffer.get(), n)) != n) {
      if (m < 0)
        std::cerr << what << ":storing: " << mars::error_description() << "\n";
      else
        std::cerr << what << ":storing: write shortfall"
                  << "\n";
      exit(1);
    }
  if (n < 0) {
    std::cerr << what << ":collecting: " << mars::error_description() << "\n";
    exit(1);
  }
  close(fd);
  close(client);
  transit->send(temp_filename);
}

void deliver(const std::string what, int remote_server,
             std::shared_ptr<mars::Transit> transit) {
  size_t size = 64 * 1024;
  std::unique_ptr<char[]> buffer = std::unique_ptr<char[]>(new char[size]);
  for (;;) {
    std::string temp_filename = transit->receive();
    int fd = open(temp_filename.c_str(), O_RDONLY);
    int n, m;
    while ((n = read(fd, buffer.get(), size)) > 0)
      if ((m = write(remote_server, buffer.get(), n)) != n) {
        if (m < 0)
          std::cerr << what << ":delivering: " << mars::error_description()
                    << "\n";
        else
          std::cerr << what << ":delivering: write shortfall"
                    << "\n";
        exit(1);
      }
    if (n < 0) {
      std::cerr << what << ":loading : " << mars::error_description() << "\n";
      exit(1);
    }
    close(fd);
    std::remove(temp_filename.c_str());
  }
}

time_t ten_seconds() { return time(0) + 10; }

void usage(std::string program_name) {
  std::cerr << "usage: " << program_name
            << "planet [--here port] [--there host port] [-temps directory] "
               "[allowed-clients...]\n";
}

int main(int argc, char **argv) {
  std::string error;
  std::string program_name = argv[0];
  --argc;
  argv++;
  if (argc == 1 && std::string(*argv) == "--help") {
    usage(program_name);
    return 0;
  }
  if (argc == 0) {
    usage(program_name);
    return 1;
  }
  std::string planet = std::string(*argv);
  --argc;
  argv++;
  uint16_t here_port, there_port;
  if (planet == "mars") {
    here_port = mars::me_port;
    there_port = mars::earth_port;
  } else if (planet == "earth") {
    here_port = mars::em_port;
    there_port = mars::mars_port;
  } else {
    std::cerr << program_name << ": unknown planet:" << planet << "\n";
    return 1;
  }
  std::string what = program_name + ": " + planet;
  std::string there_host = "127.0.0.1";
  std::string temps_directory = "./tmp";
  std::vector<std::string> allowed_hostnames;
  while (argc > 0)
    if (std::string(*argv) == "--here") {
      if (argc < 2) {
        usage(program_name);
        return 1;
      }
      if (!mars::parse_port(argv[1], &here_port, &error)) {
        std::cerr << what << ": " << error << "\n";
        return 1;
      }
      argc -= 2;
      argv += 2;
    } else if (std::string(*argv) == "--temps") {
      if (argc < 2) {
        usage(program_name);
        return 1;
      }
      temps_directory = std::string(argv[1]);
      argc -= 2;
      argv += 2;
    } else if (std::string(*argv) == "--there") {
      if (argc < 3) {
        usage(program_name);
        return 1;
      }
      there_host = std::string(argv[1]);
      allowed_hostnames.emplace_back(there_host);
      if (!mars::parse_port(argv[2], &there_port, &error)) {
        std::cerr << what << ": " << error << "\n";
        return 1;
      }
      argc -= 3;
      argv += 3;
    } else {
      allowed_hostnames.emplace_back(*argv);
      --argc;
      argv++;
    }
  std::shared_ptr<mars::Temps> temps =
      mars::Temps::make(temps_directory, &error);
  if (temps == nullptr) {
    std::cerr << what << ": " << error << "\n";
    return 1;
  }
  std::shared_ptr<mars::Transit> transit =
      mars::Transit::make(ten_seconds, &error);
  if (transit == nullptr) {
    std::cerr << what << ": " << error << "\n";
    return 1;
  }
  std::shared_ptr<mars::Server> server = mars::Server::make(here_port, &error);
  if (server == nullptr) {
    std::cerr << what << ": " << error << "\n";
    return 1;
  }
  {
    int remote_server = mars::connect2server(there_host, there_port, &error);
    if (remote_server < 0) {
      std::cerr << what << ": " << error << "\n";
      return 1;
    }
    std::thread t(deliver, what, remote_server, transit);
    t.detach();
  }
  int client;
  while ((client = server->client(&error)) != -1) {
    std::thread t(collect, what, client, temps, transit);
    t.detach();
  }
  return 0;
}
