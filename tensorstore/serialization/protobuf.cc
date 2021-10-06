// Copyright 2021 The TensorStore Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "tensorstore/serialization/protobuf.h"

#include "google/protobuf/message_lite.h"
#include "absl/status/status.h"
#include "riegeli/bytes/limiting_reader.h"
#include "riegeli/messages/message_parse.h"
#include "riegeli/messages/message_serialize.h"
#include "tensorstore/serialization/serialization.h"

namespace tensorstore {
namespace serialization {

bool ProtobufSerializer::Encode(EncodeSink& sink,
                                const google::protobuf::MessageLite& value) {
  riegeli::SerializeOptions options;
  options.set_partial(true);
  if (!serialization::WriteSize(sink.writer(), options.GetByteSize(value))) {
    return false;
  }
  auto status = riegeli::SerializeToWriter(value, &sink.writer(), options);
  if (!status.ok()) {
    sink.Fail(std::move(status));
    return false;
  }
  return true;
}

bool ProtobufSerializer::Decode(DecodeSource& source,
                                google::protobuf::MessageLite& value) {
  size_t size;
  if (!serialization::ReadSize(source.reader(), size)) return false;
  riegeli::ParseOptions options;
  options.set_partial(true);
  auto status = riegeli::ParseFromReader(
      riegeli::LimitingReader(
          &source.reader(),
          riegeli::LimitingReaderBase::Options().set_exact_length(size)),
      value, options);
  if (!status.ok()) {
    source.Fail(std::move(status));
    return false;
  }
  return true;
}

}  // namespace serialization
}  // namespace tensorstore
