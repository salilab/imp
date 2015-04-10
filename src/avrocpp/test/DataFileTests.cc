/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <boost/test/included/unit_test_framework.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <sstream>

#include "DataFile.hh"
#include "Generic.hh"
#include "Stream.hh"
#include "Compiler.hh"

using boost::shared_ptr;
using std::string;
using std::pair;
using std::vector;
using std::map;
using std::istringstream;
using std::ostringstream;

using boost::array;
using boost::shared_ptr;
using boost::unit_test::test_suite;

using internal_avro::ValidSchema;
using internal_avro::GenericDatum;
using internal_avro::GenericRecord;

const int count = 1000;

template <typename T>
struct Complex {
  T re;
  T im;
  Complex() : re(0), im(0) {}
  Complex(T r, T i) : re(r), im(i) {}
};

struct Integer {
  int64_t re;
  Integer() : re(0) {}
  Integer(int64_t r) : re(r) {}
};

typedef Complex<int64_t> ComplexInteger;
typedef Complex<double> ComplexDouble;

struct Double {
  double re;
  Double() : re(0) {}
  Double(double r) : re(r) {}
};

struct PaddedRecord {
  int32_t index;
  std::vector<uint8_t> padding;
};

namespace internal_avro {

template <typename T>
struct codec_traits<Complex<T> > {
  static void encode(Encoder& e, const Complex<T>& c) {
    internal_avro::encode(e, c.re);
    internal_avro::encode(e, c.im);
  }

  static void decode(Decoder& d, Complex<T>& c) {
    internal_avro::decode(d, c.re);
    internal_avro::decode(d, c.im);
  }
};

template <>
struct codec_traits<Integer> {
  static void decode(Decoder& d, Integer& c) { internal_avro::decode(d, c.re); }
};

template <>
struct codec_traits<Double> {
  static void decode(Decoder& d, Double& c) { internal_avro::decode(d, c.re); }
};

template<> struct codec_traits<PaddedRecord> {
  static void encode(Encoder& e, const PaddedRecord& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.padding);
  }

  static void decode(Decoder& d, PaddedRecord& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.padding);
  }
};
}

static ValidSchema makeValidSchema(const char* schema) {
  istringstream iss(schema);
  ValidSchema vs;
  compileJsonSchema(iss, vs);
  return ValidSchema(vs);
}

static const char sch[] =
    "{\"type\": \"record\","
    "\"name\":\"ComplexInteger\", \"fields\": ["
    "{\"name\":\"re\", \"type\":\"long\"},"
    "{\"name\":\"im\", \"type\":\"long\"}"
    "]}";
static const char isch[] =
    "{\"type\": \"record\","
    "\"name\":\"ComplexInteger\", \"fields\": ["
    "{\"name\":\"re\", \"type\":\"long\"}"
    "]}";
static const char dsch[] =
    "{\"type\": \"record\","
    "\"name\":\"ComplexDouble\", \"fields\": ["
    "{\"name\":\"re\", \"type\":\"double\"},"
    "{\"name\":\"im\", \"type\":\"double\"}"
    "]}";
static const char dblsch[] =
    "{\"type\": \"record\","
    "\"name\":\"ComplexDouble\", \"fields\": ["
    "{\"name\":\"re\", \"type\":\"double\"}"
    "]}";
static const char prsch[] = "{ \"type\" : \"record\","
    "\"name\" : \"PaddedRecord\", \"fields\" : ["
    "{ \"type\" : \"int\", \"name\" : \"index\" },"
    "{ \"type\" : \"bytes\", \"name\" : \"padding\" }"
    "]}";


string toString(const ValidSchema& s) {
  ostringstream oss;
  s.toJson(oss);
  return oss.str();
}

class DataFileTest {
  const char* filename;
  const ValidSchema writerSchema;
  const ValidSchema readerSchema;

