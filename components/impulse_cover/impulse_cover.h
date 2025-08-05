#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/output/binary_output.h"
#include <vector>

namespace esphome {
#ifdef USE_BINARY_SENSOR
namespace binary_sensor {
class BinarySensor;
}
#endif
namespace impulse_cover {

// Forward declarations for trigger classes
class OnOpenTrigger;
class OnCloseTrigger;
class OnIdleTrigger;
class SafetyTrigger;

enum class ImpulseCoverOperation {
  IDLE = 0,
  OPENING = 1,
  CLOSING = 2,
};

class ImpulseCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  
  // Configuration setters
  void set_open_duration(uint32_t duration) { this->open_duration_ = duration; }
  void set_close_duration(uint32_t duration) { this->close_duration_ = duration; }
  void set_pulse_delay(uint32_t delay) { this->pulse_delay_ = delay; }
  void set_safety_timeout(uint32_t timeout) { this->safety_timeout_ = timeout; }
  void set_safety_max_cycles(uint8_t cycles) { this->safety_max_cycles_ = cycles; }
  
  // Safety control
  void reset_safety_mode() { this->safety_triggered_ = false; this->safety_cycle_count_ = 0; }
  bool is_safety_triggered() const { return this->safety_triggered_; }
  
  void set_output(output::BinaryOutput *output) { this->output_ = output; }
#ifdef USE_BINARY_SENSOR
  void set_open_sensor(binary_sensor::BinarySensor *sensor);
  void set_close_sensor(binary_sensor::BinarySensor *sensor);
  void set_open_sensor_inverted(bool inverted) { this->open_sensor_inverted_ = inverted; }
  void set_close_sensor_inverted(bool inverted) { this->close_sensor_inverted_ = inverted; }
#endif
  
  // Override cover traits
  cover::CoverTraits get_traits() override;

 protected:
  void control(const cover::CoverCall &call) override;
  void start_direction(cover::CoverOperation dir);
  void stop_movement();
  
 private:
  void send_pulse();
  void update_position();
  void check_safety();
#ifdef USE_BINARY_SENSOR
  void handle_endstop();
  void update_position_from_sensors();
#endif
  bool is_at_target_position();
  
  // Configuration
  uint32_t open_duration_{15000};    // 15 seconds default
  uint32_t close_duration_{15000};   // 15 seconds default
  uint32_t pulse_delay_{500};        // 500ms between pulses
  uint32_t safety_timeout_{60000};   // 1 minute safety timeout
  uint8_t safety_max_cycles_{5};     // Max cycles before safety trigger
  
  // Hardware
  output::BinaryOutput *output_{nullptr};
#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *open_sensor_{nullptr};
  binary_sensor::BinarySensor *close_sensor_{nullptr};
  bool open_sensor_inverted_{false};
  bool close_sensor_inverted_{false};
#endif
  
  // State tracking
  ImpulseCoverOperation current_operation_{ImpulseCoverOperation::IDLE};
  ImpulseCoverOperation target_operation_{ImpulseCoverOperation::IDLE};
  uint32_t operation_start_time_{0};
  uint32_t last_pulse_time_{0};
  bool pending_reverse_{false};
  bool pulse_sent_{false};
  bool safety_triggered_{false};
  uint8_t safety_cycle_count_{0};
  uint32_t last_direction_change_{0};
  
  // Position calculation
  float target_position_{0};
  float start_position_{0};
  bool has_initial_state_{false};
  uint32_t last_position_update_{0};
  
  // Public accessors for triggers
 public:
  ImpulseCoverOperation get_current_operation() const { return current_operation_; }
  
  // Automation triggers  
  void add_on_open_trigger(Trigger<> *trigger);
  void add_on_close_trigger(Trigger<> *trigger);
  void add_on_idle_trigger(Trigger<> *trigger);
  void add_on_safety_trigger(SafetyTrigger *trigger);

 protected:
  // Helper methods for firing triggers
  void fire_on_open_triggers_();
  void fire_on_close_triggers_();
  void fire_on_idle_triggers_();
  void fire_on_safety_triggers_();

  // Trigger lists
  std::vector<Trigger<> *> on_open_triggers_;
  std::vector<Trigger<> *> on_close_triggers_;
  std::vector<Trigger<> *> on_idle_triggers_;
  std::vector<SafetyTrigger *> on_safety_triggers_;
};

// Specific trigger classes to avoid ID conflicts
class OnOpenTrigger : public Trigger<> {
 public:
  explicit OnOpenTrigger(ImpulseCover *parent) : parent_(parent) {}

 protected:
  ImpulseCover *parent_;
};

class OnCloseTrigger : public Trigger<> {
 public:
  explicit OnCloseTrigger(ImpulseCover *parent) : parent_(parent) {}

 protected:
  ImpulseCover *parent_;
};

class OnIdleTrigger : public Trigger<> {
 public:
  explicit OnIdleTrigger(ImpulseCover *parent) : parent_(parent) {}

 protected:
  ImpulseCover *parent_;
};

class SafetyTrigger : public Trigger<> {
 public:
  explicit SafetyTrigger(ImpulseCover *parent) : parent_(parent) {}

 protected:
  ImpulseCover *parent_;
};

// Action classes
template<typename... Ts> class ResetSafetyAction : public Action<Ts...> {
 public:
  explicit ResetSafetyAction(ImpulseCover *cover) : cover_(cover) {}

  void play(Ts... x) override { this->cover_->reset_safety_mode(); }

 protected:
  ImpulseCover *cover_;
};

}  // namespace impulse_cover
}  // namespace esphome