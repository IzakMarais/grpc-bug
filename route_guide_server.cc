/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "route_guide.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using routeguide::Point;
using routeguide::Feature;
using routeguide::RouteNote;
using routeguide::RouteGuide;
using std::chrono::system_clock;


class RouteGuideImpl final : public RouteGuide::Service {
 public:
  explicit RouteGuideImpl() { }

  Status GetFeature(ServerContext* context, const Point* point,
                    Feature* feature) override {
    feature->set_name("test");
    feature->mutable_location()->CopyFrom(*point);
    return Status::OK;
  }

  Status RouteChat(ServerContext* context,
                   ServerReaderWriter<RouteNote, RouteNote>* stream) override {
    RouteNote note;
    while (stream->Read(&note)) {
      stream->Write(note);
    }

    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  RouteGuideImpl guidService;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&guidService);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}
