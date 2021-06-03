
#include "buffer.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "gtest/gtest.h"

using random_bytes_engine =
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT,
                                 unsigned char>;

std::vector<unsigned char> randomBytes(std::size_t size) {
  static random_bytes_engine rbe;
  std::vector<unsigned char> data(size);
  std::generate(begin(data), end(data), std::ref(rbe));
  return data;
}

TEST(buffer, push) {
  tl::buffer b;
  b.default_chunk_size(2);
  ASSERT_EQ(b.size(), 0);

  b.push("1", 1);
  ASSERT_EQ(b.size(), 1);
  ASSERT_EQ(b[0], '1');

  b.push("2", 1);
  ASSERT_EQ(b.size(), 2);
  ASSERT_EQ(b[0], '1');
  ASSERT_EQ(b[1], '2');

  b.push("3", 1);
  ASSERT_EQ(b.size(), 3);
  ASSERT_EQ(b[0], '1');
  ASSERT_EQ(b[1], '2');
  ASSERT_EQ(b[2], '3');

  b.push("456789", 6);
  ASSERT_EQ(b.size(), 9);
  ASSERT_EQ(b[0], '1');
  ASSERT_EQ(b[1], '2');
  ASSERT_EQ(b[2], '3');
  ASSERT_EQ(b[3], '4');
  ASSERT_EQ(b[4], '5');
  ASSERT_EQ(b[5], '6');
  ASSERT_EQ(b[6], '7');
  ASSERT_EQ(b[7], '8');
  ASSERT_EQ(b[8], '9');
}

TEST(buffer, drain) {
  tl::buffer b;
  b.default_chunk_size(2);

  std::string data = "0123456789";
  b.push(data.data(), data.size());
  ASSERT_EQ(b.size(), data.size());

  for (std::size_t i = 0; i < data.size(); ++i) {
    ASSERT_EQ(b[i], data[i]) << i;
  }

  b.drain(1);
  ASSERT_EQ(b.size(), data.size() - 1);
  for (std::size_t i = 1; i < data.size(); ++i) {
    ASSERT_EQ(b[i - 1], data[i]) << i;
  }

  b.drain(3);
  ASSERT_EQ(b.size(), data.size() - 4);
  for (std::size_t i = 4; i < data.size(); ++i) {
    ASSERT_EQ(b[i - 4], data[i]) << i;
  }

  b.push(data.data(), data.size());
  std::string new_data = "4567890123456789";
  ASSERT_EQ(b.size(), new_data.size());
  for (std::size_t i = 0; i < new_data.size(); ++i) {
    ASSERT_EQ(b[i], new_data[i]) << i;
  }

  b.drain(7);
  ASSERT_EQ(b.size(), new_data.size() - 7);
  for (std::size_t i = 7; i < new_data.size(); ++i) {
    ASSERT_EQ(b[i - 7], new_data[i]) << i;
  }

  b.drain(b.size());
  ASSERT_EQ(b.size(), 0);
}

TEST(buffer, data) {
  tl::buffer b;
  b.default_chunk_size(2);
  std::string data = "0123456789";
  b.push(data.data(), data.size());
  const void *pdata;
  for (std::size_t i = 1; i < data.size(); ++i) {
    const void *pdata;
    b.data(pdata, i);
    const unsigned char *p = (const unsigned char *)pdata;
    for (std::size_t j = 0; j < i; ++j) {
      ASSERT_EQ(p[j], data[j]) << "i=" << i << ", j=" << j;
      ASSERT_EQ(b[j], data[j]) << "i=" << i << ", j=" << j;
    }
  }

  b.drain(b.size());
  for (std::size_t i = 0; i < data.size(); ++i) {
    b.push(data.data() + i, 1);
  }
  for (std::size_t i = 0; i < b.size(); i++) {
    ASSERT_EQ(b[i], data[i]);
  }
  b.data(pdata, 5);
  for (std::size_t i = 0; i < b.size(); i++) {
    ASSERT_EQ(b[i], data[i]);
  }
}

TEST(buffer, dataChunk) {
  std::string data = "0123456789";
  tl::buffer b;
  b.default_chunk_size(2);

  for (std::size_t i = 0; i < data.size(); i++) {
    b.push(data.data() + i, 1);
  }

  for (std::size_t i = 1; i < data.size(); i++) {
    b.drain(1);
    for (std::size_t j = 0; j < b.size(); j++) {
      ASSERT_EQ(b[j], data[j + i]);
    }
  }
}

