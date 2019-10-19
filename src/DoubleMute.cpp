#include "plugin.hpp"

struct DoubleMute : Module {

    const float PRESET_FADE = 0.1;

    enum ParamIds {
        MUTE_PARAM,
        FADE_IN_PARAM,
        FADE_OUT_PARAM,
        SCALE_IN_PARAM,
        SCALE_OUT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN1_INPUT,
        IN2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        MUTE_LIGHT,
        NUM_LIGHTS
    };

    enum States {
        HIGH,
        LOW,
        RAMP_UP,
        RAMP_DOWN
    };

    States state;
    dsp::BooleanTrigger muteTrigger;
    float fadeTimeEllapsed;

    DoubleMute() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MUTE_PARAM, 0.0, 1.0, 0.0, "Mute channels");
        configParam(FADE_IN_PARAM, 0.01, 1.0, PRESET_FADE, "Fade-in time");
        configParam(FADE_OUT_PARAM, 0.01, 1.0, PRESET_FADE, "Fade-out time");
        configParam(SCALE_IN_PARAM, 0.0, 2.0, 0.0, "Fade-in scale");
        configParam(SCALE_OUT_PARAM, 0.0, 2.0, 0.0, "Fade-out scale");

        onReset();
    }

    void onReset() override {
    }

    void process(const ProcessArgs& args) override {
        setState();

        switch(state) {
            case HIGH:
                lights[MUTE_LIGHT].setBrightness(0.9f);
                high();
                break;
            case RAMP_UP:
                rampUp(args.sampleTime);
                break;
            case LOW:
                break;
            case RAMP_DOWN:
                rampDown(args.sampleTime);
                break;
        }
    }

    void setState() {
        if (muteTrigger.process(params[MUTE_PARAM].getValue() > 0.f)) {
            switch(state) {
                case HIGH:
                    state = RAMP_DOWN;
                    fadeTimeEllapsed = rampDownTime();
                    lights[MUTE_LIGHT].setBrightness(0.f);
                    break;
                case RAMP_UP:
                    state = RAMP_DOWN;
                    fadeTimeEllapsed = rampUpToDownTime();
                    lights[MUTE_LIGHT].setBrightness(0.f);
                    break;
                case LOW:
                    state = RAMP_UP;
                    fadeTimeEllapsed = 0.f;
                    lights[MUTE_LIGHT].setBrightness(0.9f);
                    break;
                case RAMP_DOWN:
                    state = RAMP_UP;
                    fadeTimeEllapsed = rampDownToUpTime();
                    lights[MUTE_LIGHT].setBrightness(0.9f);
                    break;
            }
        }
    }

    float rampUpToDownTime() {
        float ratio = fadeTimeEllapsed / rampUpTime();
        return rampDownTime() * ratio;
    }

    float rampDownToUpTime() {
        float ratio = fadeTimeEllapsed / rampDownTime();
        return rampUpTime() * ratio;
    }

    float rampDownTime() {
        float time = params[FADE_OUT_PARAM].getValue();
        int mult = (int) std::round(params[SCALE_OUT_PARAM].getValue());
        switch(mult) {
            case 1:
                time *= 10;
                break;
            case 2:
                time *= 100;
                break;
            default:
                break;
        }
        return time;
    }

    float rampUpTime() {
        float time = params[FADE_IN_PARAM].getValue();
        int mult = (int) std::round(params[SCALE_IN_PARAM].getValue());
        switch(mult) {
            case 1:
                time *= 10;
                break;
            case 2:
                time *= 100;
                break;
            default:
                break;
        }
        return time;
    }

    void rampUp(float sampleTime) {
        fadeTimeEllapsed += sampleTime;
        float userValue = rampUpTime();

        if (inputs[IN1_INPUT].isConnected() && outputs[OUT1_OUTPUT].isConnected()) {
            outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * (fadeTimeEllapsed / userValue));
        }
        if (inputs[IN2_INPUT].isConnected() && outputs[OUT2_OUTPUT].isConnected()) {
            outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * (fadeTimeEllapsed / userValue));
        }
        if (fadeTimeEllapsed >= userValue) {
            state = HIGH;
        }
    }

    void high() {
        if (inputs[IN1_INPUT].isConnected() && outputs[OUT1_OUTPUT].isConnected()) {
            outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
        }
        if (inputs[IN2_INPUT].isConnected() && outputs[OUT2_OUTPUT].isConnected()) {
            outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
        }
    }

    void rampDown(float sampleTime) {
        fadeTimeEllapsed -= sampleTime;
        float userValue = rampDownTime();

        if (inputs[IN1_INPUT].isConnected() && outputs[OUT1_OUTPUT].isConnected()) {
            outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * (fadeTimeEllapsed) / userValue);
        }
        if (inputs[IN2_INPUT].isConnected() && outputs[OUT2_OUTPUT].isConnected()) {
            outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * (fadeTimeEllapsed) / userValue);
        }
        if (fadeTimeEllapsed <= 0.f) {
            state = LOW;
        }
    }

    json_t *dataToJson() override {
        float value;
        if (state == HIGH || state == RAMP_UP) {
            value = 1;
        } else {
            value = 0;
        }
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "state", json_integer(value));
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override {
        json_t *stateJ = json_object_get(rootJ, "state");
        if (stateJ) {
            float value = json_integer_value(stateJ);
            if (value == 1) {
                state = HIGH;
            } else {
                state = LOW;
            }
        }
    }
};

template <typename BASE>
struct MuteLight : BASE {
    MuteLight() {
        this->box.size = mm2px(Vec(6.f, 6.f));
    }
};

struct DoubleMuteWidget : ModuleWidget {
    DoubleMuteWidget(DoubleMute* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DoubleMute.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<LEDBezel>(mm2px(Vec(15.24, 32.5)), module, DoubleMute::MUTE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 64.0)), module, DoubleMute::FADE_IN_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 99.5)), module, DoubleMute::FADE_OUT_PARAM));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(15.24, 76.5)), module, DoubleMute::SCALE_IN_PARAM));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(15.24, 112.0)), module, DoubleMute::SCALE_OUT_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 22.5)), module, DoubleMute::IN1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 42.5)), module, DoubleMute::IN2_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 22.5)), module, DoubleMute::OUT1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 42.5)), module, DoubleMute::OUT2_OUTPUT));

        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(15.24, 32.5)), module, DoubleMute::MUTE_LIGHT));
    }
};


Model* modelDoubleMute = createModel<DoubleMute, DoubleMuteWidget>("DoubleMute");