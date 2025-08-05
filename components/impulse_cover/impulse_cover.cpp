#include "impulse_cover.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#include <cmath>

namespace esphome {
namespace impulse_cover {

static const char *const TAG = "impulse_cover";

void ImpulseCover::setup() {
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
    this->has_initial_state_ = true;
  } else {
    this->position = 0.5f;  // Default to middle position if unknown
  }
  
  // Set initial state based on sensors if available
  if (this->open_sensor_ != nullptr && this->close_sensor_ != nullptr) {
    bool open_state = this->open_sensor_inverted_ ? !this->open_sensor_->state : this->open_sensor_->state;
    bool close_state = this->close_sensor_inverted_ ? !this->close_sensor_->state : this->close_sensor_->state;
    
    if (open_state) {
      this->position = 1.0f;  // COVER_OPEN
      this->current_operation_ = ImpulseCoverOperation::IDLE;
      this->has_initial_state_ = true;
    } else if (close_state) {
      this->position = 0.0f;  // COVER_CLOSED
      this->current_operation_ = ImpulseCoverOperation::IDLE;
      this->has_initial_state_ = true;
    }
  }
  
  ESP_LOGCONFIG(TAG, "Setting up Impulse Cover '%s'", this->name_.c_str());
}

void ImpulseCover::loop() {
  const uint32_t now = millis();
  
  // Check for safety conditions
  this->check_safety();
  
  if (this->safety_triggered_) {
    return;
  }
  
  // Handle pending reverse operation
  if (this->pending_reverse_ && (now - this->last_pulse_time_) >= this->pulse_delay_) {
    this->pending_reverse_ = false;
    this->send_pulse();  // Send second pulse for reverse
    return;
  }
  
  // Update position based on current operation
  if (this->current_operation_ != ImpulseCoverOperation::IDLE) {
    this->update_position();
    
    // Check if we've reached the target or an endstop
    this->handle_endstop();
    
    // Check if we've reached our time-based target
    if (this->is_at_target_position()) {
      this->stop_movement();
    }
  }
}

void ImpulseCover::dump_config() {
  ESP_LOGCONFIG(TAG, "Impulse Cover '%s'", this->name_.c_str());
  ESP_LOGCONFIG(TAG, "  Open Duration: %u ms", this->open_duration_);
  ESP_LOGCONFIG(TAG, "  Close Duration: %u ms", this->close_duration_);
  ESP_LOGCONFIG(TAG, "  Pulse Delay: %u ms", this->pulse_delay_);
  ESP_LOGCONFIG(TAG, "  Safety Timeout: %u ms", this->safety_timeout_);
  ESP_LOGCONFIG(TAG, "  Safety Max Cycles: %u", this->safety_max_cycles_);
  
  if (this->open_sensor_) {
    ESP_LOGCONFIG(TAG, "  Open Sensor: '%s'", this->open_sensor_->get_name().c_str());
    ESP_LOGCONFIG(TAG, "  Open Sensor Inverted: %s", this->open_sensor_inverted_ ? "YES" : "NO");
  }
  if (this->close_sensor_) {
    ESP_LOGCONFIG(TAG, "  Close Sensor: '%s'", this->close_sensor_->get_name().c_str());
    ESP_LOGCONFIG(TAG, "  Close Sensor Inverted: %s", this->close_sensor_inverted_ ? "YES" : "NO");
  }
}

cover::CoverTraits ImpulseCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_supports_position(true);
  traits.set_supports_stop(true);
  traits.set_is_assumed_state(this->open_sensor_ == nullptr && this->close_sensor_ == nullptr);
  return traits;
}

