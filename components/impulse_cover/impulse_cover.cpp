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
  ESP_LOGCONFIG(TAG, "Setting up Impulse Cover...");
  
  if (this->output_ == nullptr) {
    ESP_LOGE(TAG, "Output is required!");
    this->mark_failed();
    return;
  }
  
  // Initialize position based on sensors if available
#ifdef USE_BINARY_SENSOR
  if (this->open_sensor_ != nullptr && this->close_sensor_ != nullptr) {
    bool open_state = this->open_sensor_inverted_ ? !this->open_sensor_->state : this->open_sensor_->state;
    bool close_state = this->close_sensor_inverted_ ? !this->close_sensor_->state : this->close_sensor_->state;
    
    if (open_state && !close_state) {
      this->position = 1.0f;  // Fully open
      this->has_initial_state_ = true;
    } else if (!open_state && close_state) {
      this->position = 0.0f;  // Fully closed
      this->has_initial_state_ = true;
    } else {
      this->position = 0.5f;  // Unknown position
    }
  } else {
    // No sensors, start at unknown position
    this->position = 0.5f;
  }
#else
  // No sensors, start at unknown position
  this->position = 0.5f;
#endif
  
  ESP_LOGCONFIG(TAG, "Impulse Cover setup complete");
}

void ImpulseCover::loop() {
  const uint32_t now = millis();
  
  // Store previous state for change detection
  float previous_position = this->position;
  cover::CoverOperation previous_operation = this->current_operation_ == ImpulseCoverOperation::IDLE ? 
                                            cover::COVER_OPERATION_IDLE : 
                                            (this->current_operation_ == ImpulseCoverOperation::OPENING ? 
                                             cover::COVER_OPERATION_OPENING : cover::COVER_OPERATION_CLOSING);
  
  // Update position based on time if moving
  this->update_position();
  
  // Check safety conditions
  this->check_safety();
  
  // Handle endstop checks and position updates from sensors
#ifdef USE_BINARY_SENSOR
  this->handle_endstop();
  this->update_position_from_sensors();
#endif
  
  // Handle pulse timing
  if (this->current_operation_ != ImpulseCoverOperation::IDLE) {
    if (!this->pulse_sent_ && (now - this->last_pulse_time_) >= this->pulse_delay_) {
      this->send_pulse();
    }
    
    // Check if we should reverse direction or stop
    if (this->pending_reverse_ && (now - this->last_pulse_time_) >= this->pulse_delay_) {
      this->pending_reverse_ = false;
      this->start_direction(this->target_operation_ == ImpulseCoverOperation::OPENING ? 
                           cover::COVER_OPERATION_OPENING : cover::COVER_OPERATION_CLOSING);
    }
    
    // Check if operation is complete
    if (this->is_at_target_position()) {
      this->stop_movement();
    }
  }
  
  // Auto-reset safety cycle count after 30 seconds of inactivity
  if (this->current_operation_ == ImpulseCoverOperation::IDLE && 
      this->safety_cycle_count_ > 0 && 
      (now - this->last_direction_change_) > 30000) {
    ESP_LOGD(TAG, "Auto-resetting safety cycle count after inactivity");
    this->safety_cycle_count_ = 0;
  }
  
  // Only publish state if something changed
  cover::CoverOperation current_operation = this->current_operation_ == ImpulseCoverOperation::IDLE ? 
                                           cover::COVER_OPERATION_IDLE : 
                                           (this->current_operation_ == ImpulseCoverOperation::OPENING ? 
                                            cover::COVER_OPERATION_OPENING : cover::COVER_OPERATION_CLOSING);
  
  if (fabs(this->position - previous_position) > 0.01f || current_operation != previous_operation) {
    this->publish_state();
  }
}

void ImpulseCover::dump_config() {
  ESP_LOGCONFIG(TAG, "Impulse Cover:");
  ESP_LOGCONFIG(TAG, "  Open Duration: %ums", this->open_duration_);
  ESP_LOGCONFIG(TAG, "  Close Duration: %ums", this->close_duration_);
  ESP_LOGCONFIG(TAG, "  Pulse Delay: %ums", this->pulse_delay_);
  ESP_LOGCONFIG(TAG, "  Safety Timeout: %ums", this->safety_timeout_);
  ESP_LOGCONFIG(TAG, "  Safety Max Cycles: %u", this->safety_max_cycles_);
  
#ifdef USE_BINARY_SENSOR
  if (this->open_sensor_) {
    ESP_LOGCONFIG(TAG, "  Open Sensor: %s", this->open_sensor_->get_name().c_str());
    ESP_LOGCONFIG(TAG, "  Open Sensor Inverted: %s", this->open_sensor_inverted_ ? "YES" : "NO");
  }
  if (this->close_sensor_) {
    ESP_LOGCONFIG(TAG, "  Close Sensor: %s", this->close_sensor_->get_name().c_str());
    ESP_LOGCONFIG(TAG, "  Close Sensor Inverted: %s", this->close_sensor_inverted_ ? "YES" : "NO");
  }
#endif
}

