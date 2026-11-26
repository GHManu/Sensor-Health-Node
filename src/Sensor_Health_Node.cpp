#include "Sensor-Health-Node/Sensor_Health_Node.hpp"


using std::placeholders::_1;
using namespace std::chrono_literals;


class Sensor_Health_Node : public rclcpp::Node{
public:
  Sensor_Health_Node() : Node("Sensor_Health_Node") {
    RCLCPP_INFO(this->get_logger(), "WatchDog avviato pronto a controllare");

    configs = read_config.read_all_yaml_files(CONFIG_DIR);
    set_topics();

    subscribers.clear();

    

    // Inizializzo i timestamp con il clock corrente del nodo per evitare mismatch di clock
    const auto now = this->get_clock()->now();

    sensors_["imu"] = SensorStatus{"imu", "imu", now, false};
    sensors_["laser_scan"] = SensorStatus{"laser_scan", "laser_scan", now, false};
    sensors_["point_cloud"] = SensorStatus{"point_cloud", "point_cloud", now, false};

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


  // Imposta i topic dei sensori dai file di configurazione YAML
  void set_topics(){
    for (const auto& cfg : configs) {
      const auto& node = cfg.node;
      if (node.IsNull() || !node["fast_limo"] || !node["fast_limo"]["ros__parameters"] ||
          !node["fast_limo"]["ros__parameters"]["topics"] ||
          !node["fast_limo"]["ros__parameters"]["topics"]["input"] ||
          !node["fast_limo"]["ros__parameters"]["topics"]["input"]["lidar"] ||
          !node["fast_limo"]["ros__parameters"]["topics"]["input"]["imu"]) {
        RCLCPP_ERROR(this->get_logger(), "Errore nella configurazione dei topic nel file %s", cfg.filename.c_str());
        continue;
      }

      std::string lidar_topic = node["fast_limo"]["ros__parameters"]["topics"]["input"]["lidar"].as<std::string>();
      std::string imu_topic = node["fast_limo"]["ros__parameters"]["topics"]["input"]["imu"].as<std::string>();

      if (cfg.filename == "cat.yaml") {
        cat_lidar_topic = lidar_topic;
        cat_imu_topic = imu_topic;
      }
      //...
    }
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

  // Oggetti per la lettura dei file di configurazione YAML
  Read_Config read_config;
  std::list<ConfigFile> configs;

  //lista di subscribers per i topic dei sensori
  std::list<rclcpp::SubscriptionBase::SharedPtr> subscribers;

  std::string cat_lidar_topic;
  std::string cat_imu_topic;
  
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