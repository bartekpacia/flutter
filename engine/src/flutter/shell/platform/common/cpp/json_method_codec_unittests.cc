// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/cpp/json_method_codec.h"

#include "gtest/gtest.h"

namespace flutter {

namespace {

// Returns true if the given method calls have the same method name, and their
// arguments have equivalent values.
bool MethodCallsAreEqual(const MethodCall<rapidjson::Document>& a,
                         const MethodCall<rapidjson::Document>& b) {
  if (a.method_name() != b.method_name()) {
    return false;
  }
  // Treat nullptr and Null as equivalent.
  if ((!a.arguments() || a.arguments()->IsNull()) &&
      (!b.arguments() || b.arguments()->IsNull())) {
    return true;
  }
  return *a.arguments() == *b.arguments();
}

}  // namespace

TEST(JsonMethodCodec, HandlesMethodCallsWithNullArguments) {
  const JsonMethodCodec& codec = JsonMethodCodec::GetInstance();
  MethodCall<rapidjson::Document> call("hello", nullptr);
  auto encoded = codec.EncodeMethodCall(call);
  ASSERT_TRUE(encoded);
  std::unique_ptr<MethodCall<rapidjson::Document>> decoded =
      codec.DecodeMethodCall(*encoded);
  ASSERT_TRUE(decoded);
  EXPECT_TRUE(MethodCallsAreEqual(call, *decoded));
}

TEST(JsonMethodCodec, HandlesMethodCallsWithArgument) {
  const JsonMethodCodec& codec = JsonMethodCodec::GetInstance();

  auto arguments = std::make_unique<rapidjson::Document>(rapidjson::kArrayType);
  auto& allocator = arguments->GetAllocator();
  arguments->PushBack(42, allocator);
  arguments->PushBack("world", allocator);
  MethodCall<rapidjson::Document> call("hello", std::move(arguments));
  auto encoded = codec.EncodeMethodCall(call);
  ASSERT_TRUE(encoded);
  std::unique_ptr<MethodCall<rapidjson::Document>> decoded =
      codec.DecodeMethodCall(*encoded);
  ASSERT_TRUE(decoded);
  EXPECT_TRUE(MethodCallsAreEqual(call, *decoded));
}

TEST(JsonMethodCodec, HandlesSuccessEnvelopesWithNullResult) {
  const JsonMethodCodec& codec = JsonMethodCodec::GetInstance();
  auto encoded = codec.EncodeSuccessEnvelope();
  ASSERT_TRUE(encoded);
  std::vector<uint8_t> bytes = {'[', 'n', 'u', 'l', 'l', ']'};
  EXPECT_EQ(*encoded, bytes);
  // TODO: Add round-trip check once decoding replies is implemented.
}

TEST(JsonMethodCodec, HandlesSuccessEnvelopesWithResult) {
  const JsonMethodCodec& codec = JsonMethodCodec::GetInstance();
  rapidjson::Document result;
  result.SetInt(42);
  auto encoded = codec.EncodeSuccessEnvelope(&result);
  ASSERT_TRUE(encoded);
  std::vector<uint8_t> bytes = {'[', '4', '2', ']'};
  EXPECT_EQ(*encoded, bytes);
  // TODO: Add round-trip check once decoding replies is implemented.
}

TEST(JsonMethodCodec, HandlesErrorEnvelopesWithNulls) {
  const JsonMethodCodec& codec = JsonMethodCodec::GetInstance();
  auto encoded = codec.EncodeErrorEnvelope("errorCode");
  ASSERT_TRUE(encoded);
  std::vector<uint8_t> bytes = {
      '[', '"', 'e', 'r', 'r', 'o', 'r', 'C', 'o', 'd', 'e',
      '"', ',', '"', '"', ',', 'n', 'u', 'l', 'l', ']',
  };
  EXPECT_EQ(*encoded, bytes);
  // TODO: Add round-trip check once decoding replies is implemented.
}

TEST(JsonMethodCodec, HandlesErrorEnvelopesWithDetails) {
  const JsonMethodCodec& codec = JsonMethodCodec::GetInstance();
  rapidjson::Document details(rapidjson::kArrayType);
  auto& allocator = details.GetAllocator();
  details.PushBack("a", allocator);
  details.PushBack(42, allocator);
  auto encoded =
      codec.EncodeErrorEnvelope("errorCode", "something failed", &details);
  ASSERT_NE(encoded.get(), nullptr);
  std::vector<uint8_t> bytes = {
      '[', '"', 'e', 'r', 'r', 'o', 'r', 'C', 'o', 'd', 'e', '"', ',', '"',
      's', 'o', 'm', 'e', 't', 'h', 'i', 'n', 'g', ' ', 'f', 'a', 'i', 'l',
      'e', 'd', '"', ',', '[', '"', 'a', '"', ',', '4', '2', ']', ']',
  };
  EXPECT_EQ(*encoded, bytes);
  // TODO: Add round-trip check once decoding replies is implemented.
}

}  // namespace flutter
