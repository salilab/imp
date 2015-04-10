
#include "traits.h"
#include "raw_frame.h"

RMF_ENABLE_WARNINGS

namespace {
const std::string old_schema_json =
    "{    \"type\": \"record\",    \"name\": \"Frame\",    \"fields\": [     "
    "   {            \"name\": \"info\",            \"type\": [              "
    "  {                    \"type\": \"record\",                    "
    "\"name\": \"FrameInfo\",                    \"fields\": [               "
    "         {                            \"name\": \"id\",                 "
    "           \"type\": \"int\"                        },                  "
    "      {                            \"name\": \"name\",                  "
    "          \"type\": \"string\"                        },                "
    "        {                            \"name\": \"type\",                "
    "            \"type\": \"int\"                        },                 "
    "       {                            \"name\": \"parents\",              "
    "              \"type\": {                                \"type\": "
    "\"array\",                                \"items\": \"int\"            "
    "                }                        }                    ]         "
    "       },                {                    \"type\": \"record\",     "
    "               \"name\": \"FileInfo\",                    \"fields\": [ "
    "                       {                            \"name\": "
    "\"description\",                            \"type\": \"string\"        "
    "                },                        {                            "
    "\"name\": \"producer\",                            \"type\": \"string\" "
    "                       },                        {                      "
    "      \"name\": \"categories\",                            \"type\": {  "
    "                              \"type\": \"array\",                      "
    "          \"items\": {                                    \"type\": "
    "\"record\",                                    \"name\": \"Label\",     "
    "                               \"fields\": [                            "
    "            {                                            \"name\": "
    "\"id\",                                            \"type\": \"int\"    "
    "                                    },                                  "
    "      {                                            \"name\": \"name\",  "
    "                                          \"type\": \"string\"          "
    "                              }                                    ]    "
    "                            }                            }              "
    "          },                        {                            "
    "\"name\": \"node_types\",                            \"type\": {        "
    "                        \"type\": \"array\",                            "
    "    \"items\": \"Label\"                            }                   "
    "     },                        {                            \"name\": "
    "\"frame_types\",                            \"type\": {                 "
    "               \"type\": \"array\",                                "
    "\"items\": \"Label\"                            }                       "
    " },                        {                            \"name\": "
    "\"node_sets\",                            \"type\": {                   "
    "             \"type\": \"array\",                                "
    "\"items\": {                                    \"type\": \"record\",   "
    "                                 \"name\": \"NodeSet\",                 "
    "                   \"fields\": [                                        "
    "{                                            \"name\": \"id\",          "
    "                                  \"type\": \"int\"                     "
    "                   },                                        {          "
    "                                  \"name\": \"nodes\",                  "
    "                          \"type\": {                                   "
    "             \"type\": \"array\",                                       "
    "         \"items\": \"int\"                                            "
    "}                                        }                              "
    "      ]                                }                            }   "
    "                     }                    ]                }            "
    "]        },        {            \"name\": \"nodes\",            "
    "\"type\": {                \"type\": \"array\",                "
    "\"items\": {                    \"type\": \"record\",                   "
    " \"name\": \"Node\",                    \"fields\": [                   "
    "     {                            \"name\": \"id\",                     "
    "       \"type\": \"int\"                        },                      "
    "  {                            \"name\": \"name\",                      "
    "      \"type\": \"string\"                        },                    "
    "    {                            \"name\": \"type\",                    "
    "        \"type\": \"int\"                        },                     "
    "   {                            \"name\": \"parents\",                  "
    "          \"type\": {                                \"type\": "
    "\"array\",                                \"items\": \"int\"            "
    "                }                        }                    ]         "
    "       }            }        },        {            \"name\": \"keys\", "
    "           \"type\": {                \"type\": \"array\",              "
    "  \"items\": {                    \"type\": \"record\",                 "
    "   \"name\": \"KeyInfo\",                    \"fields\": [              "
    "          {                            \"name\": \"id\",                "
    "            \"type\": \"int\"                        },                 "
    "       {                            \"name\": \"name\",                 "
    "           \"type\": \"string\"                        },               "
    "         {                            \"name\": \"category\",           "
    "                 \"type\": \"int\"                        },            "
    "            {                            \"name\": \"type\",            "
    "                \"type\": {                                \"type\": "
    "\"enum\",                                \"name\": \"Type\",            "
    "                    \"symbols\": [                                    "
    "\"INT\",                                    \"FLOAT\",                  "
    "                  \"STRING\",                                    "
    "\"INTS\",                                    \"FLOATS\",                "
    "                    \"STRINGS\",                                    "
    "\"VECTOR3\",                                    \"VECTOR4\",            "
    "                        \"VECTOR3S\"                                ]   "
    "                         }                        }                    "
    "]                }            }        },        {            \"name\": "
    "\"data\",            \"type\": {                \"type\": \"record\",   "
    "             \"name\": \"TypeDatas\",                \"fields\": [      "
    "              {                        \"name\": \"int_data\",          "
    "              \"type\": {                            \"type\": "
    "\"array\",                            \"items\": {                      "
    "          \"type\": \"record\",                                "
    "\"name\": \"IntNodeData\",                                \"fields\": [ "
    "                                   {                                    "
    "    \"name\": \"key\",                                        \"type\": "
    "\"int\"                                    },                           "
    "         {                                        \"name\": \"values\", "
    "                                       \"type\": {                      "
    "                      \"type\": \"array\",                              "
    "              \"items\": {                                              "
    "  \"type\": \"record\",                                                "
    "\"name\": \"IntValue\",                                                "
    "\"fields\": [                                                    {      "
    "                                                  \"name\": \"id\",     "
    "                                                   \"type\": \"int\"    "
    "                                                },                      "
    "                              {                                         "
    "               \"name\": \"value\",                                     "
    "                   \"type\": \"int\"                                    "
    "                }                                                ]      "
    "                                      }                                 "
    "       }                                    }                           "
    "     ]                            }                        }            "
    "        },                    {                        \"name\": "
    "\"float_data\",                        \"type\": {                      "
    "      \"type\": \"array\",                            \"items\": {      "
    "                          \"type\": \"record\",                         "
    "       \"name\": \"FloatNodeData\",                                "
    "\"fields\": [                                    {                      "
    "                  \"name\": \"key\",                                    "
    "    \"type\": \"int\"                                    },             "
    "                       {                                        "
    "\"name\": \"values\",                                        \"type\": "
    "{                                            \"type\": \"array\",       "
    "                                     \"items\": {                       "
    "                         \"type\": \"record\",                          "
    "                      \"name\": \"FloatValue\",                         "
    "                       \"fields\": [                                    "
    "                {                                                       "
    " \"name\": \"id\",                                                      "
    "  \"type\": \"int\"                                                    "
    "},                                                    {                 "
    "                                       \"name\": \"value\",             "
    "                                           \"type\": \"float\"          "
    "                                          }                             "
    "                   ]                                            }       "
    "                                 }                                    } "
    "                               ]                            }           "
    "             }                    },                    {               "
    "         \"name\": \"string_data\",                        \"type\": {  "
    "                          \"type\": \"array\",                          "
    "  \"items\": {                                \"type\": \"record\",     "
    "                           \"name\": \"StringNodeData\",                "
    "                \"fields\": [                                    {      "
    "                                  \"name\": \"key\",                    "
    "                    \"type\": \"int\"                                   "
    " },                                    {                                "
    "        \"name\": \"values\",                                        "
    "\"type\": {                                            \"type\": "
    "\"array\",                                            \"items\": {      "
    "                                          \"type\": \"record\",         "
    "                                       \"name\": \"StringValue\",       "
    "                                         \"fields\": [                  "
    "                                  {                                     "
    "                   \"name\": \"id\",                                    "
    "                    \"type\": \"int\"                                   "
    "                 },                                                    "
    "{                                                        \"name\": "
    "\"value\",                                                        "
    "\"type\": \"string\"                                                    "
    "}                                                ]                      "
    "                      }                                        }        "
    "                            }                                ]          "
    "                  }                        }                    },      "
    "              {                        \"name\": \"ints_data\",         "
    "               \"type\": {                            \"type\": "
    "\"array\",                            \"items\": {                      "
    "          \"type\": \"record\",                                "
    "\"name\": \"IntsNodeData\",                                \"fields\": "
    "[                                    {                                  "
    "      \"name\": \"key\",                                        "
    "\"type\": \"int\"                                    },                 "
    "                   {                                        \"name\": "
    "\"values\",                                        \"type\": {          "
    "                                  \"type\": \"array\",                  "
    "                          \"items\": {                                  "
    "              \"type\": \"record\",                                     "
    "           \"name\": \"IntsValue\",                                     "
    "           \"fields\": [                                                "
    "    {                                                        \"name\": "
    "\"key\",                                                        "
    "\"type\": \"int\"                                                    }, "
    "                                                   {                    "
    "                                    \"name\": \"value\",                "
    "                                        \"type\": {                     "
    "                                       \"type\": \"array\",             "
    "                                               \"items\": \"int\"       "
    "                                                 }                      "
    "                              }                                         "
    "       ]                                            }                   "
    "                     }                                    }             "
    "                   ]                            }                       "
    " }                    },                    {                        "
    "\"name\": \"floats_data\",                        \"type\": {           "
    "                 \"type\": \"array\",                            "
    "\"items\": {                                \"type\": \"record\",       "
    "                         \"name\": \"FloatsNodeData\",                  "
    "              \"fields\": [                                    {        "
    "                                \"name\": \"id\",                       "
    "                 \"type\": \"int\"                                    "
    "},                                    {                                 "
    "       \"name\": \"values\",                                        "
    "\"type\": {                                            \"type\": "
    "\"array\",                                            \"items\": {      "
    "                                          \"type\": \"record\",         "
    "                                       \"name\": \"FloatsValue\",       "
    "                                         \"fields\": [                  "
    "                                  {                                     "
    "                   \"name\": \"id\",                                    "
    "                    \"type\": \"int\"                                   "
    "                 },                                                    "
    "{                                                        \"name\": "
    "\"value\",                                                        "
    "\"type\": {                                                            "
    "\"type\": \"array\",                                                    "
    "        \"items\": \"float\"                                            "
    "            }                                                    }      "
    "                                          ]                             "
    "               }                                        }               "
    "                     }                                ]                 "
    "           }                        }                    },             "
    "       {                        \"name\": \"strings_data\",             "
    "           \"type\": {                            \"type\": \"array\",  "
    "                          \"items\": {                                "
    "\"type\": \"record\",                                \"name\": "
    "\"StringsNodeData\",                                \"fields\": [       "
    "                             {                                        "
    "\"name\": \"id\",                                        \"type\": "
    "\"int\"                                    },                           "
    "         {                                        \"name\": \"values\", "
    "                                       \"type\": {                      "
    "                      \"type\": \"array\",                              "
    "              \"items\": {                                              "
    "  \"type\": \"record\",                                                "
    "\"name\": \"StringsValue\",                                             "
    "   \"fields\": [                                                    {   "
    "                                                     \"name\": \"id\",  "
    "                                                      \"type\": \"int\" "
    "                                                   },                   "
    "                                 {                                      "
    "                  \"name\": \"value\",                                  "
    "                      \"type\": {                                       "
    "                     \"type\": \"array\",                               "
    "                             \"items\": \"string\"                      "
    "                                  }                                     "
    "               }                                                ]       "
    "                                     }                                  "
    "      }                                    }                            "
    "    ]                            }                        }             "
    "       },                    {                        \"name\": "
    "\"vector3_data\",                        \"type\": {                    "
    "        \"type\": \"array\",                            \"items\": {    "
    "                            \"type\": \"record\",                       "
    "         \"name\": \"Vector3NodeData\",                                "
    "\"fields\": [                                    {                      "
    "                  \"name\": \"key\",                                    "
    "    \"type\": \"int\"                                    },             "
    "                       {                                        "
    "\"name\": \"values\",                                        \"type\": "
    "{                                            \"type\": \"array\",       "
    "                                     \"items\": {                       "
    "                         \"type\": \"record\",                          "
    "                      \"name\": \"Vector3Value\",                       "
    "                         \"fields\": [                                  "
    "                  {                                                     "
    "   \"name\": \"id\",                                                    "
    "    \"type\": \"int\"                                                   "
    " },                                                    {                "
    "                                        \"name\": \"value\",            "
    "                                            \"type\": {                 "
    "                                           \"type\": \"record\",        "
    "                                                    \"name\": "
    "\"Vector3\",                                                            "
    "\"fields\": [                                                           "
    "     {                                                                  "
    "  \"name\": \"x\",                                                      "
    "              \"type\": \"float\"                                       "
    "                         },                                             "
    "                   {                                                    "
    "                \"name\": \"y\",                                        "
    "                            \"type\": \"float\"                         "
    "                                       },                               "
    "                                 {                                      "
    "                              \"name\": \"z\",                          "
    "                                          \"type\": \"float\"           "
    "                                                     }                  "
    "                                          ]                             "
    "                           }                                            "
    "        }                                                ]              "
    "                              }                                        "
    "}                                    }                                ] "
    "                           }                        }                   "
    " },                    {                        \"name\": "
    "\"vector4_data\",                        \"type\": {                    "
    "        \"type\": \"array\",                            \"items\": {    "
    "                            \"type\": \"record\",                       "
    "         \"name\": \"Vector4NodeData\",                                "
    "\"fields\": [                                    {                      "
    "                  \"name\": \"key\",                                    "
    "    \"type\": \"int\"                                    },             "
    "                       {                                        "
    "\"name\": \"values\",                                        \"type\": "
    "{                                            \"type\": \"array\",       "
    "                                     \"items\": {                       "
    "                         \"type\": \"record\",                          "
    "                      \"name\": \"Vector4Value\",                       "
    "                         \"fields\": [                                  "
    "                  {                                                     "
    "   \"name\": \"id\",                                                    "
    "    \"type\": \"int\"                                                   "
    " },                                                    {                "
    "                                        \"name\": \"value\",            "
    "                                            \"type\": {                 "
    "                                           \"type\": \"record\",        "
    "                                                    \"name\": "
    "\"Vector4\",                                                            "
    "\"fields\": [                                                           "
    "     {                                                                  "
    "  \"name\": \"w\",                                                      "
    "              \"type\": \"float\"                                       "
    "                         },                                             "
    "                   {                                                    "
    "                \"name\": \"x\",                                        "
    "                            \"type\": \"float\"                         "
    "                                       },                               "
    "                                 {                                      "
    "                              \"name\": \"y\",                          "
    "                                          \"type\": \"float\"           "
    "                                                     },                 "
    "                                               {                        "
    "                                            \"name\": \"z\",            "
    "                                                        \"type\": "
    "\"float\"                                                               "
    " }                                                            ]         "
    "                                               }                        "
    "                            }                                           "
    "     ]                                            }                     "
    "                   }                                    }               "
    "                 ]                            }                        "
    "}                    },                    {                        "
    "\"name\": \"vector3s_data\",                        \"type\": {         "
    "                   \"type\": \"array\",                            "
    "\"items\": {                                \"type\": \"record\",       "
    "                         \"name\": \"Vector3sNodeData\",                "
    "                \"fields\": [                                    {      "
    "                                  \"name\": \"key\",                    "
    "                    \"type\": \"int\"                                   "
    " },                                    {                                "
    "        \"name\": \"values\",                                        "
    "\"type\": {                                            \"type\": "
    "\"array\",                                            \"items\": {      "
    "                                          \"type\": \"record\",         "
    "                                       \"name\": \"Vector3sValue\",     "
    "                                           \"fields\": [                "
    "                                    {                                   "
    "                     \"name\": \"id\",                                  "
    "                      \"type\": \"int\"                                 "
    "                   },                                                   "
    " {                                                        \"name\": "
    "\"value\",                                                        "
    "\"type\": {                                                            "
    "\"type\": \"array\",                                                    "
    "        \"items\": \"Vector3\"                                          "
    "              }                                                    }    "
    "                                            ]                           "
    "                 }                                        }             "
    "                       }                                ]               "
    "             }                        }                    }            "
    "    ]            }        }    ]}";
const internal_avro::ValidSchema valid_backwards_schema =
    internal_avro::compileJsonSchemaFromString(old_schema_json);

struct BackwardsTypeDatas {
  std::vector<rmf_raw_avro2::IntNodeData> int_data;
  std::vector<rmf_raw_avro2::FloatNodeData> float_data;
  std::vector<rmf_raw_avro2::StringNodeData> string_data;
  std::vector<rmf_raw_avro2::Vector3NodeData> vector3_data;
  std::vector<rmf_raw_avro2::Vector4NodeData> vector4_data;
  std::vector<rmf_raw_avro2::IntsNodeData> ints_data;
  std::vector<rmf_raw_avro2::FloatsNodeData> floats_data;
  std::vector<rmf_raw_avro2::StringsNodeData> strings_data;
  std::vector<rmf_raw_avro2::Vector3sNodeData> vector3s_data;
  std::vector<rmf_raw_avro2::Vector4sNodeData> vector4s_data;
};

struct BackwardsFrame {
  typedef rmf_raw_avro2::_Frame_json_Union__0__ info_t;
  info_t info;
  std::vector<rmf_raw_avro2::Node> nodes;
  std::vector<rmf_raw_avro2::KeyInfo> keys;
  BackwardsTypeDatas data;
};
}
namespace internal_avro {

template <>
struct codec_traits<BackwardsTypeDatas> {
  static void encode(Encoder& e, const BackwardsTypeDatas& v) {
    internal_avro::encode(e, v.int_data);
    internal_avro::encode(e, v.float_data);
    internal_avro::encode(e, v.string_data);
    internal_avro::encode(e, v.ints_data);
    internal_avro::encode(e, v.floats_data);
    internal_avro::encode(e, v.strings_data);
    internal_avro::encode(e, v.vector3_data);
    internal_avro::encode(e, v.vector4_data);
    internal_avro::encode(e, v.vector3s_data);
  }
  static void decode(Decoder& d, BackwardsTypeDatas& v) {
    internal_avro::decode(d, v.int_data);
    internal_avro::decode(d, v.float_data);
    internal_avro::decode(d, v.string_data);
    internal_avro::decode(d, v.ints_data);
    internal_avro::decode(d, v.floats_data);
    internal_avro::decode(d, v.strings_data);
    internal_avro::decode(d, v.vector3_data);
    internal_avro::decode(d, v.vector4_data);
    internal_avro::decode(d, v.vector3s_data);
  }
};

template <>
struct codec_traits<BackwardsFrame> {
  template <class Encoder>
  static void encode(Encoder& e, const BackwardsFrame& v) {
    internal_avro::encode(e, v.info);
    internal_avro::encode(e, v.nodes);
    internal_avro::encode(e, v.keys);
    internal_avro::encode(e, v.data);
  }
  template <class Decoder>
  static void decode(Decoder& d, BackwardsFrame& v) {
    internal_avro::decode(d, v.info);
    internal_avro::decode(d, v.nodes);
    internal_avro::decode(d, v.keys);
    internal_avro::decode(d, v.data);
  }
};
}