 public:
  DataFileTest(const char* f, const char* wsch, const char* rsch)
      : filename(f),
        writerSchema(makeValidSchema(wsch)),
        readerSchema(makeValidSchema(rsch)) {}

  typedef pair<ValidSchema, GenericDatum> Pair;

  void testCleanup() { BOOST_CHECK(boost::filesystem::remove(filename)); }

  void testWrite() {
    internal_avro::DataFileWriter<ComplexInteger> df(filename, writerSchema,
                                                     100);
    int64_t re = 3;
    int64_t im = 5;
    for (int i = 0; i < count; ++i, re *= im, im += 3) {
      ComplexInteger c(re, im);
      df.write(c);
    }
    df.close();
  }

  void testWriteGeneric() {
    internal_avro::DataFileWriter<Pair> df(filename, writerSchema, 100);
    int64_t re = 3;
    int64_t im = 5;
    Pair p(writerSchema, GenericDatum());

    GenericDatum& c = p.second;
    c = GenericDatum(writerSchema.root());
    GenericRecord& r = c.value<GenericRecord>();

    for (int i = 0; i < count; ++i, re *= im, im += 3) {
      r.fieldAt(0) = re;
      r.fieldAt(1) = im;
      df.write(p);
    }
    df.close();
  }

  void testWriteDouble() {
    internal_avro::DataFileWriter<ComplexDouble> df(filename, writerSchema,
                                                    100);
    double re = 3.0;
    double im = 5.0;
    for (int i = 0; i < count; ++i, re += im - 0.7, im += 3.1) {
      ComplexDouble c(re, im);
      df.write(c);
    }
    df.close();
  }

  void testTruncate() {
    testWriteDouble();
    uintmax_t size = boost::filesystem::file_size(filename);
    {
      internal_avro::DataFileWriter<Pair> df(filename, writerSchema, 100);
      df.close();
    }
    uintmax_t new_size = boost::filesystem::file_size(filename);
    BOOST_CHECK(size > new_size);
  }

  void testReadFull() {
    internal_avro::DataFileReader<ComplexInteger> df(filename, writerSchema);
    int i = 0;
    ComplexInteger ci;
    int64_t re = 3;
    int64_t im = 5;
    while (df.read(ci)) {
      BOOST_CHECK_EQUAL(ci.re, re);
      BOOST_CHECK_EQUAL(ci.im, im);
      re *= im;
      im += 3;
      ++i;
    }
    BOOST_CHECK_EQUAL(i, count);
  }

  void testReadProjection() {
    internal_avro::DataFileReader<Integer> df(filename, readerSchema);
    int i = 0;
    Integer integer;
    int64_t re = 3;
    int64_t im = 5;
    while (df.read(integer)) {
      BOOST_CHECK_EQUAL(integer.re, re);
      re *= im;
      im += 3;
      ++i;
    }
    BOOST_CHECK_EQUAL(i, count);
  }

  void testReaderGeneric() {
    internal_avro::DataFileReader<Pair> df(filename, writerSchema);
    int i = 0;
    Pair p(writerSchema, GenericDatum());
    int64_t re = 3;
    int64_t im = 5;

    const GenericDatum& ci = p.second;
    while (df.read(p)) {
      BOOST_REQUIRE_EQUAL(ci.type(), internal_avro::AVRO_RECORD);
      const GenericRecord& r = ci.value<GenericRecord>();
      const size_t n = 2;
      BOOST_REQUIRE_EQUAL(r.fieldCount(), n);
      const GenericDatum& f0 = r.fieldAt(0);
      BOOST_REQUIRE_EQUAL(f0.type(), internal_avro::AVRO_LONG);
      BOOST_CHECK_EQUAL(f0.value<int64_t>(), re);

      const GenericDatum& f1 = r.fieldAt(1);
      BOOST_REQUIRE_EQUAL(f1.type(), internal_avro::AVRO_LONG);
      BOOST_CHECK_EQUAL(f1.value<int64_t>(), im);
      re *= im;
      im += 3;
      ++i;
    }
    BOOST_CHECK_EQUAL(i, count);
  }

