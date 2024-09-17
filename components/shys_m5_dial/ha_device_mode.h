#pragma once

#include "ha_device.h"
#include "ha_api.h"
#include "m5dial_display.h"
#include "M5Dial.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDevice;
        class HaDeviceMode {
            protected:
                float value = 0;
                float minValue = 0;
                float maxValue = 100;

                float rotaryStepWidth = 10;

                int apiSendDelay = 1000;
                int apiSendLock = 2000;
            
                bool currentValueModified = false;
                unsigned long lastApiCall = 0;
                unsigned long lastValueUpdate = 0;

                bool endlessRotaryValue = false;

                bool displayRefreshNeeded = false;
                double lastDisplayRefresh = 0;

                HaApi haApi;
                HaDevice& device;


                HaDevice& getDevice() const {
                    return this->device; 
                }

                bool isApiCallNeeded(){
                    if ( esphome::millis() - lastApiCall < apiSendLock ) {
                        return false;
                    }

                    if ( esphome::millis() - lastValueUpdate < apiSendDelay ) {
                        return false;
                    }

                    return true;
                }

                void raiseCurrentValue(){
                    float newValue = this->getValue() + rotaryStepWidth;
                    newValue = getNextToRotaryStepwidth(newValue);

                    if(newValue > this->maxValue){
                        setValue(endlessRotaryValue?this->minValue:this->maxValue);
                    } else {
                        setValue(newValue);
                    }
                }

                void reduceCurrentValue(){
                    float newValue = this->getValue() - rotaryStepWidth;
                    newValue = getNextToRotaryStepwidth(newValue);

                    if(newValue >= this->minValue){
                        setValue(newValue);
                    } else {
                        setValue(endlessRotaryValue?this->maxValue:this->minValue);
                    }
                }

                int getNextToRotaryStepwidth(float val){
                    if(rotaryStepWidth > 1){
                        float rst = (val % rotaryStepWidth);
                        if(rst >= (rotaryStepWidth/2)){
                            val += rotaryStepWidth-rst;
                        } else {
                            val -= rst;
                        }
                    }
                    return val;
                }

                float getValueForXPosition(float x){
                    return map(x, 0, M5Dial.Display.width(), this->minValue, this->maxValue);
                }

                float getValueForYPosition(float y){
                    return this->maxValue - map(y, 0, M5Dial.Display.height(), this->minValue, this->maxValue) + this->minValue;
                }

                float getDisplayPositionX(float currentValue){
                    return map(currentValue, this->minValue, this->maxValue, 0, M5Dial.Display.width());
                }

                float getDisplayPositionY(float currentValue){
                    return M5Dial.Display.height() - map(currentValue, this->minValue, this->maxValue, 0, M5Dial.Display.height());
                }


                bool defaultOnTouch(M5DialDisplay& display, float x, float y)  {
                    if(y > display.getHeight() * .97){
                        y = display.getHeight();
                    } else if (y < display.getHeight() * .03){
                        y = 0;
                    }
                    
                    float result = this->getValueForYPosition(y);
                    result = getNextToRotaryStepwidth(result);

                    this->setValue(result); 
                    ESP_LOGD("TOUCH", "Aktuellen Wert auf %i gesetzt", result);
                    
                    return true;                    
                }

                bool defaultOnRotary(M5DialDisplay& display, const char * direction)  {
                    if (strcmp(direction, ROTARY_LEFT)==0){
                        this->reduceCurrentValue();
                    } else if (strcmp(direction, ROTARY_RIGHT)==0){
                        this->raiseCurrentValue();
                    }

                    return true;
                }

                virtual void sendValueToHomeAssistant(int value) = 0;

            public:
                HaDeviceMode(HaDevice& device) : device(device) {}

                virtual void refreshDisplay(M5DialDisplay& display, bool init) = 0;
                virtual void registerHAListener() = 0;


                virtual bool isDisplayRefreshNeeded(){
                    return displayRefreshNeeded;
                }

                virtual bool onTouch(M5DialDisplay& display, float x, float y) {
                    return false;
                }

                virtual bool onSwipe(M5DialDisplay& display, const char * direction) {
                    return false;
                }

                virtual bool onRotary(M5DialDisplay& display, const char * direction) {                    
                    return false;
                }

                virtual bool onButton(M5DialDisplay& display, const char * clickType) {
                    return false;
                }

                virtual void onLoop(){}


                void updateHomeAssistantValue(){
                    if(this->isValueModified() && this->isApiCallNeeded() ) {
                        lastApiCall = esphome::millis();

                        this->sendValueToHomeAssistant(getValue());

                        currentValueModified = false;
                    }
                }

                virtual float getValue(){
                    return this->value;
                }

                void setValue(float val){
                    this->value = val;
                    this->lastValueUpdate = esphome::millis();
                    this->currentValueModified = true;
                }

                void updateDisplay(){
                    this->lastValueUpdate = esphome::millis();
                    this->currentValueModified = true;
                }

                void setReceivedValue(float val){
                    //this->value = val;
                    this->setValue(val);
                }


                float getMinValue(){
                    return this->minValue;
                }

                void setMinValue(float val){
                    this->minValue = val;
                }

                float getMaxValue(){
                    return this->maxValue;
                }

                void setMaxValue(float val){
                    this->maxValue = val;
                }

                bool isValueModified(){
                    return this->currentValueModified;
                }

                void setRotaryStepWidth(float stepWidth){
                    this->rotaryStepWidth = stepWidth;
                }

                void setApiSendDelay(int delayInMs){
                    this->apiSendDelay = delayInMs;
                }
                
                void setApiSendLock(int delayInMs){
                    this->apiSendLock = delayInMs;
                }
        };
    }
}