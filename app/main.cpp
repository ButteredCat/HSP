#include <gdal.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <exception>
#include <iostream>
#include <string>

namespace po = boost::program_options;

namespace fs = boost::filesystem;

void time_consuming() {
  const int loop = 100;
  double a{1.0}, b{2.0};
  for (int i = 0; i < loop; ++i) {
    for (int j = 0; j < loop; ++j) {
      for (int k = 0; k < loop; ++k) {
        a *= b;
      }
    }
  }
}

void do_computation(const fs::path& input, const fs::path& output,
                    const fs::path& workdir) {
  time_consuming();
  auto filename = input.filename();
  std::vector<fs::path> source{filename,
                               fs::path(filename).replace_extension("hdr"),
                               fs::path(filename).replace_extension("jpg")};
  std::vector<fs::path> dest{output, fs::path(output).replace_extension("hdr"),
                             fs::path(output).replace_extension("jpg")};
  try {
    for (auto i = 0; i < source.size(); ++i) {
      fs::copy(workdir / source[i], dest[i],
               fs::copy_options::overwrite_existing);
    }
  } catch (...) {
  }
}

int main(int argc, char* argv[]) {
  try {
    po::options_description config("Configuration");
    config.add_options()("input,i", po::value<std::string>(), "input image")(
        "output,o", po::value<std::string>(), "output image")(
        "workdir,w", po::value<std::string>(), "workdir")(
        "task", po::value<std::string>(), "task xml");
    po::options_description cmdline_options;
    cmdline_options.add(config);

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);

    std::string input = vm["input"].as<std::string>();
    std::string output = vm["output"].as<std::string>();
    std::string workdir = vm["workdir"].as<std::string>();

    do_computation(input, output, workdir);
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return -1;
  }
  return 0;
}
