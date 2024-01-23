/**
 * @file order_parser.hpp
 * @author xiaoyc
 * @brief JSON格式订单解析
 * @version 0.1
 * @date 2024-01-22
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef SAMPLES_ORDER_PARSER_HPP_
#define SAMPLES_ORDER_PARSER_HPP_

// C++ Standard
#include <string>
#include <vector>

// Boost
#include <boost/json/src.hpp>

namespace json = boost::json;

namespace parser {
struct Input {
  std::string filename;
  bool is_raw;

  friend Input tag_invoke(boost::json::value_to_tag<Input>,
                          boost::json::value const& v);
};

struct Coeff {
  std::string dark_a;
  std::string dark_b;
  std::string rel_a;
  std::string rel_b;
  std::string etalon_a;
  std::string etalon_b;
  std::string badpixel;

  friend Coeff tag_invoke(boost::json::value_to_tag<Coeff>,
                          boost::json::value const& v);
};

struct Order {
  std::vector<Input> inputs;
  Coeff coeff;
  std::vector<std::string> outputs;
  friend Order tag_invoke(boost::json::value_to_tag<Order>,
                          boost::json::value const& v);
};

template <class T>
void extract(json::object const& obj, T& t, const std::string& key) {
  try {
    t = json::value_to<T>(obj.at(key));
  } catch (const std::out_of_range& e) {
    std::cerr << e.what() << "\n";
  }
}

Input tag_invoke(boost::json::value_to_tag<Input>,
                 boost::json::value const& v) {
  auto const& obj = v.as_object();
  Input input;
  extract(obj, input.filename, "filename");
  extract(obj, input.is_raw, "raw");
  return input;
}

Order tag_invoke(json::value_to_tag<Order>, json::value const& v) {
  auto const& obj = v.as_object();
  Order order;
  extract(obj, order.inputs, "input");
  extract(obj, order.coeff, "coeff");
  extract(obj, order.outputs, "output");
  return order;
}

Coeff tag_invoke(json::value_to_tag<Coeff>, json::value const& v) {
  auto const& obj = v.as_object();
  Coeff coeff;
  extract(obj, coeff.dark_a, "dark_a");
  extract(obj, coeff.dark_b, "dark_b");
  extract(obj, coeff.badpixel, "badpixel");
  extract(obj, coeff.rel_a, "rel_a");
  extract(obj, coeff.rel_b, "rel_b");
  extract(obj, coeff.etalon_a, "etalon_a");
  extract(obj, coeff.etalon_b, "etalon_b");
  return coeff;
}

}  // namespace parser

#endif  // SAMPLES_ORDER_PARSER_HPP_
