<template>
  <div class="plugin-container">
    <img :src="logo" alt="Logo" class="logo" /> 
    <SliderControl label="Input Level" v-model="inputLevel" />
    <SliderControl label="Output Level" v-model="outputLevel" />
    <SliderControl label="Tone Bass" v-model="toneBass" />
    <SliderControl label="Tone Mid" v-model="toneMid" />
    <SliderControl label="Tone Treble" v-model="toneTreble" />
    <SliderControl label="NoiseGate Level" v-model="noiseGateThreshold" />
    <ToggleControl label="EQ" v-model="eqToggle" />
    <ToggleControl label="Gate" v-model="noiseGateToggle" />
    <ToggleControl label="Normalize NAM" v-model="normalizeNamOutput" />
    <ToggleControl label="Normalize IR" v-model="normalizeIrOutput" />
    <ComboBoxControl label="Nam" v-model="selectedNamModel.value.value" :options="modelChoices.result.value || ['No Model']"/>
    <ComboBoxControl label="IR" v-model="selectedIR.value.value" :options="irChoices.result.value || ['No IR']"/>
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
import logo from "@/assets/logo.png?inline";

// Plugin Parameters
const inputLevel = useParameter("inputLevel", "slider");
const outputLevel = useParameter("outputLevel", "slider");
const toneBass = useParameter("toneBass", "slider");
const toneMid = useParameter("toneMid", "slider");
const toneTreble = useParameter("toneTreble", "slider");
const noiseGateThreshold = useParameter("noiseGateThreshold", "slider");
const eqToggle = useParameter("eqToggle", "toggle");
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
.logo {
  width: 60vw;
  max-width: 400px;
  height: auto;
}
.plugin-container {
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
  align-items: center;
  justify-content: center;
  height: 100%;
  overflow-y:auto;
}
</style>