void ImpulseCover::control(const cover::CoverCall &call) {
  if (this->safety_triggered_) {
    ESP_LOGW(TAG, "Cover is in safety mode, ignoring command");
    return;
  }
  
  if (call.get_stop()) {
    this->stop_movement();
    return;
  }
  
  if (call.get_position().has_value()) {
    this->target_position_ = *call.get_position();
    
    if (this->current_operation_ == ImpulseCoverOperation::IDLE) {
      // Determine direction based on current and target position
      if (this->target_position_ > this->position) {
        this->start_direction(cover::COVER_OPERATION_OPENING);
      } else if (this->target_position_ < this->position) {
        this->start_direction(cover::COVER_OPERATION_CLOSING);
      }
    } else {
      // Cover is moving, check if we need to reverse
      bool should_reverse = false;
      if (this->current_operation_ == ImpulseCoverOperation::OPENING && this->target_position_ < this->position) {
        should_reverse = true;
      } else if (this->current_operation_ == ImpulseCoverOperation::CLOSING && this->target_position_ > this->position) {
        should_reverse = true;
      }
      
      if (should_reverse) {
        // Stop current movement and reverse
        this->send_pulse();  // First pulse to stop
        this->pending_reverse_ = true;
        this->last_pulse_time_ = millis();
        
        // Set target operation for after reverse
        this->target_operation_ = (this->current_operation_ == ImpulseCoverOperation::OPENING) 
                                 ? ImpulseCoverOperation::CLOSING 
                                 : ImpulseCoverOperation::OPENING;
      }
    }
    return;
  }
  
  // Handle open/close/toggle commands
  if (call.get_command().has_value()) {
    auto command = *call.get_command();
    
    if (this->current_operation_ == ImpulseCoverOperation::IDLE) {
      // Cover is idle - determine action based on current position and command
      switch (command) {
        case cover::COVER_COMMAND_OPEN:
          if (this->position < 1.0f) {  // COVER_OPEN
            this->target_position_ = 1.0f;  // COVER_OPEN
            this->start_direction(cover::COVER_OPERATION_OPENING);
          }
          break;
          
        case cover::COVER_COMMAND_CLOSE:
          if (this->position > 0.0f) {  // COVER_CLOSED
            this->target_position_ = 0.0f;  // COVER_CLOSED
            this->start_direction(cover::COVER_OPERATION_CLOSING);
          }
          break;
          
        case cover::COVER_COMMAND_TOGGLE:
          // Toggle logic: open if closed, close if open, stop if moving
          if (this->position <= 0.1f) {  // Closed
            this->target_position_ = 1.0f;  // COVER_OPEN
            this->start_direction(cover::COVER_OPERATION_OPENING);
          } else {  // Open or partially open
            this->target_position_ = 0.0f;  // COVER_CLOSED
            this->start_direction(cover::COVER_OPERATION_CLOSING);
          }
          break;
      }
    } else {
      // Cover is moving - single pulse will stop it
      this->send_pulse();
      this->stop_movement();
    }
  }
}

void ImpulseCover::start_direction(cover::CoverOperation dir) {
  ESP_LOGD(TAG, "Starting %s operation", dir == cover::COVER_OPERATION_OPENING ? "OPEN" : "CLOSE");
  
  this->current_operation_ = (dir == cover::COVER_OPERATION_OPENING) 
                           ? ImpulseCoverOperation::OPENING 
                           : ImpulseCoverOperation::CLOSING;
  this->operation_start_time_ = millis();
  this->last_position_update_ = millis();
  this->last_direction_change_ = millis();
  
  this->send_pulse();
  this->publish_state();
}

void ImpulseCover::stop_movement() {
  ESP_LOGD(TAG, "Stopping movement");
  
  this->current_operation_ = ImpulseCoverOperation::IDLE;
  this->target_operation_ = ImpulseCoverOperation::IDLE;
  this->pending_reverse_ = false;
  this->target_position_ = this->position;  // Set target to current position
  
  this->publish_state();
}

void ImpulseCover::send_pulse() {
  if (this->output_ != nullptr) {
    ESP_LOGD(TAG, "Sending pulse");
    this->output_->turn_on();
    this->set_timeout(100, [this]() {  // 100ms pulse
      this->output_->turn_off();
    });
    this->last_pulse_time_ = millis();
    this->pulse_sent_ = true;
  }
}