cover::CoverTraits ImpulseCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_supports_position(true);
  traits.set_supports_tilt(false);
  traits.set_supports_stop(true);
#ifdef USE_BINARY_SENSOR
  traits.set_is_assumed_state(this->open_sensor_ == nullptr || this->close_sensor_ == nullptr);
#else
  traits.set_is_assumed_state(true);
#endif
  return traits;
}

void ImpulseCover::control(const cover::CoverCall &call) {
  if (this->safety_triggered_) {
    ESP_LOGW(TAG, "Cover is in safety mode, ignoring command");
    return;
  }

  // Handle stop command first
  if (call.get_stop()) {
    ESP_LOGD(TAG, "Stop command received");
    if (this->current_operation_ != ImpulseCoverOperation::IDLE) {
      this->send_pulse();  // Stop current movement
    }
    return;
  }

  // Handle position commands
  if (call.get_position().has_value()) {
    float target = *call.get_position();
    ESP_LOGD(TAG, "Position command: %.2f", target);
    
    this->target_position_ = target;
    
    if (target > this->position + 0.01f) {
      // Need to open
      this->target_operation_ = ImpulseCoverOperation::OPENING;
      this->start_direction(cover::COVER_OPERATION_OPENING);
    } else if (target < this->position - 0.01f) {
      // Need to close
      this->target_operation_ = ImpulseCoverOperation::CLOSING;
      this->start_direction(cover::COVER_OPERATION_CLOSING);
    }
    return;
  }

  // Handle toggle command
  if (call.get_toggle().has_value()) {
    ESP_LOGD(TAG, "Toggle command received");
    
    if (this->current_operation_ != ImpulseCoverOperation::IDLE) {
      // Currently moving, stop it
      this->send_pulse();
    } else {
      // Currently idle, toggle based on position
      if (this->position < 0.5f) {
        // More closed than open, so open
        this->target_position_ = 1.0f;
        this->target_operation_ = ImpulseCoverOperation::OPENING;
        this->start_direction(cover::COVER_OPERATION_OPENING);
      } else {
        // More open than closed, so close
        this->target_position_ = 0.0f;
        this->target_operation_ = ImpulseCoverOperation::CLOSING;
        this->start_direction(cover::COVER_OPERATION_CLOSING);
      }
    }
    return;
  }
}

void ImpulseCover::start_direction(cover::CoverOperation dir) {
  if (this->safety_triggered_) {
    ESP_LOGW(TAG, "Cannot start movement: safety triggered");
    return;
  }

  const uint32_t now = millis();
  
  // Prevent rapid direction changes
  if ((now - this->last_direction_change_) < this->pulse_delay_) {
    ESP_LOGD(TAG, "Direction change too rapid, delaying");
    return;
  }
  
  // If already moving in opposite direction, need to stop first then reverse
  if ((dir == cover::COVER_OPERATION_OPENING && this->current_operation_ == ImpulseCoverOperation::CLOSING) ||
      (dir == cover::COVER_OPERATION_CLOSING && this->current_operation_ == ImpulseCoverOperation::OPENING)) {
    ESP_LOGD(TAG, "Reversing direction, stopping first");
    this->pending_reverse_ = true;
    this->target_operation_ = (dir == cover::COVER_OPERATION_OPENING) ? 
                             ImpulseCoverOperation::OPENING : ImpulseCoverOperation::CLOSING;
    this->send_pulse();  // Stop current movement
    return;
  }
  
  // If already moving in same direction, continue
  if ((dir == cover::COVER_OPERATION_OPENING && this->current_operation_ == ImpulseCoverOperation::OPENING) ||
      (dir == cover::COVER_OPERATION_CLOSING && this->current_operation_ == ImpulseCoverOperation::CLOSING)) {
    ESP_LOGD(TAG, "Already moving in requested direction");
    return;
  }
  
  // Start new movement
  this->current_operation_ = (dir == cover::COVER_OPERATION_OPENING) ? 
                            ImpulseCoverOperation::OPENING : ImpulseCoverOperation::CLOSING;
  this->operation_start_time_ = now;
  this->last_direction_change_ = now;
  this->pulse_sent_ = false;
  this->start_position_ = this->position;  // Store starting position for correct calculation
  
  // Increment safety cycle count for rapid direction changes
  this->safety_cycle_count_++;
  ESP_LOGD(TAG, "Safety cycle count: %u/%u", this->safety_cycle_count_, this->safety_max_cycles_);
  
  // Fire appropriate automation triggers
  if (dir == cover::COVER_OPERATION_OPENING) {
    this->fire_on_open_triggers_();
  } else if (dir == cover::COVER_OPERATION_CLOSING) {
    this->fire_on_close_triggers_();
  }
  
  ESP_LOGD(TAG, "Starting %s operation from position %.2f to %.2f", 
           this->current_operation_ == ImpulseCoverOperation::OPENING ? "OPEN" : "CLOSE",
           this->start_position_, this->target_position_);
}

