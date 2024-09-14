#pragma once
#include "ha_device.h"
#include "ha_device_mode_input_temperature.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceInputTemp: public esphome::shys_m5_dial::HaDevice {
            protected:
                HaDeviceModeInputTemperature*             modeTemp           = new HaDeviceModeInputTemperature(*this);
                std::string automation_entity_id = "";

            public:
                HaDeviceInputTemp(const std::string& entity_id, const std::string& name, const std::string& automation_entity_id, const std::string& modes) : HaDevice(entity_id, name, modes) {
                    this->automation_entity_id = automation_entity_id;
                }

                void init() override {
                    ESP_LOGD("HA_DEVICE", "Init Input Temp: %s", this->getEntityId().c_str());

                    this->addMode(modeTemp);

                    modeTemp->setAutomationEntityID(automation_entity_id);

                    if (this->modeConfig.containsKey("temp_mode")) {
                        JsonObject temp_mode = this->modeConfig["temp_mode"];

                        if (temp_mode.containsKey("rotary_step_width")) {
                            modeTemp->setRotaryStepWidth(temp_mode["rotary_step_width"].as<int>());
                        }
                    }                    
                }

        };

    }
}