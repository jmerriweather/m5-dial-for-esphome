#pragma once

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeInputTemperature: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                std::string automation_state = "";
                bool refreshNeeded = true;

                float current_temperature = 0;
                int max_temperature = 40;
                int min_temperature = 0;

                std::string automation_entity_id = "";
                std::string currentTemperatureEntityID = "";

                void sendValueToHomeAssistant(int value) override {
                    haApi.setInputNumber(this->device.getEntityId(), value);
                }

                void showTemperatureMenu(M5DialDisplay& display){
                    LovyanGFX* gfx = display.getGfx();

                    uint16_t currentValue = getValue();

                    uint16_t height = gfx->height();
                    uint16_t width  = gfx->width();

                    gfx->setTextColor(MAROON);
                    gfx->setTextDatum(middle_center);
                    
                    gfx->startWrite();                      // Secure SPI bus

                    if(strcmp(automation_state.c_str(), "off")==0){
                        gfx->fillRect(0, 0, width, this->getDisplayPositionY(currentValue) , DARKGREY);
                        gfx->fillRect(0, this->getDisplayPositionY(currentValue), width, height, LIGHTGREY);
                        gfx->fillRect(0, this->getDisplayPositionY(current_temperature), width, 5, RED);
                    } else {
                        gfx->fillRect(0, 0, width, this->getDisplayPositionY(currentValue) , GREEN);
                        gfx->fillRect(0, this->getDisplayPositionY(currentValue), width, height, DARKGREEN);
                        gfx->fillRect(0, this->getDisplayPositionY(current_temperature), width, 5, RED);
                    }

                    display.setFontsize(3);
                    gfx->drawString(String(currentValue).c_str(),
                                    width / 2,
                                    height / 2 - 30);                        
                    
                    display.setFontsize(1);
                    
                    gfx->drawString(String(current_temperature).c_str(),
                                    width / 2 + 60,
                                    height / 2 - 30);   
                    std::string name;
                    name.append(this->device.getName());
                    name.append(" (");
                    name.append(this->getAutomationState());
                    name.append(")");
                    gfx->drawString(name.c_str(),
                                    width / 2,
                                    height / 2 + 20);
                    gfx->drawString("Temperature",
                                    width / 2,
                                    height / 2 + 50);

                    gfx->endWrite();                      // Release SPI bus
                }

            public:
                std::string getAutomationState(){
                    return automation_state;
                }

                void setAutomationState(const std::string&  newMode){
                    automation_state = newMode;
                }

                std::string getAutomationEntityID(){
                    return automation_entity_id;
                }

                void setCurrentTemperature(float newTemp){
                    current_temperature = newTemp;
                }

                float getCurrentTemperature(){
                    return current_temperature;
                }

                void setCurrentTemperatureEntityID(const std::string& newMode){
                    currentTemperatureEntityID = newMode;
                }

                void setAutomationEntityID(const std::string& newMode){
                    automation_entity_id = newMode;
                }

                int getMinTemperature(){
                    return min_temperature;
                }

                int getMaxTemperature(){
                    return max_temperature;
                }

                void setMinTemperature(int newMin){
                    min_temperature = newMin;
                }

                void setMaxTemperature(int newMax){
                    max_temperature = newMax;
                }

                HaDeviceModeInputTemperature(HaDevice& device) : HaDeviceMode(device){
                    this->maxValue = 40;
                }

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    this->showTemperatureMenu(display);
                    ESP_LOGD("DISPLAY", "Temperature-Modus");
                }

                bool isDisplayRefreshNeeded() override {
                    bool refresh = refreshNeeded;
                    refreshNeeded = false;                    
                    return refresh;
                }

                void registerHAListener() override {        
                    std::string attrName = "";
                    api::global_api_server->subscribe_home_assistant_state(
                                automation_entity_id.c_str(),
                                attrName, 
                                [this](const std::string &state) {
                                    
                        ESP_LOGI("HA_API", "Got automation value %s for %s", state.c_str(), automation_entity_id.c_str());

                        automation_state = state;
                        refreshNeeded = true;
                    });

                    std::string attrName2 = "";
                    api::global_api_server->subscribe_home_assistant_state(
                                currentTemperatureEntityID.c_str(),
                                attrName2, 
                                [this](const std::string &state) {
                                    
                        ESP_LOGI("HA_API", "Got current temperature value %s for %s", state.c_str(), currentTemperatureEntityID.c_str());
                        
                        auto val = parse_number<float>(state);
                        if (!val.has_value()) {
                            current_temperature = 0;
                            ESP_LOGD("HA_API", "No current temperature value in %s for %s", state.c_str(), currentTemperatureEntityID.c_str());
                        } else {
                            float new_val = val.value();
                            if(new_val != current_temperature){
                                current_temperature = new_val;
                                refreshNeeded = true;
                                ESP_LOGI("HA_API", "Got current temperature value %i for %s", new_val, currentTemperatureEntityID.c_str());
                            }
                        }


                    });

                    std::string attrNameTemp = "";
                    api::global_api_server->subscribe_home_assistant_state(
                                this->device.getEntityId().c_str(),
                                attrNameTemp, 
                                [this](const std::string &state) {

                        if(this->isValueModified()){
                            return;
                        }

                        auto val = parse_number<float>(state);

                        if (!val.has_value()) {
                            this->setReceivedValue(0);
                            ESP_LOGD("HA_API", "No Temperature value in %s for %s", state.c_str(), this->device.getEntityId().c_str());
                        } else {
                            this->setReceivedValue(int(val.value()));
                            ESP_LOGI("HA_API", "Got Temperature value %i for %s", int(val.value()), this->device.getEntityId().c_str());
                        }
                        refreshNeeded = true;
                    });
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    //return defaultOnRotary(display, direction);

                    if (strcmp(direction, ROTARY_LEFT)==0){
                        if(this->getValue() > min_temperature){
                            this->reduceCurrentValue();
                        }
                    } else if (strcmp(direction, ROTARY_RIGHT)==0){
                        if(this->getValue() < max_temperature){
                            this->raiseCurrentValue();
                        }
                    }

                    return true;
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0){
                        haApi.toggleInputBoolean(this->getAutomationEntityID());
                        return true;
                    } 
                    return false;
                }

        };
    }
}