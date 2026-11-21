#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"  //tipi: Header(timestamp e id), String, Int32

//messaggi per sensori
/* https://docs.ros.org/en/noetic/api/sensor_msgs/html/index-msg.html */
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"


using namespace std::chrono_literals;

class Sensor_Sim : public rclcpp::Node {
public:
  Sensor_Sim() : Node("sensor_simulator") {
    RCLCPP_INFO(this->get_logger(), "Simulatore sensore avviato");

    //string_publisher = this->create_publisher<std_msgs::msg::String>("topic", 10);
    //inizializzo e creo i publisher, i topic vengono creati in automatico, e come tipo hanno il messaggio
    //ogni topic ha un tipo di messaggio, non posso pubblicare messaggi diversi in uno stesso topic, ognuno
    //ha un topic
    imu_publisher = this->create_publisher<sensor_msgs::msg::Imu>("/imu/data", 10);
    laser_scan_publisher = this->create_publisher<sensor_msgs::msg::LaserScan>("/lidar/scan", 10);
    point_cloud_publisher = this->create_publisher<sensor_msgs::msg::PointCloud2>("/lidar/point_cloud", 10);
    timer = this->create_wall_timer(500ms, std::bind(&Sensor_Sim::timer_callback, this)); //0.5s
  }

private:
  void timer_callback() {
    auto now = this->get_clock()->now();

    auto imu_msg = sensor_msgs::msg::Imu();
    imu_msg.header.stamp = now;
    imu_msg.header.frame_id = "imu_link";
    imu_msg.orientation.w = 1.0;

    auto laser_scan_msg = sensor_msgs::msg::LaserScan();
    laser_scan_msg.header.stamp = now;
    laser_scan_msg.header.frame_id = "laser_link";
    laser_scan_msg.angle_min = -1.57f;
    laser_scan_msg.angle_max = 1.57f;
    laser_scan_msg.angle_increment = 0.01f;
    laser_scan_msg.range_min = 0.1f;
    laser_scan_msg.range_max = 10.0f;

    auto point_cloud_msg = sensor_msgs::msg::PointCloud2();
    point_cloud_msg.header.stamp = now;
    point_cloud_msg.header.frame_id = "lidar_link";
    point_cloud_msg.height = 1;
    point_cloud_msg.is_dense = true;

    std::string total_msg = imu_msg.header.frame_id + ", " +
                            laser_scan_msg.header.frame_id + ", " +
                            point_cloud_msg.header.frame_id;
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", total_msg.c_str());

    imu_publisher->publish(imu_msg);
    laser_scan_publisher->publish(laser_scan_msg);
    point_cloud_publisher->publish(point_cloud_msg);
  }
  //dichiaro le variabili
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_publisher;
  rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr laser_scan_publisher;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr point_cloud_publisher;
  rclcpp::TimerBase::SharedPtr timer;

protected:
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Sensor_Sim>());
  rclcpp::shutdown();
  return 0;
}