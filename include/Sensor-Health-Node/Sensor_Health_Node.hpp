#pragma once
//std
#include <chrono>
#include <string>
#include <unordered_map>
#include <list>
#include <iostream>

//msg
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"

//ros
#include "rclcpp/rclcpp.hpp"

//yaml
#include <yaml-cpp/yaml.h>
#include "Sensor-Health-Node/Read_Config.hpp"

constexpr float TIMEOUT_THRESHOLD = 1.0f; //secondi

// stato del sensore
struct SensorStatus {
  std::string name{};
  std::string type{};
  rclcpp::Time last_msg_time{0, 0, RCL_ROS_TIME};
  bool warned{false};
};