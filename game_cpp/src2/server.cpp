#include "snapshot_json.h"
#include "world.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

namespace {

std::atomic<bool> g_running(true);

void handle_signal(int) {
  g_running = false;
}

bool parse_spawn_food_message(const std::string& payload, int& x, int& y) {
  if (payload.find("\"type\":\"spawn_food\"") == std::string::npos &&
      payload.find("\"type\": \"spawn_food\"") == std::string::npos) {
    return false;
  }

  auto read_int_field = [&](const std::string& field, int& value) -> bool {
    const std::string key = "\"" + field + "\"";
    const size_t key_pos = payload.find(key);
    if (key_pos == std::string::npos) return false;
    const size_t colon_pos = payload.find(':', key_pos);
    if (colon_pos == std::string::npos) return false;
    size_t start = colon_pos + 1;
    while (start < payload.size() && std::isspace(static_cast<unsigned char>(payload[start]))) {
      start++;
    }
    size_t end = start;
    if (end < payload.size() && payload[end] == '-') {
      end++;
    }
    while (end < payload.size() && std::isdigit(static_cast<unsigned char>(payload[end]))) {
      end++;
    }
    if (start == end) return false;
    value = std::stoi(payload.substr(start, end - start));
    return true;
  };

  return read_int_field("x", x) && read_int_field("y", y);
}

class Session;

class SimulationServer {
public:
  explicit SimulationServer(unsigned short port)
      : ioc_(1),
        acceptor_(ioc_, tcp::endpoint(asio::ip::make_address("127.0.0.1"), port)),
        world_(8, 8, 1280, 720, 0.1f),
        port_(port) {
    seed_world();
  }

  void run() {
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    std::cout << "Simulation WebSocket server listening on ws://127.0.0.1:" << port_ << "\n";

    do_accept();

    std::thread network_thread([this]() {
      ioc_.run();
    });

    simulation_loop();

    beast::error_code ec;
    acceptor_.close(ec);
    ioc_.stop();
    if (network_thread.joinable()) {
      network_thread.join();
    }
  }

  void register_session(const std::shared_ptr<Session>& session) {
    std::lock_guard<std::mutex> lock(session_mutex_);
    sessions_.insert(session);
  }

  void unregister_session(const std::shared_ptr<Session>& session) {
    std::lock_guard<std::mutex> lock(session_mutex_);
    sessions_.erase(session);
  }

  std::string current_snapshot_json() {
    std::lock_guard<std::mutex> lock(world_mutex_);
    return snapshot_to_json(world_.snapshot());
  }

  void spawn_food(SimPoint pt) {
    std::lock_guard<std::mutex> lock(world_mutex_);
    if (!PtInRectPortable(&world_.brc, pt)) {
      return;
    }
    world_.spawn_food(pt, false);
  }

private:
  void seed_world() {
    world_.spawn_cell({220, 220}, 1);
    world_.spawn_cell({980, 520}, 2);
    world_.spawn_food({420, 300}, true);
    world_.spawn_food({780, 410}, false);
    world_.spawn_food({630, 180}, false);
    world_.spawn_linear_barrier({300, 500}, 0, 0, 0, 500);
  }

  void do_accept();

  void simulation_loop() {
    using clock = std::chrono::steady_clock;
    const auto tick = std::chrono::milliseconds(50);

    while (g_running.load()) {
      const auto frame_start = clock::now();

      std::string payload;
      {
        std::lock_guard<std::mutex> lock(world_mutex_);
        world_.simulate_world(0.1f);
        payload = snapshot_to_json(world_.snapshot());
      }

      broadcast(payload);
      std::this_thread::sleep_until(frame_start + tick);
    }
  }

  void broadcast(const std::string& payload);

  asio::io_context ioc_;
  tcp::acceptor acceptor_;
  World world_;
  std::mutex world_mutex_;
  std::mutex session_mutex_;
  std::set<std::shared_ptr<Session>> sessions_;
  unsigned short port_;

  friend class Session;
};

class Session : public std::enable_shared_from_this<Session> {
public:
  Session(tcp::socket socket, SimulationServer& server)
      : ws_(std::move(socket)),
        server_(server) {}

  void start() {
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    ws_.accept();
    server_.register_session(shared_from_this());
    send(server_.current_snapshot_json());
    do_read();
  }

  void send(const std::string& message) {
    auto self = shared_from_this();
    asio::post(ws_.get_executor(), [self, message]() {
      const bool writing = !self->outbox_.empty();
      self->outbox_.push_back(message);
      if (!writing) {
        self->do_write();
      }
    });
  }

private:
  void do_read() {
    auto self = shared_from_this();
    ws_.async_read(buffer_, [self](beast::error_code ec, std::size_t) {
      if (ec) {
        self->close();
        return;
      }

      const std::string payload = beast::buffers_to_string(self->buffer_.data());
      self->buffer_.consume(self->buffer_.size());

      if (payload == "ping") {
        self->send(R"({"type":"pong"})");
      } else {
        int x = 0;
        int y = 0;
        if (parse_spawn_food_message(payload, x, y)) {
          self->server_.spawn_food({x, y});
        }
      }

      self->do_read();
    });
  }

  void do_write() {
    auto self = shared_from_this();
    ws_.text(true);
    ws_.async_write(
        asio::buffer(outbox_.front()),
        [self](beast::error_code ec, std::size_t) {
          if (ec) {
            self->close();
            return;
          }

          self->outbox_.pop_front();
          if (!self->outbox_.empty()) {
            self->do_write();
          }
        });
  }

  void close() {
    if (closed_.exchange(true)) {
      return;
    }
    beast::error_code ec;
    ws_.close(websocket::close_code::normal, ec);
    server_.unregister_session(shared_from_this());
  }

  websocket::stream<tcp::socket> ws_;
  beast::flat_buffer buffer_;
  std::deque<std::string> outbox_;
  SimulationServer& server_;
  std::atomic<bool> closed_{false};
};

void SimulationServer::do_accept() {
  acceptor_.async_accept([this](beast::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<Session>(std::move(socket), *this)->start();
    }

    if (g_running.load()) {
      do_accept();
    }
  });
}

void SimulationServer::broadcast(const std::string& payload) {
  std::vector<std::shared_ptr<Session>> snapshot;
  {
    std::lock_guard<std::mutex> lock(session_mutex_);
    snapshot.assign(sessions_.begin(), sessions_.end());
  }

  for (const auto& session : snapshot) {
    if (session) {
      session->send(payload);
    }
  }
}

}  // namespace

int main() {
  SimulationServer server(5001);
  server.run();
  return 0;
}