TEST(buffer, space) {
  std::string data = "0123456789";
  tl::buffer b;
  b.default_chunk_size(2);
  void *buf;
  unsigned char *p = (unsigned char *)buf;

  b.space(buf, 1);
  ASSERT_NE(buf, nullptr);
  p = (unsigned char *)buf;
  p[0] = '0';
  b.spaceHaveSeted(1);
  ASSERT_EQ(b.size(), 1);
  ASSERT_EQ(b[0], '0');

  b.space(buf, 10);
  ASSERT_NE(buf, nullptr);
  p = (unsigned char *)buf;
  p[0] = '1';
  p[1] = '2';
  b.spaceHaveSeted(2);
  ASSERT_EQ(b.size(), 3);
  ASSERT_EQ(b[0], '0');
  ASSERT_EQ(b[1], '1');
  ASSERT_EQ(b[2], '2');
}

TEST(buffer, push_drain) {
  srand((unsigned)time(NULL));
  std::vector<unsigned char> in, out;
  tl::buffer b;
  b.default_chunk_size(20);
  const void *data;
  for (int i = 0; i < 100; i++) {
    auto rand_bytes = randomBytes(rand() % 500 + 3);
    in.insert(in.end(), rand_bytes.begin(), rand_bytes.end());
    b.push(rand_bytes.data(), rand_bytes.size());

    for (std::size_t x = b.size() - rand_bytes.size(); x < b.size(); ++x) {
      ASSERT_EQ(b[x], rand_bytes[x + rand_bytes.size() - b.size()]);
    }

    for (std::size_t x = 0; x < b.size(); ++x) {
      ASSERT_EQ(b[x], in[in.size() - b.size() + x]);
    }

    if (rand() % 5 == 3 && b.size()) {
      std::size_t drain_size = rand() % b.size() + 1;

      ASSERT_LE(drain_size, b.size());

      b.data(data, drain_size);
      out.insert(out.end(), (const unsigned char *)data,
                 (const unsigned char *)data + drain_size);
      b.drain(drain_size);
    }
  }

  b.data(data, b.size());
  out.insert(out.end(), (const unsigned char *)data,
             (const unsigned char *)data + b.size());
  b.drain(b.size());

  ASSERT_EQ(in.size(), out.size()) << in.size();

  std::ofstream inf("a.dat", std::ios::out | std::ios::binary),
      outf("b.dat", std::ios::out | std::ios::binary);
  inf.write((char *)in.data(), in.size());
  outf.write((char *)out.data(), out.size());

  for (std::size_t i = 0; i < in.size(); ++i) {
    ASSERT_EQ(in[i], out[i]) << i;
  }

  ASSERT_TRUE(in == out);
}

TEST(buffer, spaceChunk) {
  std::string data = "0123456789";
  tl::buffer b;
  b.default_chunk_size(2);

  void *buf;
  unsigned char *p;
  std::size_t len;

  b.spaceChunk(buf, len);
  ASSERT_EQ(len, 2);
  ASSERT_NE(buf, nullptr);
  p = (unsigned char *)buf;
  p[0] = '0';
  b.spaceHaveSeted(1);
  ASSERT_EQ(b.size(), 1);
  ASSERT_EQ(b[0], '0');

  b.spaceChunk(buf, len);
  ASSERT_EQ(len, 1);
  ASSERT_NE(buf, nullptr);
  p = (unsigned char *)buf;
  p[0] = '1';
  b.spaceHaveSeted(1);
  ASSERT_EQ(b.size(), 2);
  ASSERT_EQ(b[0], '0');
  ASSERT_EQ(b[1], '1');

  b.spaceChunk(buf, len);
  ASSERT_EQ(len, 2);
  ASSERT_NE(buf, nullptr);
  p = (unsigned char *)buf;
  p[0] = '2';
  p[1] = '3';
  b.spaceHaveSeted(2);

  b.spaceChunk(buf, len);
  p = (unsigned char *)buf;
  ASSERT_EQ(len, 2);
  ASSERT_NE(buf, nullptr);
  p[0] = '4';
  b.spaceHaveSeted(1);

  ASSERT_EQ(b.size(), 5);

  for (std::size_t i = 0; i < 5; i++) {
    ASSERT_EQ(b[i], data[i]) << "i=" << i;
  }
}

TEST(buffer, swap) {
  tl::buffer b1, b2;
  std::string data1 = "0123456789";      // 10
  std::string data2 = "abcdefghijklmn";  // 14
  b1.default_chunk_size(2);
  b2.default_chunk_size(3);

  b1.push(data1.data(), data1.size());
  b2.push(data2.data(), data2.size());

  // std::swap(b1, b2);
  b1.swap(b2);

  ASSERT_EQ(b1.size(), data2.size());
  ASSERT_EQ(b2.size(), data1.size());
  for (std::size_t i = 0; i < data2.size(); i++) {
    ASSERT_EQ(b1[i], data2[i]);
  }
  for (std::size_t i = 0; i < data1.size(); i++) {
    ASSERT_EQ(b2[i], data1[i]);
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
