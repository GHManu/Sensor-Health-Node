# Sensor-Health-Node
A system for ROS 2 Humble, designed for a Formula Student autonomous electric vehicle team of UNIMORE university, that monitors whether sensors are still publishing information.

---> **Documentation**: [Doc](https://docs.ros.org/en/humble/Tutorials.html)


# Idea
## WatchDog
The idea is to implement a "watchdog", a safety mechanism that detects system freezes, infinite loops, or malfunctions in a computing system and intervenes to restore it, typically by triggering a restart.

How it works:
   - Countdown: A timer starts from a preset value and counts down toward zero.

   - Periodic reset ("kicking" or "petting"): As long as the program or system is functioning correctly, the software sends a signal at regular intervals to reset the timer.
   
   - Intervention in the event of a freeze: If the system freezes or enters a deadlock and fails to update the timer in time, the countdown expires, triggering a predefined action (such as a hardware or software restart, transitioning to a safe state, or saving diagnostic data).

## General Architecture
**How it works**:
- the health node subscribes to the measurement topic;
- upon receiving each message, it updates the "timestamp of the last message";
- a periodic timer checks whether the difference between the current time and the timestamp of the last message exceeds a specific threshold (timeout);
- if it does, the sensor is considered faulty, and an action is triggered.

> The timeout should be selected based on the publication frequency (e.g., for a sensor operating at 10 Hz, a timeout of 0.5 seconds,allowing for a few missed messages, helps avoid false alarms).

**Watchdog actions**:
   - logging a warning or error;
   - publishing a health status to a dedicated topic (e.g., `/sensor_health`), allowing other nodes to respond;
   - invoking a ROS2 service or action (e.g., stopping the robot or restarting the node);
   - handling recovery: if the sensor resumes publishing, the health node should detect this and signal that the system has recovered. (THIS SHOULD BE IMPLEMENTED)

**Two useful concepts**:
- QoS (Quality of Service): publishers and subscribers must have compatible profiles. For sensor data, "best effort" is frequently used; however, if the profiles are incompatible, no data will be received.
- Deadline QoS: ROS2 includes a built-in mechanism (QoS deadline) that signals when a message fails to arrive within a specified timeframe. While this serves as an alternative to a manual timer, using a watchdog with a timer is often clearer and more flexible for demonstration purposes.