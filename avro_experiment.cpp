#include <iostream>
#include <thread>
#include <future>
#include <fstream>
#include <fmt/core.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
// https://avro.apache.org/docs/1.11.1/api/cpp/html/
#include <avro/Encoder.hh>
#include <avro/Decoder.hh>
#include <avro/ValidSchema.hh>
#include <avro/Compiler.hh>
#include "model.hh"

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;

/*
struct cpx
{
    double re;
    double im;
};
*/

class custom_error_handler : public nlohmann::json_schema::basic_error_handler
{
    void error(const json::json_pointer & pointer, const json & instance, const std::string & message) override
    {
        nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
        std::cerr << "ERROR: '" << pointer << "' - '" << instance << "': " << message << "\n";
    }
};

int main() {

    // https://avro.apache.org/docs/1.3.0/api/cpp/html/index.html

    std::ifstream f("data.json");
    json payload = json::parse(f);

    std::ifstream f2("schema.json");
    json schema = json::parse(f2);

    std::ifstream f3("schema.json");
    avro::ValidSchema cpxSchema;
    avro::compileJsonSchema(f3, cpxSchema);

    std::unique_ptr<avro::OutputStream> out = avro::memoryOutputStream();
    // avro::EncoderPtr e = avro::binaryEncoder();
    avro::EncoderPtr e = avro::validatingEncoder(cpxSchema, avro::binaryEncoder());
    e->init(*out);
    c::cpx c1;
    c1.re = 1.0;
    c1.im = 2.13;
    avro::encode(*e, c1);

    std::unique_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
    // avro::DecoderPtr d = avro::binaryDecoder();
    avro::DecoderPtr d = avro::validatingDecoder(cpxSchema, avro::binaryDecoder());
    d->init(*in);

    c::cpx c2;
    avro::decode(*d, c2);
    std::cout << '(' << c2.re << ", " << c2.im << ')' << std::endl;

    /*
    c::cpx c1;
    c1.re = 1.0;
    c1.im = 2.13;
    avro::encode(*e, c1);
    */

    /*
    c::cpx c2;
    avro::decode(*d, c2);
    */

    zmq::context_t context(0);

    auto result = std::async(std::launch::async, [&]() {
        zmq::socket_t subscriber(context, zmq::socket_type::pull);
        subscriber.connect("inproc://#1");

        std::vector<zmq::message_t> recv_messages;
        while(true)
        {
            recv_messages.clear();
            zmq::recv_result_t result = zmq::recv_multipart(subscriber, std::back_inserter(recv_messages));
            assert(result && "recv failed");
            size_t count = 0;
            for (const auto& msg : recv_messages)
            {
                json dataset = msg.to_string();
                fmt::print("ok{} received {}\n", count, dataset.dump());
                ++count;
            }
        }
    });

    zmq::socket_t publisher(context, zmq::socket_type::push);
    publisher.bind("inproc://#1");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    json parameters = {
        {"happy", true},
        {"pi", 3.141},
    };
    /*
    json payload = {
        {"metadata", {
            {"id", "123"},
            {"timestamp", "2021-01-01T00:00:00Z"},
            {"source", "source1"},
            {"type", "type1"}
        }},
        {"name", "job1"},
        {"parameters", parameters}
    };
    */

    // validate payload
    /*
    json_validator validator;
    validator.set_root_schema(schema);
    custom_error_handler err;
    validator.validate(payload, err);
    */

    fmt::print("{}\n", payload.dump());

    while(true)
    {
        zmq::message_t msg(schema.dump());
        publisher.send(msg, zmq::send_flags::sndmore);

        zmq::message_t msg2(payload.dump());
        publisher.send(msg, zmq::send_flags::sndmore);

        zmq::message_t msg3(payload.dump());
        publisher.send(msg3, zmq::send_flags::none);

        fmt::print("ok sent json\n");
    }
    return 0;
}
