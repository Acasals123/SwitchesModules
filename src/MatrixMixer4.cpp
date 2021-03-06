#include "plugin.hpp"

struct MatrixMixer4 : Module {

    enum ParamIds {
        ENUMS(POT_PARAMS, 16),
        ENUMS(ROW_PARAMS, 4),
        ENUMS(COL_PARAMS, 4),
        MUTE_ALGO_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        ENUMS(IN_INPUTS, 4),
        ENUMS(ROW_CV_INPUTS, 4),
        ENUMS(COL_CV_INPUTS, 4),
        ALGO_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        ENUMS(OUT_OUTPUTS, 4),
        NUM_OUTPUTS
    };

    enum LightIds {
        ENUMS(SMALL_LEDS, 16),
        ENUMS(ROW_LEDS, 4),
        ENUMS(COL_LEDS, 4),
        ENUMS(ALGO_LEDS, 4),
        NUM_LIGHTS
    };

    // Amplitude algorithm:
    //   0 - Ducking (the more voices in a row, the less amplitude per voice)
    //   1 - Hard clipping 10 Vpp
    //   2 - No processing
    int amplitudeAlgorithm = 0;

    // Mute algorithm (do not use 0):
    //   0 - I repeat, DO NOT USE 0!
    //   1 - Force (default)
    //   2 - Flip-flop (it's like XOR)
    //   3 - Intersections (it's like AND)
    int muteAlgorithm = 1;

    dsp::BooleanTrigger rowTrigger[4];
    dsp::BooleanTrigger colTrigger[4];
    dsp::BooleanTrigger algoTrigger;
    bool ledMatrix[16];
    bool rowState[4];
    bool colState[4];

    void onAdd() override {
    }

    void onReset() override {
        reset();
    }