  void testReaderGenericProjection() {
    internal_avro::DataFileReader<Pair> df(filename, readerSchema);
    int i = 0;
    Pair p(readerSchema, GenericDatum());
    int64_t re = 3;
    int64_t im = 5;

    const GenericDatum& ci = p.second;
    while (df.read(p)) {
      BOOST_REQUIRE_EQUAL(ci.type(), internal_avro::AVRO_RECORD);
      const GenericRecord& r = ci.value<GenericRecord>();
      const size_t n = 1;
      BOOST_REQUIRE_EQUAL(r.fieldCount(), n);
      const GenericDatum& f0 = r.fieldAt(0);
      BOOST_REQUIRE_EQUAL(f0.type(), internal_avro::AVRO_LONG);
      BOOST_CHECK_EQUAL(f0.value<int64_t>(), re);

      re *= im;
      im += 3;
      ++i;
    }
    BOOST_CHECK_EQUAL(i, count);
  }

  void testReadDouble() {
    internal_avro::DataFileReader<ComplexDouble> df(filename, writerSchema);
    int i = 0;
    ComplexDouble ci;
    double re = 3.0;
    double im = 5.0;
    while (df.read(ci)) {
      BOOST_CHECK_CLOSE(ci.re, re, 0.0001);
      BOOST_CHECK_CLOSE(ci.im, im, 0.0001);
      re += (im - 0.7);
      im += 3.1;
      ++i;
    }
    BOOST_CHECK_EQUAL(i, count);
  }

  /**
   * Constructs the DataFileReader in two steps.
   */
  void testReadDoubleTwoStep() {
    boost::shared_ptr<internal_avro::DataFileReaderBase> base(
        new internal_avro::DataFileReaderBase(filename));
    internal_avro::DataFileReader<ComplexDouble> df(base);
    BOOST_CHECK_EQUAL(toString(writerSchema), toString(df.readerSchema()));
    BOOST_CHECK_EQUAL(toString(writerSchema), toString(df.dataSchema()));
    int i = 0;
    ComplexDouble ci;
    double re = 3.0;
    double im = 5.0;
    while (df.read(ci)) {
      BOOST_CHECK_CLOSE(ci.re, re, 0.0001);
      BOOST_CHECK_CLOSE(ci.im, im, 0.0001);
      re += (im - 0.7);
      im += 3.1;
      ++i;
    }
    BOOST_CHECK_EQUAL(i, count);
  }

  /**
   * Constructs the DataFileReader in two steps using a different
   * reader schema.
   */
  void testReadDoubleTwoStepProject() {
    boost::shared_ptr<internal_avro::DataFileReaderBase> base(
        new internal_avro::DataFileReaderBase(filename));
    internal_avro::DataFileReader<Double> df(base, readerSchema);

    BOOST_CHECK_EQUAL(toString(readerSchema), toString(df.readerSchema()));
    BOOST_CHECK_EQUAL(toString(writerSchema), toString(df.dataSchema()));
    int i = 0;
    Double ci;
    double re = 3.0;
    double im = 5.0;
    while (df.read(ci)) {
      BOOST_CHECK_CLOSE(ci.re, re, 0.0001);
      re += (im - 0.7);
      im += 3.1;
      ++i;
    }
    BOOST_CHECK_EQUAL(i, count);
  }

