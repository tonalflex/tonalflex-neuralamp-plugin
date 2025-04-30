<template>
  <div class="plugin-container" :style="{ backgroundImage: `url(${bg})` }">
    <div class="control-panel">
      <div class="section-1">
        <SliderControl label="BASS" v-model="toneBass"  :knobImage="greenKnob" :frames="89"/>
        <SliderControl label="MID" v-model="toneMid" :knobImage="greenKnob" :frames="89"/>
        <SliderControl label="TREBLE" v-model="toneTreble" :knobImage="greenKnob" :frames="89"/>
      </div>
      <div class="divider"></div>
      <div class="section-2">
        <ToggleControl label="EQ" v-model="eqToggle" :toggleImage="metalSwitch" :frames="2"/>
        <ToggleControl label="IR" v-model="irToggle" :toggleImage="metalSwitch" :frames="2"/>
        <ToggleControl label="Gate" v-model="noiseGateToggle" :toggleImage="metalSwitch" :frames="2"/>
        <ToggleControl label="Normalize NAM" v-model="normalizeNamOutput" :toggleImage="metalSwitch" :frames="2"/>
        <ToggleControl label="Normalize IR" v-model="normalizeIrOutput" :toggleImage="metalSwitch" :frames="2"/>
      </div>
      <div class="divider"></div>
      <div class="section-3">
        <SliderControl label="INPUT" v-model="inputLevel" :knobImage="greenKnob" :frames="89"/>
        <SliderControl label="GATE" v-model="noiseGateThreshold" :knobImage="greenKnob" :frames="89"/>
        <SliderControl label="OUTPUT" v-model="outputLevel" :knobImage="greenKnob" :frames="89"/>
      </div>
      <div class="divider"></div>
      <div class="section-4">
        <ComboBoxControl label="Nam" v-model="selectedNamModel.value.value" :options="modelChoices.result.value || ['No Model']"/>
        <ComboBoxControl label="IR" v-model="selectedIR.value.value" :options="irChoices.result.value || ['No IR']"/>
      </div>
    </div>
    <div class="section-5">
         <img :src="neuralamp" width="100%"/>
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
const noiseGateThreshold = useParameter("noiseGateThreshold", "slider");
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
<style scoped>
.plugin-container {
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  background-size: 100% 100%;
  background-repeat: no-repeat;
  background-position: top left;
  overflow-x: hidden;
}

.control-panel{
  position:relative;
  display: flex;
  flex-direction: column;
  top:2%;
  width:96%;
  background: linear-gradient(to bottom, #444, #222);
  box-shadow: inset 0 2px 5px rgba(0, 0, 0, 0.5);
}

.section-1 {
  width: 100%;
  display: flex;
  justify-content: center;
  gap: 10%;
  padding: 10px;
  overflow: hidden;
}

.section-2 {
  width: 98%;
  display: flex;
  justify-content: center;
  gap: 2%;
  padding: 10px;
  overflow: hidden;
}

/* Bottom section */
.section-3 {
  width: 98%;
  display: flex;
  justify-content: center;
  gap: 5%;
  padding: 10px;
  overflow: hidden;
}

.section-4{
  width:98%;
  display:flex;
  background-color:rgba(0, 0, 0, 0.2);
}

.section-5{
  width: 96%;
  margin-bottom: 3%;
  display:flex;
  flex: 1;
}

.divider{
  width: 100%;
  height: 1px;
  background-color: rgb(88, 88, 88);
}
</style>