    void reset() {
        for (int i = 0; i < 16; i++) {
            ledMatrix[i] = true;
        }
        for (int i = 0; i < 4; i++) {
            rowState[i] = true;
            colState[i] = true;
        }
    }

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
        configParam(ROW_PARAMS + 0, 0.0, 1.0, 0.0, "Mute row 1");
        configParam(ROW_PARAMS + 1, 0.0, 1.0, 0.0, "Mute row 2");
        configParam(ROW_PARAMS + 2, 0.0, 1.0, 0.0, "Mute row 3");
        configParam(ROW_PARAMS + 3, 0.0, 1.0, 0.0, "Mute row 4");
        configParam(COL_PARAMS + 0, 0.0, 1.0, 0.0, "Mute col 1");
        configParam(COL_PARAMS + 1, 0.0, 1.0, 0.0, "Mute col 2");
        configParam(COL_PARAMS + 2, 0.0, 1.0, 0.0, "Mute col 3");
        configParam(COL_PARAMS + 3, 0.0, 1.0, 0.0, "Mute col 4");
        configParam(MUTE_ALGO_PARAM, 0.0, 1.0, 0.0, "Mute algorithm");
    }

    void process(const ProcessArgs& args) override {
        lightTheLeds();
        setAudio();
        setLightsState();
    }

    void lightTheLeds() {
        for (int i = 0; i < 16; i++) {
            if (ledMatrix[i]) {
                lights[SMALL_LEDS + i].setBrightness(0.9f);
            } else {
                lights[SMALL_LEDS + i].setBrightness(0.f);
            }
        }
        for (int i = 0; i < 4; i++) {
            if (rowState[i]) {
                lights[ROW_LEDS + i].setBrightness(0.9f);
            } else {
                lights[ROW_LEDS + i].setBrightness(0.f);
            }
            if (colState[i]) {
                lights[COL_LEDS + i].setBrightness(0.9f);
            } else {
                lights[COL_LEDS + i].setBrightness(0.f);
            }
        }
        for (int i = 0; i < 4; i++) {
            if (muteAlgorithm == i + 1) {
                lights[ALGO_LEDS + i].setBrightness(0.9f);
            } else {
                lights[ALGO_LEDS + i].setBrightness(0.f);
            }
        }
    }

    void setAudio() {
        for (int outputNumber = 0; outputNumber < 4; outputNumber++) {
            if (outputs[OUT_OUTPUTS + outputNumber].isConnected()) {
                float out = 0.f;
                int numberOfConnections = 0;

                for (int i = 0; i < 4; i++) {
                    if (inputs[IN_INPUTS + i].isConnected() &&
                            ledMatrix[4 * outputNumber + i]) {
                        out += inputs[IN_INPUTS + i]
                               .getVoltage() *
                               params[POT_PARAMS + (4 * outputNumber) + i]
                               .getValue();
                        numberOfConnections++;
                    }
                }

                if (amplitudeAlgorithm == 0) {
                    if (numberOfConnections == 2) {
                        out *= 0.5;
                    } else if (numberOfConnections == 3) {
                        out *= 0.33333;
                    } else if (numberOfConnections == 4) {
                        out *= 0.25;
                    }
                } else if (amplitudeAlgorithm == 1) {
                    out = clamp(out, -5.f, 5.f);
                }

                outputs[OUT_OUTPUTS + outputNumber].setVoltage(out);
            }
        }
    }

    void setLightsState() {
        if (algoTrigger.process(params[MUTE_ALGO_PARAM].getValue() +
                                inputs[ALGO_CV_INPUT].getVoltage())) {
            muteAlgorithm++;
            if (muteAlgorithm > 3) {
                muteAlgorithm = 1;
            }
        }

        for (int row = 0; row < 4; row++) {
            if (rowTrigger[row].process(params[ROW_PARAMS + row].getValue() +
                                   inputs[ROW_CV_INPUTS + row].getVoltage())) {
                rowState[row] = !rowState[row];
                for (int i = 0; i < 4; i++) {
                    int aLed = 4 * row + i;
                    if (muteAlgorithm == 1) {
                        ledMatrix[aLed] = rowState[row];
                    } else if (muteAlgorithm == 3) {
                        ledMatrix[aLed] = rowState[row] && colState[i];
                    } else {
                        ledMatrix[aLed] = !ledMatrix[aLed];
                    }
                }
            }
        }

        for (int col = 0; col < 4; col++) {
            if (colTrigger[col].process(params[COL_PARAMS + col].getValue() +
                                    inputs[COL_CV_INPUTS + col].getVoltage())) {
                colState[col] = !colState[col];
                for (int i = 0; i < 4; i++) {
                    int aLed = col + i * 4;
                    if (muteAlgorithm == 1) {
                        ledMatrix[aLed] = colState[col];
                    } else if (muteAlgorithm == 3) {
                        ledMatrix[aLed] = colState[col] && rowState[i];
                    } else {
                        ledMatrix[aLed] = !ledMatrix[aLed];
                    }
                }
            }
        }
    }

    json_t *dataToJson() override {
        json_t *rootJ = json_object();

        json_t* ledsJ = json_array();
        for (int i = 0; i < 16; i++) {
            json_t* ledJ = json_boolean(ledMatrix[i]);
            json_array_append_new(ledsJ, ledJ);
        }
        json_object_set_new(rootJ, "leds", ledsJ);

        json_t* rowsJ = json_array();
        for (int i = 0; i < 4; i++) {
            json_t* rowJ = json_boolean(rowState[i]);
            json_array_append_new(rowsJ, rowJ);
        }
        json_object_set_new(rootJ, "rows", rowsJ);

        json_t* colsJ = json_array();
        for (int i = 0; i < 4; i++) {
            json_t* colJ = json_boolean(colState[i]);
            json_array_append_new(colsJ, colJ);
        }
        json_object_set_new(rootJ, "cols", colsJ);

        json_object_set_new(rootJ, "amplitudeAlgorithm",
                            json_integer(amplitudeAlgorithm));

        json_object_set_new(rootJ, "muteAlgorithm",
                            json_integer(muteAlgorithm));

        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* ledsJ = json_object_get(rootJ, "leds");
        if (ledsJ) {
            for (int i = 0; i < 16; i++) {
                json_t* ledJ = json_array_get(ledsJ, i);
                if (ledJ) ledMatrix[i] = json_boolean_value(ledJ);
            }
        }

        json_t* rowsJ = json_object_get(rootJ, "rows");
        if (rowsJ) {
            for (int i = 0; i < 4; i++) {
                json_t* rowJ = json_array_get(rowsJ, i);
                if (rowJ) rowState[i] = json_boolean_value(rowJ);
            }
        }

        json_t* colsJ = json_object_get(rootJ, "cols");
        if (colsJ) {
            for (int i = 0; i < 4; i++) {
                json_t* colJ = json_array_get(colsJ, i);
                if (colJ) colState[i] = json_boolean_value(colJ);
            }
        }

        json_t *amplitudeAlgorithmJ = json_object_get(rootJ,
                                                      "amplitudeAlgorithm");
        if (amplitudeAlgorithmJ) {
            amplitudeAlgorithm = json_integer_value(amplitudeAlgorithmJ);
        }

        json_t *muteAlgorithmJ = json_object_get(rootJ, "muteAlgorithm");
        if (muteAlgorithmJ) {
            muteAlgorithm = json_integer_value(muteAlgorithmJ);
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
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.426, 35.5)), module, MatrixMixer4::ROW_CV_INPUTS + 0));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.426, 54.17)), module, MatrixMixer4::ROW_CV_INPUTS + 1));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.426, 72.84)), module, MatrixMixer4::ROW_CV_INPUTS + 2));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.426, 91.51)), module, MatrixMixer4::ROW_CV_INPUTS + 3));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.5, 115.0)), module, MatrixMixer4::COL_CV_INPUTS + 0));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.17, 115.0)), module, MatrixMixer4::COL_CV_INPUTS + 1));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.84, 115.0)), module, MatrixMixer4::COL_CV_INPUTS + 2));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.51, 115.0)), module, MatrixMixer4::COL_CV_INPUTS + 3));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(93.6, 115.0)), module, MatrixMixer4::ALGO_CV_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.6, 35.5)), module, MatrixMixer4::OUT_OUTPUTS + 0));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.6, 54.17)), module, MatrixMixer4::OUT_OUTPUTS + 1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.6, 72.84)), module, MatrixMixer4::OUT_OUTPUTS + 2));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.6, 91.51)), module, MatrixMixer4::OUT_OUTPUTS + 3));

        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(33.5, 29.5)), module, MatrixMixer4::SMALL_LEDS + 0));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(50.17, 29.5)), module, MatrixMixer4::SMALL_LEDS + 1));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(66.84, 29.5)), module, MatrixMixer4::SMALL_LEDS + 2));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(83.51, 29.5)), module, MatrixMixer4::SMALL_LEDS + 3));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(33.5, 48.17)), module, MatrixMixer4::SMALL_LEDS + 4));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(50.17, 48.17)), module, MatrixMixer4::SMALL_LEDS + 5));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(66.84, 48.17)), module, MatrixMixer4::SMALL_LEDS + 6));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(83.51, 48.17)), module, MatrixMixer4::SMALL_LEDS + 7));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(33.5, 66.84)), module, MatrixMixer4::SMALL_LEDS + 8));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(50.17, 66.84)), module, MatrixMixer4::SMALL_LEDS + 9));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(66.84, 66.84)), module, MatrixMixer4::SMALL_LEDS + 10));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(83.51, 66.84)), module, MatrixMixer4::SMALL_LEDS + 11));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(33.5, 85.51)), module, MatrixMixer4::SMALL_LEDS + 12));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(50.17, 85.51)), module, MatrixMixer4::SMALL_LEDS + 13));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(66.84, 85.51)), module, MatrixMixer4::SMALL_LEDS + 14));
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(83.51, 85.51)), module, MatrixMixer4::SMALL_LEDS + 15));

        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(15.5, 29.5)), module, MatrixMixer4::ROW_LEDS + 0));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(15.5, 48.17)), module, MatrixMixer4::ROW_LEDS + 1));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(15.5, 66.84)), module, MatrixMixer4::ROW_LEDS + 2));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(15.5, 85.51)), module, MatrixMixer4::ROW_LEDS + 3));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(27.5, 99.9)), module, MatrixMixer4::COL_LEDS + 0));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(44.17, 99.9)), module, MatrixMixer4::COL_LEDS + 1));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(60.84, 99.9)), module, MatrixMixer4::COL_LEDS + 2));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(77.51, 99.9)), module, MatrixMixer4::COL_LEDS + 3));

        addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(87.5, 105.0)), module, MatrixMixer4::ALGO_LEDS + 0));
        addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(87.5, 108.5)), module, MatrixMixer4::ALGO_LEDS + 1));
        addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(87.5, 112.0)), module, MatrixMixer4::ALGO_LEDS + 2));
        addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(87.5, 115.5)), module, MatrixMixer4::ALGO_LEDS + 3));

        addChild(createParamCentered<TL1105>(mm2px(Vec(15.5, 35.5)), module, MatrixMixer4::ROW_PARAMS + 0));
        addChild(createParamCentered<TL1105>(mm2px(Vec(15.5, 54.17)), module, MatrixMixer4::ROW_PARAMS + 1));
        addChild(createParamCentered<TL1105>(mm2px(Vec(15.5, 72.84)), module, MatrixMixer4::ROW_PARAMS + 2));
        addChild(createParamCentered<TL1105>(mm2px(Vec(15.5, 91.51)), module, MatrixMixer4::ROW_PARAMS + 3));
        addChild(createParamCentered<TL1105>(mm2px(Vec(27.5, 105.9)), module, MatrixMixer4::COL_PARAMS + 0));
        addChild(createParamCentered<TL1105>(mm2px(Vec(44.17, 105.9)), module, MatrixMixer4::COL_PARAMS + 1));
        addChild(createParamCentered<TL1105>(mm2px(Vec(60.84, 105.9)), module, MatrixMixer4::COL_PARAMS + 2));
        addChild(createParamCentered<TL1105>(mm2px(Vec(77.51, 105.9)), module, MatrixMixer4::COL_PARAMS + 3));
        addChild(createParamCentered<TL1105>(mm2px(Vec(93.6, 105.9)), module, MatrixMixer4::MUTE_ALGO_PARAM));
    }

    struct MatrixMixer4AmplitudeItem : MenuItem {
        MatrixMixer4 *module;
        int algo;
        void onAction(const event::Action &e) override {
            module->amplitudeAlgorithm = algo;
        }
        void step() override {
            rightText = (module->amplitudeAlgorithm == algo) ? "✔" : "";
        }
    };

    void appendContextMenu(Menu *menu) override {
        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        MatrixMixer4 *module = dynamic_cast<MatrixMixer4*>(this->module);
        assert(module);

        MenuLabel *themeLabel = new MenuLabel();
        themeLabel->text = "Amplitude Algorithm";
        menu->addChild(themeLabel);

        MatrixMixer4AmplitudeItem *item1 = new MatrixMixer4AmplitudeItem();
        item1->text = "Ducking (default)";
        item1->module = module;
        item1->algo = 0;
        menu->addChild(item1);

        MatrixMixer4AmplitudeItem *item2 = new MatrixMixer4AmplitudeItem();
        item2->text = "Hard Clipping 10 Vpp";
        item2->module = module;
        item2->algo = 1;
        menu->addChild(item2);

        MatrixMixer4AmplitudeItem *item3 = new MatrixMixer4AmplitudeItem();
        item3->text = "No processing";
        item3->module = module;
        item3->algo = 2;
        menu->addChild(item3);
    }
};


Model* modelMatrixMixer4 = createModel<MatrixMixer4, MatrixMixer4Widget>("MatrixMixer4");
