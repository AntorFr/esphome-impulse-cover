#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/output/binary_output.h"

namespace esphome {
namespace impulse_cover {

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
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_open_duration(uint32_t duration) { this->open_duration_ = duration; }
  void set_close_duration(uint32_t duration) { this->close_duration_ = duration; }
  void set_pulse_delay(uint32_t delay) { this->pulse_delay_ = delay; }
  void set_safety_timeout(uint32_t timeout) { this->safety_timeout_ = timeout; }
  void set_safety_max_cycles(uint8_t cycles) { this->safety_max_cycles_ = cycles; }
  
  void set_output(output::BinaryOutput *output) { this->output_ = output; }
  void set_open_sensor(binary_sensor::BinarySensor *sensor);
  void set_close_sensor(binary_sensor::BinarySensor *sensor);
  
  // Override cover traits
  cover::CoverTraits get_traits() override;

 protected:
  void control(const cover::CoverCall &call) override;
  void start_direction(cover::CoverOperation dir);
  void stop_movement();
  void send_pulse();
  void update_position();
  void check_safety();
  void handle_endstop();
  bool is_at_target_position();
  
  // Configuration
  uint32_t open_duration_{15000};    // 15 seconds default
  uint32_t close_duration_{15000};   // 15 seconds default
  uint32_t pulse_delay_{500};        // 500ms between pulses
  uint32_t safety_timeout_{60000};   // 1 minute safety timeout
  uint8_t safety_max_cycles_{5};     // Max cycles before safety trigger
  
  // Hardware
  output::BinaryOutput *output_{nullptr};
  binary_sensor::BinarySensor *open_sensor_{nullptr};
  binary_sensor::BinarySensor *close_sensor_{nullptr};
  
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
  bool has_initial_state_{false};
  uint32_t last_position_update_{0};
};

// Automation triggers
class ImpulseCoverOpenTrigger : public Trigger<> {
 public:
  explicit ImpulseCoverOpenTrigger(ImpulseCover *parent) {
    parent->add_on_state_callback([this, parent]() {
      if (parent->current_operation == cover::COVER_OPERATION_OPENING) {
        this->trigger();
      }
    });
  }
};

class ImpulseCoverCloseTrigger : public Trigger<> {
 public:
  explicit ImpulseCoverCloseTrigger(ImpulseCover *parent) {
    parent->add_on_state_callback([this, parent]() {
      if (parent->current_operation == cover::COVER_OPERATION_CLOSING) {
        this->trigger();
      }
    });
  }
};

class ImpulseCoverIdleTrigger : public Trigger<> {
 public:
  explicit ImpulseCoverIdleTrigger(ImpulseCover *parent) {
    parent->add_on_state_callback([this, parent]() {
      if (parent->current_operation == cover::COVER_OPERATION_IDLE) {
        this->trigger();
      }
    });
  }
};

class ImpulseCoverSafetyTrigger : public Trigger<> {
 public:
  explicit ImpulseCoverSafetyTrigger(ImpulseCover *parent) : parent_(parent) {}
  
  void check_and_trigger() {
    if (this->parent_->safety_triggered_) {
      this->trigger();
    }
  }

 protected:
  ImpulseCover *parent_;
};

}  // namespace impulse_cover
}  // namespace esphome
