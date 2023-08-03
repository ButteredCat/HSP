#include "hycodec/HgyNIRDecoder.h"
#include "hycodec/LineScanData.h"
#include "pch.h"
#include "radiometric.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  GDALAllRegister();
  try {
    // Declare a group of options that will be
    // allowed only on command line
    po::options_description generic("Generic options");
    generic.add_options()("version", "print version string")(
        "help", "produce help message")("config,c", po::value<std::string>(),
                                        "config file");

    // Declare a group of options that will be
    // allowed both on command line and in
    // config file
    int opt;
    po::options_description config("Configuration");
    config.add_options()("output-dir,o", po::value<std::string>(),
                         "output directory")("gain", po::value<std::string>(),
                                             "relative coefficient a (gain)")(
        "offset", po::value<std::string>(), "relative coefficient b (offset)")(
        "dark", po::value<std::string>(), "dark background")(
        "dp", po::value<std::string>(), "defect pixel list");

    // Hidden options, will be allowed both on command line and
    // in config file, but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()("input-file", po::value<std::vector<std::string>>(),
                         "input file");

    po::positional_options_description positional;
    positional.add("input-file", -1);

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config).add(hidden);

    po::options_description config_file_options;
    config_file_options.add(config).add(hidden);

    po::options_description visible("Allowed options");
    visible.add(generic).add(config);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
                  .options(cmdline_options)
                  .positional(positional)
                  .run(),
              vm);
    if (vm.count("config")) {
      std::ifstream config_file(vm["config"].as<std::string>());
      po::store(po::parse_config_file(config_file, config), vm);
    }
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << "Usage: hsip [options] file...\n\n";
      std::cout << visible << "\n";
      return 0;
    }

    if (vm.count("version")) {
      std::cout << "Version: alpha.\n";
      return 0;
    }

    std::vector<std::string> input_files;
    if (vm.count("input-file")) {
      input_files = vm["input-file"].as<decltype(input_files)>();
      for (auto &&each : input_files) {
        std::cout << each << "\n";
        //hsp::read(each);
        using namespace hsp;
        auto decoder = std::make_shared<HgyNIRDecoder>();
        LineScanData raw_data(each, decoder);
        raw_data.set_frame_aux_size(1024).set_word_length(12).set_compressed(false).set_leading_bytes("NAIS");
        raw_data.traverse();
      }
    } else {
      throw std::invalid_argument("no input files");
    }
  } catch (const std::runtime_error &e) {
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return -1;
  }
  return 0;
}
