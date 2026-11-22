#include <array>
#include <memory>
#include <chrono>

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


class Sensor_Health_Node : public rclcpp::Node{
public:
  Sensor_Health_Node() : Node("Sensor_Health_Node") {
    RCLCPP_INFO(this->get_logger(), "WatchDog avviato pronto a controllare");

    // Inizializzo i timestamp con il clock corrente del nodo per evitare mismatch di clock
    const auto now = this->get_clock()->now();
    imu_last_msg_time = now;
    laser_scan_last_msg_time = now;
    point_cloud_last_msg_time = now;

    //il nodo può iscriversi a più topic
    imu_subscriber = this->create_subscription<sensor_msgs::msg::Imu>(
      "/imu/data", 10, std::bind(&Sensor_Health_Node::topic_callback_imu, this, _1));

    laser_scan_subscriber = this->create_subscription<sensor_msgs::msg::LaserScan>(
      "/lidar/scan", 10, std::bind(&Sensor_Health_Node::topic_callback_laser_scan, this, _1));
      
    point_cloud_subscriber = this->create_subscription<sensor_msgs::msg::PointCloud2>(
      "/lidar/point_cloud", 10, std::bind(&Sensor_Health_Node::topic_callback_point_cloud, this, _1));

    // devo creare una funzione che mi controlli il tempo dall'ultimo messaggio 
    // ricevuto a quello che riceverò dopo, così da poter capire se il sensore è attivo o meno, 
    // se non ricevo messaggi per un certo periodo di tempo, stampo un warning 
    // quindi devo salvare il timestamp dell'ultimo messaggio
    //  ricevuto, e poi controllo se il tempo trascorso è maggiore di un certo valore, 
    // se lo è stampo un warning

    // Esegue watchdog_timer() periodicamente ogni 200 ms ovvero 0.2 secondi, 
    // quindi controlla se è passato più di 1 secondo dall'ultimo messaggio ricevuto
    timer_ = this->create_wall_timer(
      200ms, std::bind(&Sensor_Health_Node::watchdog_timer, this));
  }

private:

  void topic_callback_imu(const sensor_msgs::msg::Imu::SharedPtr msg) {
    (void)msg;  //è un cast per evitare warning di variabile non usata, ma non è necessario
    RCLCPP_INFO(this->get_logger(), "Ricevuto messaggio IMU");
    imu_last_msg_time = this->get_clock()->now();
    imu_warned = false;
  }

  void topic_callback_laser_scan(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
    (void)msg;
    RCLCPP_INFO(this->get_logger(), "Ricevuto messaggio LaserScan");
    laser_scan_last_msg_time = this->get_clock()->now();
    laser_scan_warned = false;
  }

  void topic_callback_point_cloud(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    (void)msg;
    RCLCPP_INFO(this->get_logger(), "Ricevuto messaggio PointCloud2");
    point_cloud_last_msg_time = this->get_clock()->now();
    point_cloud_warned = false;
  }

  void watchdog_timer() {
    const auto now = this->get_clock()->now();

    if ((now - imu_last_msg_time).seconds() > TIMEOUT_THRESHOLD && !imu_warned) {
      RCLCPP_WARN(this->get_logger(), "Nessun messaggio ricevuto da IMU");
      imu_warned = true;
    }
    if ((now - laser_scan_last_msg_time).seconds() > TIMEOUT_THRESHOLD && !laser_scan_warned) {
      RCLCPP_WARN(this->get_logger(), "Nessun messaggio ricevuto da LaserScan");
      laser_scan_warned = true;
    }
    if ((now - point_cloud_last_msg_time).seconds() > TIMEOUT_THRESHOLD && !point_cloud_warned) {
      RCLCPP_WARN(this->get_logger(), "Nessun messaggio ricevuto da PointCloud2");
      point_cloud_warned = true;
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_subscriber;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_scan_subscriber;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr point_cloud_subscriber;
  rclcpp::TimerBase::SharedPtr timer_;

  rclcpp::Time imu_last_msg_time;
  rclcpp::Time laser_scan_last_msg_time;
  rclcpp::Time point_cloud_last_msg_time;

  bool imu_warned = false;
  bool laser_scan_warned = false;
  bool point_cloud_warned = false;

protected:
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Sensor_Health_Node>());
  rclcpp::shutdown();
  return 0;
}