void ImpulseCover::stop_movement() {
  if (this->current_operation_ != ImpulseCoverOperation::IDLE) {
    ESP_LOGD(TAG, "Stopping movement");
    
    // Ensure position matches target when stopping
    if (this->is_at_target_position()) {
      this->position = this->target_position_;
      ESP_LOGD(TAG, "Movement completed, position set to target: %.2f", this->position);
    }
    
    this->current_operation_ = ImpulseCoverOperation::IDLE;
    this->target_operation_ = ImpulseCoverOperation::IDLE;
    this->pending_reverse_ = false;
    this->pulse_sent_ = false;
    
    // Fire idle trigger when movement stops
    this->fire_on_idle_triggers_();
  }
}

void ImpulseCover::send_pulse() {
  if (this->output_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send pulse: output not configured");
    return;
  }
  
  const uint32_t now = millis();
  
  if (this->pulse_sent_ || (now - this->last_pulse_time_) < this->pulse_delay_) {
    return;  // Too soon for another pulse
  }
  
  ESP_LOGD(TAG, "Sending control pulse");
  
  // Send pulse (turn on briefly then off)
  this->output_->turn_on();
  this->set_timeout(100, [this]() {
    this->output_->turn_off();
  });
  
  this->last_pulse_time_ = now;
  this->pulse_sent_ = true;
  
  // Reset pulse flag after delay
  this->set_timeout(this->pulse_delay_, [this]() {
    this->pulse_sent_ = false;
  });
}

void ImpulseCover::update_position() {
  if (this->current_operation_ == ImpulseCoverOperation::IDLE) {
    return;
  }
  
  const uint32_t now = millis();
  const uint32_t elapsed = now - this->operation_start_time_;
  
  float progress = 0.0f;
  
  if (this->current_operation_ == ImpulseCoverOperation::OPENING) {
    progress = static_cast<float>(elapsed) / static_cast<float>(this->open_duration_);
    progress = std::min(1.0f, progress);  // Clamp to [0,1]
    // Calculate position based on start and target
    this->position = this->start_position_ + (this->target_position_ - this->start_position_) * progress;
    this->position = std::min(1.0f, std::max(0.0f, this->position));  // Clamp to [0,1]
  } else if (this->current_operation_ == ImpulseCoverOperation::CLOSING) {
    progress = static_cast<float>(elapsed) / static_cast<float>(this->close_duration_);
    progress = std::min(1.0f, progress);  // Clamp to [0,1]
    // Calculate position based on start and target
    this->position = this->start_position_ + (this->target_position_ - this->start_position_) * progress;
    this->position = std::min(1.0f, std::max(0.0f, this->position));  // Clamp to [0,1]
  }
  
  this->last_position_update_ = now;
}

void ImpulseCover::check_safety() {
  if (this->current_operation_ == ImpulseCoverOperation::IDLE) {
    return;
  }
  
  const uint32_t now = millis();
  const uint32_t elapsed = now - this->operation_start_time_;
  
  // Check timeout
  if (elapsed > this->safety_timeout_) {
    ESP_LOGW(TAG, "Safety timeout triggered after %ums", elapsed);
    this->safety_triggered_ = true;
    this->fire_on_safety_triggers_();
    this->stop_movement();
    return;
  }
  
  // Check cycle count
  if (this->safety_cycle_count_ >= this->safety_max_cycles_) {
    ESP_LOGW(TAG, "Safety max cycles triggered (%u cycles)", this->safety_cycle_count_);
    this->safety_triggered_ = true;
    this->fire_on_safety_triggers_();
    this->stop_movement();
    return;
  }
}