namespace RMF {
namespace avro2 {

void flush_buffer(boost::shared_ptr<internal_avro::DataFileWriterBase> writer,
                  boost::shared_ptr<internal_avro::OutputStream> stream,
                  BufferHandle buffer) {
  RMF_INFO("Flushing to buffer");
  // avoid rewriting later
  writer->flush();
  buffer.access_buffer().clear();
  boost::shared_ptr<internal_avro::InputStream> input_stream =
      internal_avro::memoryInputStream(*stream);
  const uint8_t* data;
  size_t len;
  while (input_stream->next(&data, &len)) {
    buffer.access_buffer().insert(buffer.access_buffer().end(), data,
                                  data + len);
  }
}

BufferConstHandle try_convert(BufferConstHandle buffer, std::string message) {
  boost::shared_ptr<internal_avro::InputStream> stream =
      internal_avro::memoryInputStream(buffer.get_uint8_t().first,
                                       buffer.get_uint8_t().second);

  boost::shared_ptr<internal_avro::DataFileReader<BackwardsFrame> > reader;
  try {
    reader = boost::make_shared<internal_avro::DataFileReader<BackwardsFrame> >(
        stream, valid_backwards_schema);
  }
  catch (std::exception e) {
    RMF_THROW(Message(message + " and " + e.what()), IOException);
  }

  boost::shared_ptr<internal_avro::OutputStream> out_stream =
      internal_avro::memoryOutputStream();
  boost::shared_ptr<internal_avro::DataFileWriterBase> writer =
      boost::make_shared<internal_avro::DataFileWriterBase>(
          out_stream, internal_avro::compileJsonSchemaFromString(
                          RMF::data_avro::frame_json),
          16 * 1024, internal_avro::DEFLATE_CODEC);

  BackwardsFrame bf;
  while (reader->read(bf)) {
    rmf_raw_avro2::Frame out;
    if (bf.info.idx() == 0) {
      out.info.set_FrameInfo(bf.info.get_FrameInfo());
    } else {
      out.info.set_FileInfo(bf.info.get_FileInfo());
    }
    out.info = bf.info;
    out.nodes = bf.nodes;
    out.keys = bf.keys;
    out.data.int_data = bf.data.int_data;
    out.data.float_data = bf.data.float_data;
    out.data.string_data = bf.data.string_data;
    out.data.ints_data = bf.data.ints_data;
    out.data.floats_data = bf.data.floats_data;
    out.data.strings_data = bf.data.strings_data;
    out.data.vector3_data = bf.data.vector3_data;
    out.data.vector4_data = bf.data.vector4_data;
    out.data.vector3s_data = bf.data.vector3s_data;
    writer->syncIfNeeded();
    internal_avro::encode(writer->encoder(), out);
    writer->incr();
  }
  BufferHandle ret;
  flush_buffer(writer, out_stream, ret);
  return ret;
}
}
}

RMF_DISABLE_WARNINGS
