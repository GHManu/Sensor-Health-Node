#include "rclcpp/rclcpp.hpp"

class Sensor_Sim : public rclcpp::Node {
public:
  Sensor_Sim() : Node("sensor_simulator") {
    RCLCPP_INFO(this->get_logger(), "Simulatore sensore avviato");
  }
private:
protected:
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Sensor_Sim>());
  rclcpp::shutdown();
  return 0;
}