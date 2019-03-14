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

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "route_guide.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using routeguide::Point;
using routeguide::Feature;
using routeguide::RouteNote;
using routeguide::RouteGuide;

Point MakePoint(long latitude, long longitude) {
  Point p;
  p.set_latitude(latitude);
  p.set_longitude(longitude);
  return p;
}

int main(int argc, char** argv) {
  std::cout << "-------------- Custom RouteChat --------------" << std::endl;
  ClientContext* context = new ClientContext();
  std::shared_ptr<Channel> channel = grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials());
  std::unique_ptr<RouteGuide::Stub> stub = RouteGuide::NewStub(channel);

  std::unique_ptr<ClientReaderWriter<RouteNote, RouteNote> > stream =
      stub->RouteChat(context);

  for (int i = 0; i < 100; i++) {
    {
      std::shared_ptr<Channel> channel = grpc::CreateChannel(
          "localhost:50051", grpc::InsecureChannelCredentials());
      std::unique_ptr<RouteGuide::Stub> stub = RouteGuide::NewStub(channel);

      Point point = MakePoint(409146138, -746188906);
      Feature feature;

      ClientContext context;
      Status status = stub->GetFeature(&context, point, &feature);

      std::cout << "Put status" << status.ok() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << i << std::endl;
  }

  return 0;
}
