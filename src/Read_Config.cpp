#include "Sensor-Health-Node/Read_Config.hpp"


std::list<ConfigFile> Read_Config::read_all_yaml_files(const std::string& directory_path) {
  fs::path dir_path(directory_path);
  std::list<ConfigFile> configs;

  if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
    std::cerr << "Percorso non valido o non e' una directory.\n";
    return configs;
  }

  for (const auto& entry : fs::directory_iterator(dir_path)) {
    if (entry.is_directory()) {
      std::cout << " [DIR]";
    } else if (entry.is_regular_file() && (entry.path().extension() == ".yaml" || entry.path().extension() == ".yml")) {
      YAML::Node config = YAML::LoadFile(entry.path().string());
      configs.push_back(ConfigFile{entry.path().filename().string(), config});
    }
  }
  return configs;
}