  /**
   * Test various seek operations.
   */
  void testSeeks() {
    const size_t padding_size = 10000;
    const size_t number_of_objects = 1000;
    // first create a large file
    ValidSchema dschema = internal_avro::compileJsonSchemaFromString(prsch);
    {
      boost::shared_ptr<internal_avro::DataFileWriter<PaddedRecord> > writer(
          new internal_avro::DataFileWriter<PaddedRecord>(filename, dschema));

      for (size_t i = 0; i < number_of_objects; ++i) {
        PaddedRecord d;
        d.index = i;
        d.padding.resize(padding_size);
        for (size_t j = 0; j < padding_size; ++j) {
          // make sure all bytes appear
          d.padding[j] = j % 256;
        }
        writer->write(d);
      }
    }

    // check seeking to the start and end
    {
      boost::shared_ptr<internal_avro::DataFileReader<PaddedRecord> > reader(
          new internal_avro::DataFileReader<PaddedRecord>(filename));
      BOOST_REQUIRE_NE(reader->sizeBytes(), -1);
      // test that seek to the start gets the first element
      reader->seekBlockBytes(reader->blockOffsetBytes());
      PaddedRecord d;
      BOOST_REQUIRE(reader->read(d));
      BOOST_CHECK_EQUAL(d.index, 0);

      reader->seekBlockBytes(reader->sizeBytes());
      BOOST_CHECK(!reader->read(d));
    }

    // check that all members are found
    std::set<size_t> block_offsets;
    {
      std::vector<int> dividers;
      {
        boost::shared_ptr<internal_avro::DataFileReader<PaddedRecord> > reader(
            new internal_avro::DataFileReader<PaddedRecord>(filename));
        int size = reader->sizeBytes();
        dividers.push_back(reader->blockOffsetBytes());
        int chunk_size = (size - dividers[0]) / 20;
        BOOST_REQUIRE_GT(chunk_size, 0);
        for (int i = 1; i < 20; ++i) {
          dividers.push_back(chunk_size * i + dividers[0]);
        }
        BOOST_REQUIRE_GT(size, dividers.back());
        dividers.push_back(size);
      }
      std::vector<int> found;
      for (size_t i = 1; i < dividers.size(); ++i) {
        boost::shared_ptr<internal_avro::DataFileReader<PaddedRecord> > reader(
            new internal_avro::DataFileReader<PaddedRecord>(filename));
        reader->seekBlockBytes(dividers[i - 1]);
        block_offsets.insert(reader->blockOffsetBytes());
        BOOST_REQUIRE_GE(reader->blockOffsetBytes(), dividers[i - 1]);
        do {
          PaddedRecord d;
          if (!reader->read(d)) break;
          if (reader->blockOffsetBytes() > dividers[i]) break;
          found.push_back(d.index);
        } while (true);
      }
      BOOST_CHECK_EQUAL(found.size(), number_of_objects);
      for (unsigned int i = 0; i < found.size(); ++i) {
        BOOST_CHECK_EQUAL(found[i], i);
      }
    }

    // check that all the block offsets are stable
    {
      for (std::set<size_t>::const_iterator it = block_offsets.begin();
           it != block_offsets.end(); ++it) {
        boost::shared_ptr<internal_avro::DataFileReader<PaddedRecord> > reader(
            new internal_avro::DataFileReader<PaddedRecord>(filename));
        reader->seekBlockBytes(*it);
        BOOST_CHECK_EQUAL(reader->blockOffsetBytes(), *it);
      }
    }
  }

  /**
   * Test writing DataFiles into other streams operations.
   */
  void testBuffer() {
    const size_t padding_size = 1000;
    const size_t number_of_objects = 100;
    // first create a large file
    ValidSchema dschema = internal_avro::compileJsonSchemaFromString(prsch);
    boost::shared_ptr<internal_avro::OutputStream> buf =
        internal_avro::memoryOutputStream();
    {
      boost::shared_ptr<internal_avro::DataFileWriter<PaddedRecord> > writer =
          boost::make_shared<internal_avro::DataFileWriter<PaddedRecord> >(
              buf, dschema);

      for (size_t i = 0; i < number_of_objects; ++i) {
        PaddedRecord d;
        d.index = i;
        d.padding.resize(padding_size);
        for (size_t j = 0; j < padding_size; ++j) {
          // make sure all bytes appear
          d.padding[j] = j % 256;
        }
        writer->write(d);
      }
    }
    {
      {
        boost::shared_ptr<internal_avro::InputStream> inbuf =
            internal_avro::memoryInputStream(*buf);
        boost::shared_ptr<internal_avro::DataFileReader<PaddedRecord> > reader =
            boost::make_shared<internal_avro::DataFileReader<PaddedRecord> >(
                inbuf);
        std::vector<int> found;
        PaddedRecord record;
        while (reader->read(record)) {
          found.push_back(record.index);
        }
        BOOST_CHECK_EQUAL(found.size(), number_of_objects);
        for (unsigned int i = 0; i < found.size(); ++i) {
          BOOST_CHECK_EQUAL(found[i], i);
        }
      }
    }
  }

