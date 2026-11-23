#include <array>
#include <chrono>
#include <string>
#include <unordered_map>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"  //tipi: Header(timestamp e id), String, Int32

//messaggi per sensori
/* https://docs.ros.org/en/noetic/api/sensor_msgs/html/index-msg.html */
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;

const float TIMEOUT_THRESHOLD = 1.0; //secondi


struct SensorStatus {
  std::string name;
  rclcpp::Time last_msg_time;
  bool warned;
};


class Sensor_Health_Node : public rclcpp::Node{
public:
  Sensor_Health_Node() : Node("Sensor_Health_Node") {
    RCLCPP_INFO(this->get_logger(), "WatchDog avviato pronto a controllare");

    // Inizializzo i timestamp con il clock corrente del nodo per evitare mismatch di clock
    const auto now = this->get_clock()->now();

    sensors_["imu"] = SensorStatus{"imu", now, false};
    sensors_["laser_scan"] = SensorStatus{"laser_scan", now, false};
    sensors_["point_cloud"] = SensorStatus{"point_cloud", now, false};

    //il nodo può iscriversi a più topic
    imu_subscriber = this->create_subscription<sensor_msgs::msg::Imu>(
      "/imu/data", 10, std::bind(&Sensor_Health_Node::topic_callback_imu, this, _1));

    laser_scan_subscriber = this->create_subscription<sensor_msgs::msg::LaserScan>(
      "/lidar/scan", 10, std::bind(&Sensor_Health_Node::topic_callback_laser_scan, this, _1));
      
    point_cloud_subscriber = this->create_subscription<sensor_msgs::msg::PointCloud2>(
      "/lidar/point_cloud", 10, std::bind(&Sensor_Health_Node::topic_callback_point_cloud, this, _1));

    // Esegue watchdog_timer() periodicamente ogni 200 ms ovvero 0.2 secondi, 
    // quindi controlla se è passato più di 1 secondo dall'ultimo messaggio ricevuto
    timer_ = this->create_wall_timer(
      200ms, std::bind(&Sensor_Health_Node::watchdog_timer, this));
  }

private:

  void topic_callback_imu(const sensor_msgs::msg::Imu::SharedPtr msg) {
    (void)msg;  //è un cast per evitare warning di variabile non usata, ma non è necessario
    on_sensor_msg("imu");
  }

  void topic_callback_laser_scan(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
    (void)msg;
    on_sensor_msg("laser_scan");
  }

  void topic_callback_point_cloud(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    (void)msg;
    on_sensor_msg("point_cloud");
  }

  void on_sensor_msg(const std::string & key) {
    auto & s = sensors_.at(key);
    RCLCPP_INFO(this->get_logger(), "Ricevuto messaggio da %s", s.name.c_str());
    s.last_msg_time = this->get_clock()->now();
    s.warned = false;
  }

  void watchdog_timer() {
    const auto now = this->get_clock()->now();

    for (auto & pair : sensors_) {
      auto & s = pair.second;
      if ((now - s.last_msg_time).seconds() > TIMEOUT_THRESHOLD && !s.warned) {
        RCLCPP_WARN(this->get_logger(), "Nessun messaggio ricevuto da %s", s.name.c_str());
        s.warned = true;
      }
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_subscriber;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_scan_subscriber;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr point_cloud_subscriber;
  rclcpp::TimerBase::SharedPtr timer_;

  std::unordered_map<std::string, SensorStatus> sensors_; //mappa per memorizzare lo stato di ciascun sensore

protected:
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Sensor_Health_Node>());
  rclcpp::shutdown();
  return 0;
}