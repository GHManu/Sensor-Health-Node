# Sensor-Health-Node
A system for ROS 2 Humble, designed for a Formula Student autonomous electric vehicle team of UNIMORE university, that monitors whether sensors are still publishing information.


# Idea
The idea is to implement a "watchdog", a safety mechanism that detects system freezes, infinite loops, or malfunctions in a computing system and intervenes to restore it, typically by triggering a restart.

How it works:
   - Countdown: A timer starts from a preset value and counts down toward zero.

   - Periodic reset ("kicking" or "petting"): As long as the program or system is functioning correctly, the software sends a signal at regular intervals to reset the timer.
   
   - Intervention in the event of a freeze: If the system freezes or enters a deadlock and fails to update the timer in time, the countdown expires, triggering a predefined action (such as a hardware or software restart, transitioning to a safe state, or saving diagnostic data).