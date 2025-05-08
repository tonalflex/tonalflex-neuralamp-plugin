<template>
  <div class="plugin-container" :style="{ backgroundImage: `url(${bg})` }">
    <div class="control-panel">
      <div class="section-1">
        <ComboBoxControl label="Nam" v-model="selectedNamModel.value.value" :options="modelChoices.result.value || ['No Model']"/>
        <ComboBoxControl label="IR" v-model="selectedIR.value.value" :options="irChoices.result.value || ['No IR']"/>
      </div>
      <div class="divider"></div>
      <div class="section-2">
        <SliderControl label="BASS" v-model="toneBass" :knobImage="greenKnob" :frames="89"/>
        <SliderControl label="MID" v-model="toneMid" :knobImage="greenKnob" :frames="89"/>
        <SliderControl label="TREBLE" v-model="toneTreble" :knobImage="greenKnob" :frames="89"/>
      </div>
      <div class="divider"></div>
      <div class="section-3">
        <ToggleControl label="EQ" v-model="eqToggle" :toggleImage="metalSwitch" :frames="2"/>
        <ToggleControl label="IR" v-model="irToggle" :toggleImage="metalSwitch" :frames="2"/>
        <ToggleControl label="Gate" v-model="noiseGateToggle" :toggleImage="metalSwitch" :frames="2"/>
        <ToggleControl label="Normalize NAM" v-model="normalizeNamOutput" :toggleImage="metalSwitch" :frames="2"/>
        <ToggleControl label="Normalize IR" v-model="normalizeIrOutput" :toggleImage="metalSwitch" :frames="2"/>
      </div>
      <div class="divider"></div>
      <div class="section-4">
        <SliderControl label="INPUT" v-model="inputLevel" :knobImage="greenKnob" :frames="89"/>
        <SliderControl label="GATE" v-model="noiseGate" :knobImage="greenKnob" :frames="89"/>
        <SliderControl label="OUTPUT" v-model="outputLevel" :knobImage="greenKnob" :frames="89"/>
      </div>
      <div class="divider"></div>
      <div class="section-5">
        <img :src="neuralamp" width="100%" />
      </div>
    </div>

  </div>
</template>

<script setup lang="ts">
import { onMounted } from "vue";
import SliderControl from "@components/controls/SliderControl.vue";
import ToggleControl from "@components/controls/ToggleControl.vue";
import ComboBoxControl from "@components/controls/ComboBoxControl.vue";
import { useParameter } from "@composables/useParameter";
import { useComboBoxParameter } from "@composables/useComboBoxParameter";
import { useFunction } from "@composables/useFunction";
import bg from "@/assets/NeuralampBG.png?inline";
import neuralamp from "@/assets/Neuralamp.png?inline";
import greenKnob from "@/assets/greenKNOB.png?inline"
import metalSwitch from "@/assets/switch_metal.png?inline"

// Plugin Parameters
const inputLevel = useParameter("inputLevel", "slider");
const outputLevel = useParameter("outputLevel", "slider");
const toneBass = useParameter("toneBass", "slider");
const toneMid = useParameter("toneMid", "slider");
const toneTreble = useParameter("toneTreble", "slider");
const noiseGate = useParameter("noiseGateThreshold", "slider");
const eqToggle = useParameter("eqToggle", "toggle");
const irToggle = useParameter("irToggle", "toggle");
const noiseGateToggle = useParameter("noiseGateToggle", "toggle");
const normalizeNamOutput = useParameter("normalizeNamOutput", "toggle");
const normalizeIrOutput = useParameter("normalizeIrOutput", "toggle");
const selectedNamModel = useComboBoxParameter("selectedNamModel");
const selectedIR = useComboBoxParameter("selectedIR");

// Plugin Native Functions
const modelChoices = useFunction<string[]>("getModelChoices");
const irChoices = useFunction<string[]>("getIRChoices");

onMounted(() => {
  modelChoices.invoke();
  irChoices.invoke();
});
</script>

<!-- Use style.css to change global styles such as font, colors, etc. -->