#include "rclcpp/rclcpp.hpp"

class Sensor_Health_Node : public rclcpp::Node{
public:
  Sensor_Health_Node() : Node("Sensor_Health_Node") {
    RCLCPP_INFO(this->get_logger(), "WatchDog avviato pronto a controllare");
  }
private:
protected:
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Sensor_Health_Node>());
  rclcpp::shutdown();
  return 0;
}