# How to Set Up
## General Info
* Operating System used: Fedora
* Container System used: Podman (built-in in Fedora)

---

## Installation, Set Up and Test

### Step 1: Allow the container to use the screen (for RViz and Gazebo)

Since Fedora uses Wayland/X11, you need to authorize the display server so graphical windows can open from the container:

```bash
xhost +local:root

```

### Step 2: Create the container

```bash
podman run -it \
  --name container_ros \
  --net=host \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  -v ~/ros2_ws/src:/root/ros2_ws/src:Z \
  --security-opt label=disable \
  docker.io/osrf/ros:humble-desktop bash

```
* **IMPORTANT**: -v ~/ros2_ws/src:/root/ros2_ws/src:Z: links your local src folder inside /root/ros2_ws/src in the container right from the very first second. This way, you can put your GitHub repo inside the local folder and it will already be synchronized, if you make changes in Podman, they update locally as well and vice versa, and then you can push from your local machine.

* To automatically remove the container upon exit, simply add `--rm` right after `-it` at the beginning.
* --name container_ros: gli assegna subito un nome fisso, così non devi usare podman rename dopo.


* `osrf/ros:humble-desktop` downloads the official environment based on **Ubuntu 22.04**.
* `--volume="$HOME/ros2_ws:/root/ros2_ws:z"` creates a folder named `ros2_ws` in your Fedora Home directory and links it inside the Ubuntu container (the `:z` flag is needed for Fedora/SELinux to prevent permission issues).

### Step 3: Enable ROS

```bash
source /opt/ros/humble/setup.bash

```

### To Rename the container

```bash
podman rename <OLD_NAME_OR_ID> <NEW_NAME>

```

#### Quick test to check if it works:

1. Enter this to start the test node (Talker):

```bash
ros2 run demo_nodes_cpp talker

```

2. Open **another terminal on your Fedora host** (outside the container) and connect to the same running container by typing:

```bash
podman ps

```

Copy the container ID (or name) and attach to it:

```bash
podman exec -it <CONTAINER_ID_OR_NAME> bash

```

3. In the new terminal, source ROS 2 and start the Listener:

```bash
source /opt/ros/humble/setup.bash
ros2 run demo_nodes_py listener

```

* To **detach** from the container without stopping it, press `Ctrl + P` followed by `Ctrl + Q`.
* To **re-enter** it whenever you need:

```bash
podman start -ai <CONTAINER_ID_OR_NAME>

```

---

## How to resume in the future

* In a regular terminal as soon as you open it:

```bash
xhost +local:root

```

* Then run:

```bash
podman ps

```

* Then run:

```bash
podman start -ai <CONTAINER_ID_OR_NAME>

```

* Then run:

```bash
source /opt/ros/humble/setup.bash

```

---


## How To Build

If you don't have anything inside `src` yet and want to set up a **C++** package in ROS 2 Humble, you just need to create a package with the `ament_cmake` build type inside `/root/ros2_ws/src` and then build it from the workspace root.

1. **Enter `src` and create the C++ package:** 1 min.
Navigate to `/root/ros2_ws/src` and generate the basic structure of a C++ package with common dependencies (`rclcpp` for C++ nodes and `std_msgs` for standard messages):

```bash
cd /root/ros2_ws/src
ros2 pkg create --build-type ament_cmake <REPO_NAME> --dependencies rclcpp std_msgs

```

*Verification:* By running `ls -la /root/ros2_ws/src/<REPO_NAME>`, you should see `CMakeLists.txt`, `package.xml`, the `include/` folder, and the `src/` folder.

2. **Add a minimal C++ source file and update CMakeLists.txt:** 2 min.
Create a simple C++ node in `src/test_node.cpp`:

```bash
cat << 'EOF' > /root/ros2_ws/src/<REPO_NAME>/src/test_node.cpp
#include <chrono>
#include <memory>
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class TestNode : public rclcpp::Node {
public:
  TestNode() : Node("test_node") {
    timer_ = this->create_wall_timer(
        1000ms, [this]() { RCLCPP_INFO(this->get_logger(), "Hello from C++ in ROS 2!"); });
  }
private:
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TestNode>());
  rclcpp::shutdown();
  return 0;
}
EOF

```

Then add the executable in `/root/ros2_ws/src/<REPO_NAME>/CMakeLists.txt` right before the `ament_package()` line:

```cmake
add_executable(test_node src/test_node.cpp)
ament_target_dependencies(test_node rclcpp std_msgs)

install(TARGETS
  test_node
  DESTINATION lib/${PROJECT_NAME})

```

*Verification:* Check with `tail -n 20 /root/ros2_ws/src/my_cpp_node/CMakeLists.txt` that the `add_executable` and `install(TARGETS ...)` block appears before `ament_package()`.

3. **Build with `colcon build` and source the workspace:** 1 min.
Return to the workspace root, build, and activate your workspace environment:

```bash
cd /root/ros2_ws
colcon build
source /root/ros2_ws/install/setup.bash

```

*Verification:* `colcon build` should finish with `Summary: 1 package finished` (with no CMake or `rclcpp` errors).

4. **Run the C++ node to test:**
Launch the newly compiled executable:

```bash
ros2 run my_cpp_node test_node

```

*Verification:* The log `Hello from C++ in ROS 2!` should appear in the terminal every second. Press `Ctrl+C` to stop it.

### What to do in the future when adding new nodes

* **If the new node is in the same package (`my_cpp_node`):**

1. Create the new `.cpp` file inside `src/` (and any `.hpp` header files in `include/my_cpp_node/`).
2. Add a new `add_executable(new_node_name src/new_file.cpp)` block, `ament_target_dependencies(...)`, and add `new_node_name` inside `install(TARGETS ...)` in `CMakeLists.txt`.
3. If the new node uses new libraries or message types (e.g., `sensor_msgs`), add them to `CMakeLists.txt` (`find_package(...)` and `ament_target_dependencies(...)`) and to `package.xml` (`<depend>...</depend>`).
4. Run `colcon build --symlink-install` again from `/root/ros2_ws` (you don't need to re-run `source /root/ros2_ws/install/setup.bash` if it is already sourced in the same terminal, though doing so doesn't hurt).

* **If you want to create a separate package for another group of nodes:**

1. Go to `/root/ros2_ws/src` and run `ros2 pkg create --build-type ament_cmake another_package --dependencies ...`.
2. Repeat the steps for writing the node, configuring `CMakeLists.txt`, and running `colcon build` from `/root/ros2_ws`.