void ImpulseCover::update_position() {
  const uint32_t now = millis();
  const uint32_t elapsed = now - this->last_position_update_;
  
  if (elapsed < 100) {  // Update every 100ms
    return;
  }
  
  float position_change = 0.0f;
  if (this->current_operation_ == ImpulseCoverOperation::OPENING) {
    position_change = (float) elapsed / this->open_duration_;
  } else if (this->current_operation_ == ImpulseCoverOperation::CLOSING) {
    position_change = -(float) elapsed / this->close_duration_;
  }
  
  this->position = clamp(this->position + position_change, 0.0f, 1.0f);
  this->last_position_update_ = now;
  
  this->publish_state();
}

void ImpulseCover::check_safety() {
  const uint32_t now = millis();
  
  // Check for safety timeout
  if (this->current_operation_ != ImpulseCoverOperation::IDLE) {
    if ((now - this->operation_start_time_) > this->safety_timeout_) {
      ESP_LOGW(TAG, "Safety timeout triggered - stopping movement");
      this->safety_triggered_ = true;
      this->stop_movement();
      return;
    }
  }
  
  // Check for cycling (rapid direction changes)
  if (this->current_operation_ != ImpulseCoverOperation::IDLE && 
      (now - this->last_direction_change_) < 2000) {  // Less than 2 seconds since last direction change
    this->safety_cycle_count_++;
    if (this->safety_cycle_count_ >= this->safety_max_cycles_) {
      ESP_LOGW(TAG, "Safety cycling detected - stopping movement");
      this->safety_triggered_ = true;
      this->stop_movement();
      return;
    }
  }
  
  // Reset cycle count if enough time has passed
  if ((now - this->last_direction_change_) > 10000) {  // 10 seconds
    this->safety_cycle_count_ = 0;
  }
  
  // Reset safety trigger if cover has been idle for a while
  if (this->safety_triggered_ && this->current_operation_ == ImpulseCoverOperation::IDLE &&
      (now - this->operation_start_time_) > 30000) {  // 30 seconds idle
    ESP_LOGI(TAG, "Resetting safety trigger");
    this->safety_triggered_ = false;
    this->safety_cycle_count_ = 0;
  }
}

void ImpulseCover::handle_endstop() {
  bool at_open = false;
  bool at_closed = false;
  
  if (this->open_sensor_ != nullptr) {
    at_open = this->open_sensor_inverted_ ? !this->open_sensor_->state : this->open_sensor_->state;
  }
  
  if (this->close_sensor_ != nullptr) {
    at_closed = this->close_sensor_inverted_ ? !this->close_sensor_->state : this->close_sensor_->state;
  }
  
  // Check if we hit an endstop
  if (at_open && this->current_operation_ == ImpulseCoverOperation::OPENING) {
    ESP_LOGD(TAG, "Reached open endstop");
    this->position = 1.0f;  // COVER_OPEN
    this->stop_movement();
  } else if (at_closed && this->current_operation_ == ImpulseCoverOperation::CLOSING) {
    ESP_LOGD(TAG, "Reached close endstop");
    this->position = 0.0f;  // COVER_CLOSED
    this->stop_movement();
  }
}

bool ImpulseCover::is_at_target_position() {
  const float tolerance = 0.01f;  // 1% tolerance
  return fabs(this->position - this->target_position_) < tolerance;
}

#ifdef USE_BINARY_SENSOR
void ImpulseCover::set_open_sensor(binary_sensor::BinarySensor *sensor) {
  this->open_sensor_ = sensor;
  if (sensor != nullptr) {
    sensor->add_on_state_callback([this](bool state) {
      bool actual_state = this->open_sensor_inverted_ ? !state : state;
      if (actual_state && this->current_operation_ == ImpulseCoverOperation::OPENING) {
        this->position = 1.0f;  // COVER_OPEN
        this->stop_movement();
      }
    });
  }
}

void ImpulseCover::set_close_sensor(binary_sensor::BinarySensor *sensor) {
  this->close_sensor_ = sensor;
  if (sensor != nullptr) {
    sensor->add_on_state_callback([this](bool state) {
      bool actual_state = this->close_sensor_inverted_ ? !state : state;
      if (actual_state && this->current_operation_ == ImpulseCoverOperation::CLOSING) {
        this->position = 0.0f;  // COVER_CLOSED
        this->stop_movement();
      }
    });
  }
}
#endif

}  // namespace impulse_cover
}  // namespace esphome
