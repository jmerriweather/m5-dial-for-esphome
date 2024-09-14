#pragma once

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeInputTemperature: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                std::string automation_state = "";

                std::string automation_entity_id = "";

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

                    if(strcmp(this->getAutomationState().c_str(), "off")==0){
                        gfx->fillRect(0, 0, width, this->getDisplayPositionY(currentValue) , DARKGREY);
                        gfx->fillRect(0, this->getDisplayPositionY(currentValue), width, height, LIGHTGREY);
                    } else {
                        gfx->fillRect(0, 0, width, this->getDisplayPositionY(currentValue) , RED);
                        gfx->fillRect(0, this->getDisplayPositionY(currentValue), width, height, YELLOW);
                    }

                    display.setFontsize(3);
                    gfx->drawString(String(currentValue).c_str(),
                                    width / 2,
                                    height / 2 - 30);                        
                    
                    display.setFontsize(1);
                    std::string name;
                    name.append(this->device.getName().c_str());
                    name.append(" (");
                    name.append(this->getAutomationState().c_str());
                    name.append(")");
                    gfx->drawString(name,
                                    width / 2,
                                    height / 2 + 20);
                    gfx->drawString("Temperature",
                                    width / 2,
                                    height / 2 + 50);

                    gfx->endWrite();                      // Release SPI bus
                }

            public:
                std::string getAutomationState(){
                    return this->automation_state;
                }

                void setAutomationState(const std::string&  newMode){
                    this->automation_state = newMode;
                }

                std::string getAutomationEntityID(){
                    return this->automation_entity_id;
                }

                void setAutomationEntityID(const std::string& newMode){
                    this->automation_entity_id = newMode;
                }

                HaDeviceModeInputTemperature(HaDevice& device) : HaDeviceMode(device){
                    this->maxValue = 40;
                }

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    this->showTemperatureMenu(display);
                    ESP_LOGD("DISPLAY", "Temperature-Modus");
                }

                void registerHAListener() override {                    
                    std::string attrName = "";
                    api::global_api_server->subscribe_home_assistant_state(
                                this->getAutomationEntityID().c_str(),
                                attrName, 
                                [this](const std::string &state) {
                                    
                        if(this->isValueModified()){
                            // do nothing
                        } else {
                            this->setAutomationState(state.c_str());
                            ESP_LOGI("HA_API", "Got value %s for %s", state.c_str(), this->device.getEntityId().c_str());
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
                    });
                }


                bool onTouch(M5DialDisplay& display, uint16_t x, uint16_t y) override {
                    return defaultOnTouch(display, x, y);        
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    return defaultOnRotary(display, direction);
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0){
                        if(strcmp(this->getAutomationState().c_str(), "off")==0){
                            haApi.turnInputBooleanOn(this->getAutomationEntityID());
                        } else {
                            haApi.turnInputBooleanOff(this->getAutomationEntityID());
                        }
                        
                        return true;
                    } 
                    return false;
                }

        };
    }
}