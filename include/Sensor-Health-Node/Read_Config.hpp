#pragma once
#include <iostream>
#include <filesystem>
#include <list>
#include <string>
#include <utility>

//yaml
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

//std::string non è un tipo letterale in C++17 (ha un distruttore non banale),
// quindi constexpr std::string dà errore di compilazione. quindi metto inline
inline const std::string CONFIG_DIR = "./src/Sensor-Health-Node/config";


// Associa il nome del file (es. "cat.yaml") al relativo albero YAML
struct ConfigFile {
  std::string filename{};
  YAML::Node node{};
};

class Read_Config {
public:
  std::list<ConfigFile> read_all_yaml_files(const std::string& directory_path);
};