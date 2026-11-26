#include "Sensor-Health-Node/Sensor_Health_Node.hpp"


using std::placeholders::_1;
using namespace std::chrono_literals;


class Sensor_Health_Node : public rclcpp::Node{
public:
  Sensor_Health_Node() : Node("Sensor_Health_Node") {
    RCLCPP_INFO(this->get_logger(), "WatchDog avviato pronto a controllare");

    configs = read_config.read_all_yaml_files(CONFIG_DIR);
    set_topics();

    // Esegue watchdog_timer() periodicamente ogni 200 ms ovvero 0.2 secondi, 
    // quindi controlla se è passato più di 1 secondo dall'ultimo messaggio ricevuto
    timer_ = this->create_wall_timer(
      200ms, std::bind(&Sensor_Health_Node::watchdog_timer, this));
  }

private:

  // Restituisce il tipo di messaggio ROS 2 associato al modello di LiDAR (0: OUSTER, 1: VELODYNE, 2: HESAI, 3: LIVOX)
  std::string lidar_msg_type_from_sensor_type(int sensor_type) {
    switch (sensor_type) {
      case 0: // OUSTER
      case 1: // VELODYNE
      case 2: // HESAI
      case 3: // LIVOX
        return "sensor_msgs/msg/PointCloud2";
      default:
        return "";
    }
  }

  // Funzione chiamata quando viene ricevuto un messaggio da un sensore
  void on_sensor_msg(const std::string & key) {
    auto & s = sensors_map.at(key);
    RCLCPP_INFO(this->get_logger(), "Ricevuto messaggio da %s", s.name.c_str());
    s.last_msg_time = this->get_clock()->now();
    s.warned = false;
  }


  // Imposta i topic dei sensori dai file di configurazione YAML
  // il tipo di messaggio del topic lidar viene determinato in base al tipo di sensore specificato nel file di configurazione
  // il sensor type è un intero che rappresenta il tipo di sensore LiDAR usato, ovvero il tipo di driver del sensore LiDAR
  void set_topics() {
    // Inizializzo i timestamp con il clock corrente del nodo per evitare mismatch di clock
    const auto now = this->get_clock()->now();
    subscribers.clear();
    sensors_map.clear();

    for (const auto & cfg : configs) {
      const auto & node = cfg.node;
      if (node.IsNull() || !node["fast_limo"] || !node["fast_limo"]["ros__parameters"] ||
          !node["fast_limo"]["ros__parameters"]["topics"] ||
          !node["fast_limo"]["ros__parameters"]["topics"]["input"] ||
          !node["fast_limo"]["ros__parameters"]["topics"]["input"]["lidar"] ||
          !node["fast_limo"]["ros__parameters"]["topics"]["input"]["imu"]) {
        RCLCPP_ERROR(
          this->get_logger(),
          "Errore nella configurazione dei topic nel file %s",
          cfg.filename.c_str());
        continue;
      }

      const auto params = node["fast_limo"]["ros__parameters"];
      const std::string lidar_topic = params["topics"]["input"]["lidar"].as<std::string>();
      const std::string imu_topic = params["topics"]["input"]["imu"].as<std::string>();
      const int type = params["sensor_type"] ? params["sensor_type"].as<int>() : 0;

      const std::string lidar_key = cfg.filename + "_lidar";
      const std::string imu_key = cfg.filename + "_imu";

      sensors_map[lidar_key] = SensorStatus{lidar_key, lidar_topic, now, false};
      sensors_map[imu_key] = SensorStatus{imu_key, imu_topic, now, false};

      //il nodo può iscriversi a più topic leggendoli dinamicamente dagli YAML
      const std::string lidar_msg_type = lidar_msg_type_from_sensor_type(type);

      if (!lidar_topic.empty() && !lidar_msg_type.empty()) {
        /*
        Parameters:
          node_base – Pointer to parent node’s NodeBaseInterface
          ts_lib – Type support library, needs to correspond to topic_type
          topic_name – Topic name
          topic_type – Topic type
          qos – QoS(Quality if Service) settings for communication with the publisher
          callback – Callback for new messages of serialized form
          options – Subscription options. Not all subscription options are currently respected, 
          the only relevant options for this subscription are event_callbacks, use_default_callbacks, ignore_local_publications, and callback_group.
        */
        auto lidar_sub = this->create_generic_subscription(
          lidar_topic,
          lidar_msg_type,
          rclcpp::QoS(10),
          [this, lidar_key](std::shared_ptr<rclcpp::SerializedMessage> /*msg*/) {
            this->on_sensor_msg(lidar_key);
          });
        subscribers.push_back(lidar_sub);
      }

      if (!imu_topic.empty()) {

        auto imu_sub = this->create_generic_subscription(
          imu_topic,
          "sensor_msgs/msg/Imu",
          rclcpp::QoS(10),
          [this, imu_key](std::shared_ptr<rclcpp::SerializedMessage> /*msg*/) {
            this->on_sensor_msg(imu_key);
          });
        subscribers.push_back(imu_sub);
      }
    }
  }

  void watchdog_timer() {
    const auto now = this->get_clock()->now();

    for (auto & pair : sensors_map) {
      auto & s = pair.second;
      if ((now - s.last_msg_time).seconds() > TIMEOUT_THRESHOLD && !s.warned) {
        RCLCPP_WARN(this->get_logger(), "Nessun messaggio ricevuto da %s", s.name.c_str());
        s.warned = true;
      }
    }
  }

  // Oggetti per la lettura dei file di configurazione YAML
  Read_Config read_config;
  std::list<ConfigFile> configs;

  //lista di subscribers per i topic dei sensori
  std::list<rclcpp::SubscriptionBase::SharedPtr> subscribers;
  std::unordered_map<std::string, SensorStatus> sensors_map; //mappa per memorizzare lo stato di ciascun sensore
  
  rclcpp::TimerBase::SharedPtr timer_;

protected:
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Sensor_Health_Node>());
  rclcpp::shutdown();
  return 0;
}