#ifdef USE_BINARY_SENSOR
void ImpulseCover::handle_endstop() {
  if (this->current_operation_ == ImpulseCoverOperation::IDLE) {
    return;
  }
  
  // Check open sensor
  if (this->open_sensor_ != nullptr && this->current_operation_ == ImpulseCoverOperation::OPENING) {
    bool sensor_state = this->open_sensor_inverted_ ? !this->open_sensor_->state : this->open_sensor_->state;
    if (sensor_state) {
      ESP_LOGD(TAG, "Open sensor triggered, stopping movement");
      this->position = 1.0f;  // Fully open
      this->stop_movement();
      return;
    }
  }
  
  // Check close sensor
  if (this->close_sensor_ != nullptr && this->current_operation_ == ImpulseCoverOperation::CLOSING) {
    bool sensor_state = this->close_sensor_inverted_ ? !this->close_sensor_->state : this->close_sensor_->state;
    if (sensor_state) {
      ESP_LOGD(TAG, "Close sensor triggered, stopping movement");
      this->position = 0.0f;  // Fully closed
      this->stop_movement();
      return;
    }
  }
}

void ImpulseCover::update_position_from_sensors() {
  // Update position based on current sensor states, even when not moving
  if (this->open_sensor_ != nullptr && this->close_sensor_ != nullptr) {
    bool open_state = this->open_sensor_inverted_ ? !this->open_sensor_->state : this->open_sensor_->state;
    bool close_state = this->close_sensor_inverted_ ? !this->close_sensor_->state : this->close_sensor_->state;
    
    if (open_state && !close_state) {
      // Fully open
      if (fabs(this->position - 1.0f) > 0.01f) {
        ESP_LOGD(TAG, "Open sensor active, updating position to 1.0 (was %.2f)", this->position);
        this->position = 1.0f;
      }
    } else if (!open_state && close_state) {
      // Fully closed
      if (fabs(this->position - 0.0f) > 0.01f) {
        ESP_LOGD(TAG, "Close sensor active, updating position to 0.0 (was %.2f)", this->position);
        this->position = 0.0f;
      }
    }
    // If both sensors are active or both inactive, keep current position
    // This could indicate a sensor error or intermediate position
  }
}
#endif

bool ImpulseCover::is_at_target_position() {
  const float tolerance = 0.01f;  // 1% tolerance
  return fabs(this->position - this->target_position_) < tolerance;
}

#ifdef USE_BINARY_SENSOR
void ImpulseCover::set_open_sensor(binary_sensor::BinarySensor *sensor) {
  this->open_sensor_ = sensor;
}

void ImpulseCover::set_close_sensor(binary_sensor::BinarySensor *sensor) {
  this->close_sensor_ = sensor;
}
#endif

// Automation trigger methods
void ImpulseCover::add_on_open_trigger(Trigger<> *trigger) {
  this->on_open_triggers_.push_back(trigger);
}

void ImpulseCover::add_on_close_trigger(Trigger<> *trigger) {
  this->on_close_triggers_.push_back(trigger);
}

void ImpulseCover::add_on_idle_trigger(Trigger<> *trigger) {
  this->on_idle_triggers_.push_back(trigger);
}

void ImpulseCover::add_on_safety_trigger(SafetyTrigger *trigger) {
  this->on_safety_triggers_.push_back(trigger);
}

// Protected helper methods for firing triggers
void ImpulseCover::fire_on_open_triggers_() {
  for (auto *trigger : this->on_open_triggers_) {
    trigger->trigger();
  }
}

void ImpulseCover::fire_on_close_triggers_() {
  for (auto *trigger : this->on_close_triggers_) {
    trigger->trigger();
  }
}

void ImpulseCover::fire_on_idle_triggers_() {
  for (auto *trigger : this->on_idle_triggers_) {
    trigger->trigger();
  }
}

void ImpulseCover::fire_on_safety_triggers_() {
  for (auto *trigger : this->on_safety_triggers_) {
    trigger->trigger();
  }
}

}  // namespace impulse_cover
}  // namespace esphome
