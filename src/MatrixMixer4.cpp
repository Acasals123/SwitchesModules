#include "plugin.hpp"

struct MatrixMixer4 : Module {

    enum ParamIds {
        ENUMS(POT_PARAMS, 16),
        ROW1_PARAM,
        ROW2_PARAM,
        ROW3_PARAM,
        ROW4_PARAM,
        COL1_PARAM,
        COL2_PARAM,
        COL3_PARAM,
        COL4_PARAM,
        XOR_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        ENUMS(IN_INPUTS, 4),
        ROW1_CV_INPUT,
        ROW2_CV_INPUT,
        ROW3_CV_INPUT,
        ROW4_CV_INPUT,
        COL1_CV_INPUT,
        COL2_CV_INPUT,
        COL3_CV_INPUT,
        COL4_CV_INPUT,
        XOR_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        ENUMS(OUT_OUTPUTS, 4),
        NUM_OUTPUTS
    };

    enum LightIds {
        ENUMS(SMALL_LEDS, 16),
        ROW1_LIGHT,
        ROW2_LIGHT,
        ROW3_LIGHT,
        ROW4_LIGHT,
        COL1_LIGHT,
        COL2_LIGHT,
        COL3_LIGHT,
        COL4_LIGHT,
        XOR_LIGHT,
        NUM_LIGHTS
    };

