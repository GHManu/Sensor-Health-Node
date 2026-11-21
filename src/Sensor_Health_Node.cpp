#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"  //tipi: Header(timestamp e id), String, Int32

//messaggi per sensori
/* https://docs.ros.org/en/noetic/api/sensor_msgs/html/index-msg.html */
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"



using std::placeholders::_1;

class Sensor_Health_Node : public rclcpp::Node{
public:
  Sensor_Health_Node() : Node("Sensor_Health_Node") {
    RCLCPP_INFO(this->get_logger(), "WatchDog avviato pronto a controllare");
    //il nodo può iscriversi a più topic


  }
private:
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_subscriptorer;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_scan_subscriptorer;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr point_cloud_subscriptorer;
  
protected:
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Sensor_Health_Node>());
  rclcpp::shutdown();
  return 0;
}