  /**
   * Test writing DataFiles into other streams operations.
   */
  void testZip() {
    const size_t padding_size = 1000;
    const size_t number_of_objects = 100;
    // first create a large file
    ValidSchema dschema = internal_avro::compileJsonSchemaFromString(prsch);
    {
      boost::shared_ptr<internal_avro::DataFileWriter<PaddedRecord> > writer =
          boost::make_shared<internal_avro::DataFileWriter<PaddedRecord> >(
              filename, dschema, 16 * 1024, internal_avro::DEFLATE_CODEC);

      for (size_t i = 0; i < number_of_objects; ++i) {
        PaddedRecord d;
        d.index = i;
        d.padding.resize(padding_size);
        for (size_t j = 0; j < padding_size; ++j) {
          // make sure all bytes appear
          d.padding[j] = j % 256;
        }
        writer->write(d);
      }
    }
    {
      {
        boost::shared_ptr<internal_avro::DataFileReader<PaddedRecord> > reader =
            boost::make_shared<internal_avro::DataFileReader<PaddedRecord> >(
                filename, dschema);
        std::vector<int> found;
        PaddedRecord record;
        while (reader->read(record)) {
          found.push_back(record.index);
        }
        BOOST_CHECK_EQUAL(found.size(), number_of_objects);
        for (unsigned int i = 0; i < found.size(); ++i) {
          BOOST_CHECK_EQUAL(found[i], i);
        }
      }
    }
  }
};

void addReaderTests(test_suite* ts, const shared_ptr<DataFileTest>& t) {
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testReadFull, t));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testReadProjection, t));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testReaderGeneric, t));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testReaderGenericProjection, t));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testCleanup, t));
}

test_suite* init_unit_test_suite(int argc, char* argv[]) {
  test_suite* ts = BOOST_TEST_SUITE("DataFile tests");
  shared_ptr<DataFileTest> t1(new DataFileTest("test1.df", sch, isch));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testWrite, t1));
  addReaderTests(ts, t1);

  shared_ptr<DataFileTest> t2(new DataFileTest("test2.df", sch, isch));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testWriteGeneric, t2));
  addReaderTests(ts, t2);

  shared_ptr<DataFileTest> t3(new DataFileTest("test3.df", dsch, dblsch));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testWriteDouble, t3));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testReadDouble, t3));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testReadDoubleTwoStep, t3));
  ts->add(
      BOOST_CLASS_TEST_CASE(&DataFileTest::testReadDoubleTwoStepProject, t3));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testCleanup, t3));

  shared_ptr<DataFileTest> t4(new DataFileTest("test4.df", dsch, dblsch));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testTruncate, t4));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testCleanup, t4));

  shared_ptr<DataFileTest> t5(new DataFileTest("test5.df", dsch, dblsch));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testSeeks, t5));

  shared_ptr<DataFileTest> t6(new DataFileTest("test6.df", dsch, dblsch));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testBuffer, t6));

  shared_ptr<DataFileTest> t7(new DataFileTest("test7.df", dsch, dblsch));
  ts->add(BOOST_CLASS_TEST_CASE(&DataFileTest::testZip, t7));

  return ts;
}