    MatrixMixer4() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(POT_PARAMS + 0, 0.f, 1.f, 0.5f, "Row 1 Col 1");
        configParam(POT_PARAMS + 1, 0.f, 1.f, 0.5f, "Row 1 Col 2");
        configParam(POT_PARAMS + 2, 0.f, 1.f, 0.5f, "Row 1 Col 3");
        configParam(POT_PARAMS + 3, 0.f, 1.f, 0.5f, "Row 1 Col 4");
        configParam(POT_PARAMS + 4, 0.f, 1.f, 0.5f, "Row 2 Col 1");
        configParam(POT_PARAMS + 5, 0.f, 1.f, 0.5f, "Row 2 Col 2");
        configParam(POT_PARAMS + 6, 0.f, 1.f, 0.5f, "Row 2 Col 3");
        configParam(POT_PARAMS + 7, 0.f, 1.f, 0.5f, "Row 2 Col 4");
        configParam(POT_PARAMS + 8, 0.f, 1.f, 0.5f, "Row 3 Col 1");
        configParam(POT_PARAMS + 9, 0.f, 1.f, 0.5f, "Row 3 Col 2");
        configParam(POT_PARAMS + 10, 0.f, 1.f, 0.5f, "Row 3 Col 3");
        configParam(POT_PARAMS + 11, 0.f, 1.f, 0.5f, "Row 3 Col 4");
        configParam(POT_PARAMS + 12, 0.f, 1.f, 0.5f, "Row 4 Col 1");
        configParam(POT_PARAMS + 13, 0.f, 1.f, 0.5f, "Row 4 Col 2");
        configParam(POT_PARAMS + 14, 0.f, 1.f, 0.5f, "Row 4 Col 3");
        configParam(POT_PARAMS + 15, 0.f, 1.f, 0.5f, "Row 4 Col 4");
    }

    void process(const ProcessArgs& args) override {
        for (int outputNumber = 0; outputNumber < 4; outputNumber++) {
            if (outputs[OUT_OUTPUTS + outputNumber].isConnected()) {
                float out = 0.f;
                int numberOfConnections = 0;

                for (int i = 0; i < 4; i++) {
                    if (inputs[IN_INPUTS + i].isConnected()) {
                        out += inputs[IN_INPUTS + i]
                               .getVoltage() *
                               params[POT_PARAMS + (4 * outputNumber) + i]
                               .getValue();
                        numberOfConnections++;
                    }
                }

                if (numberOfConnections == 2) {
                    out *= 0.5;
                } else if (numberOfConnections == 3) {
                    out *= 0.33333;
                } else if (numberOfConnections == 4) {
                    out *= 0.25;
                }

                /* out = clamp(out, -5.f, 5.f); */
                outputs[OUT_OUTPUTS + outputNumber].setVoltage(out);
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

struct MatrixMixer4Widget : ModuleWidget {
    MatrixMixer4Widget(MatrixMixer4* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MatrixMixer4.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(27.5, 35.5)), module, MatrixMixer4::POT_PARAMS + 0));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.17, 35.5)), module, MatrixMixer4::POT_PARAMS + 1));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.84, 35.5)), module, MatrixMixer4::POT_PARAMS + 2));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.51, 35.5)), module, MatrixMixer4::POT_PARAMS + 3));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(27.5, 54.17)), module, MatrixMixer4::POT_PARAMS + 4));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.17, 54.17)), module, MatrixMixer4::POT_PARAMS + 5));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.84, 54.17)), module, MatrixMixer4::POT_PARAMS + 6));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.51, 54.17)), module, MatrixMixer4::POT_PARAMS + 7));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(27.5, 72.84)), module, MatrixMixer4::POT_PARAMS + 8));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.17, 72.84)), module, MatrixMixer4::POT_PARAMS + 9));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.84, 72.84)), module, MatrixMixer4::POT_PARAMS + 10));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.51, 72.84)), module, MatrixMixer4::POT_PARAMS + 11));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(27.5, 91.51)), module, MatrixMixer4::POT_PARAMS + 12));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.17, 91.51)), module, MatrixMixer4::POT_PARAMS + 13));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.84, 91.51)), module, MatrixMixer4::POT_PARAMS + 14));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.51, 91.51)), module, MatrixMixer4::POT_PARAMS + 15));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.5, 19.5)), module, MatrixMixer4::IN_INPUTS + 0));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.17, 19.5)), module, MatrixMixer4::IN_INPUTS + 1));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.84, 19.5)), module, MatrixMixer4::IN_INPUTS + 2));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.51, 19.5)), module, MatrixMixer4::IN_INPUTS + 3));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.804, 35.5)), module, MatrixMixer4::ROW1_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.388, 54.17)), module, MatrixMixer4::ROW2_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.426, 72.84)), module, MatrixMixer4::ROW3_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.426, 91.51)), module, MatrixMixer4::ROW4_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.5, 115.0)), module, MatrixMixer4::COL1_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.17, 115.0)), module, MatrixMixer4::COL2_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.84, 115.0)), module, MatrixMixer4::COL3_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.51, 115.0)), module, MatrixMixer4::COL4_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(93.6, 115.0)), module, MatrixMixer4::XOR_CV_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.6, 35.5)), module, MatrixMixer4::OUT_OUTPUTS + 0));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.6, 54.17)), module, MatrixMixer4::OUT_OUTPUTS + 1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.6, 72.84)), module, MatrixMixer4::OUT_OUTPUTS + 2));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.6, 91.51)), module, MatrixMixer4::OUT_OUTPUTS + 3));

        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(33.5, 29.5)), module, MatrixMixer4::SMALL_LEDS + 0));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(50.17, 29.5)), module, MatrixMixer4::SMALL_LEDS + 1));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(66.84, 29.5)), module, MatrixMixer4::SMALL_LEDS + 2));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(83.51, 29.5)), module, MatrixMixer4::SMALL_LEDS + 3));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(33.5, 48.17)), module, MatrixMixer4::SMALL_LEDS + 4));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(50.17, 48.17)), module, MatrixMixer4::SMALL_LEDS + 5));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(66.84, 48.17)), module, MatrixMixer4::SMALL_LEDS + 6));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(83.51, 48.17)), module, MatrixMixer4::SMALL_LEDS + 7));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(33.5, 66.84)), module, MatrixMixer4::SMALL_LEDS + 8));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(50.17, 66.84)), module, MatrixMixer4::SMALL_LEDS + 9));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(66.84, 66.84)), module, MatrixMixer4::SMALL_LEDS + 10));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(83.51, 66.84)), module, MatrixMixer4::SMALL_LEDS + 11));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(33.5, 85.51)), module, MatrixMixer4::SMALL_LEDS + 12));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(50.17, 85.51)), module, MatrixMixer4::SMALL_LEDS + 13));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(66.84, 85.51)), module, MatrixMixer4::SMALL_LEDS + 14));
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(83.51, 85.51)), module, MatrixMixer4::SMALL_LEDS + 15));

        addChild(createParamCentered<LEDBezel>(mm2px(Vec(15.913, 35.5)), module, MatrixMixer4::ROW1_PARAM));
        addChild(createParamCentered<LEDBezel>(mm2px(Vec(15.573, 54.17)), module, MatrixMixer4::ROW2_PARAM));
        addChild(createParamCentered<LEDBezel>(mm2px(Vec(15.497, 72.84)), module, MatrixMixer4::ROW3_PARAM));
        addChild(createParamCentered<LEDBezel>(mm2px(Vec(15.497, 91.51)), module, MatrixMixer4::ROW4_PARAM));
        addChild(createParamCentered<LEDBezel>(mm2px(Vec(27.5, 105.9)), module, MatrixMixer4::COL1_PARAM));
        addChild(createParamCentered<LEDBezel>(mm2px(Vec(44.17, 105.9)), module, MatrixMixer4::COL2_PARAM));
        addChild(createParamCentered<LEDBezel>(mm2px(Vec(60.84, 105.9)), module, MatrixMixer4::COL3_PARAM));
        addChild(createParamCentered<LEDBezel>(mm2px(Vec(77.51, 105.9)), module, MatrixMixer4::COL4_PARAM));
        addChild(createParamCentered<LEDBezel>(mm2px(Vec(93.6, 105.9)), module, MatrixMixer4::XOR_PARAM));

        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(15.913, 35.5)), module, MatrixMixer4::ROW1_LIGHT));
        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(15.573, 54.17)), module, MatrixMixer4::ROW2_LIGHT));
        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(15.497, 72.84)), module, MatrixMixer4::ROW3_LIGHT));
        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(15.497, 91.51)), module, MatrixMixer4::ROW4_LIGHT));
        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(27.5, 105.9)), module, MatrixMixer4::COL1_LIGHT));
        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(44.17, 105.9)), module, MatrixMixer4::COL2_LIGHT));
        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(60.84, 105.9)), module, MatrixMixer4::COL3_LIGHT));
        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(77.51, 105.9)), module, MatrixMixer4::COL4_LIGHT));
        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(93.6, 105.90)), module, MatrixMixer4::XOR_LIGHT));
    }
};


Model* modelMatrixMixer4 = createModel<MatrixMixer4, MatrixMixer4Widget>("MatrixMixer4");
