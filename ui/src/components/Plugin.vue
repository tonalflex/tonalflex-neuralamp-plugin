<template>
  <div class="plugin-container" :style="{ backgroundImage: `url(${bg})` }">
    <div class="left-spacer"></div>
    <div class="main-section">
      <div class="top-spacer"></div>
      <div class="control-panel">
        <div class="section-1">
          <div class="combobox-section">
            <div class="left-section">
              <ComboBoxControl
                label="Nam"
                v-model="selectedNamModel.value.value"
                :selectedLabel="modelChoices.result.value?.[selectedNamModel.value.value] ?? 'No Model'"
                @openOverlay="toggleDropdown('nam')"
              />
            </div>
            <div class="right-section">
              <ComboBoxControl
                label="IR"
                v-model="selectedIR.value.value"
                :selectedLabel="irChoices.result.value?.[selectedIR.value.value] ?? 'No IR'"
                @openOverlay="toggleDropdown('ir')"
              />
            </div>
          </div>
        </div>
        <div v-if="dropdownOpen" class="comboList-section">
          <div
            v-for="(option, index) in dropdownOptions"
            :key="index"
            class="dropdown-option"
            :class="{ selected: isOptionSelected(index) }"
            @click="selectFromDropdown(index)"
          >
            {{ option }}
          </div>
        </div>
        <div v-if="!dropdownOpen" class="below-section">
          <div class="divider"></div>
          <div class="section-2">
            <SliderControl label="BASS" v-model="toneBass" :knobImage="greenKnob" :frames="89"/>
            <SliderControl label="MID" v-model="toneMid" :knobImage="greenKnob" :frames="89"/>
            <SliderControl label="TREBLE" v-model="toneTreble" :knobImage="greenKnob" :frames="89"/>
          </div>
          <div class="divider"></div>
          <div class="section-3">
            <ToggleControl label="EQ" v-model="eqToggle" :toggleImage="metalSwitch" :ledImage="greenLed" :frames="2"/>
            <ToggleControl label="IR" v-model="irToggle" :toggleImage="metalSwitch" :ledImage="greenLed" :frames="2"/>
            <ToggleControl label="Gate" v-model="noiseGateToggle" :toggleImage="metalSwitch" :ledImage="greenLed" :frames="2"/>
            <ToggleControl label="Normalize NAM" v-model="normalizeNamOutput" :toggleImage="metalSwitch" :ledImage="greenLed" :frames="2"/>
            <ToggleControl label="Normalize IR" v-model="normalizeIrOutput" :toggleImage="metalSwitch" :ledImage="greenLed" :frames="2"/>
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
    </div>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref, computed } from "vue";
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
import greenLed from "@/assets/green_led.png?inline"

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

const dropdownOpen = ref<'nam' | 'ir' | null>(null);
const dropdownOptions = computed(() => {
  if (dropdownOpen.value === 'nam') return modelChoices.result.value ?? ['No Model'];
  if (dropdownOpen.value === 'ir') return irChoices.result.value ?? ['No IR'];
  return [];
});

function toggleDropdown(type: 'nam' | 'ir') {
  dropdownOpen.value = dropdownOpen.value === type ? null : type;
}

function selectFromDropdown(index: number) {
  if (dropdownOpen.value === 'nam') selectedNamModel.value.value = index;
  if (dropdownOpen.value === 'ir') selectedIR.value.value = index;
  dropdownOpen.value = null;
}

function isOptionSelected(index: number): boolean {
  if (dropdownOpen.value === 'nam') return selectedNamModel.value.value === index;
  if (dropdownOpen.value === 'ir') return selectedIR.value.value === index;
  return false;
}

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
  background-size: 100% 100%;
  background-repeat: no-repeat;
  background-position: top left;
  color: limegreen;
  text-shadow: 0 0 2px rgb(44, 44, 44), 0 0 2px rgb(54, 54, 54);
  overflow: hidden;
}

.left-spacer{
  width: 2%;
  height: 100%;
}

.main-section{
  width:96%;
  height:96%;
}

.top-spacer{
  width: 98%;
  height: 2%;
}

.control-panel{
  display: flex;
  flex-direction: column;
  width:100%;
  height:100%;
  background: linear-gradient(to bottom, #444, #222);
  box-shadow: inset 0 2px 5px rgba(0, 0, 0, 0.5);
  overflow: hidden;
}

.section-1 {
  min-height: 40px;
  width:100%;
  display:flex;
  flex-direction: column;
}

.combobox-section{
  width:100%;
  height: 10%;
  display: flex;
  flex-direction: row;
}

.left-section{
  width: 50%;
}

.right-section{
  width: 50%;
}

.comboList-section {
  width: 100%;
  display: flex;
  flex: 1;
  flex-direction: column;
  background: rgb(34, 34, 34);
  color: limegreen;
  font-size: 0.7rem;
  overflow-y: auto;
}

.dropdown-option {
  padding: 12px;
}

.dropdown-option.selected {
  background-color: #313131;
}

.dropdown-option:hover {
  background-color: #585858;
}

.below-section{
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
}

.section-2 {
  width: 100%;
  min-height: 130px;
  display: flex;
  justify-content: center;
  gap: 5%;
  padding-top: 10px;
  padding-bottom: 10px;
  overflow: hidden;
  user-select: none;
}

/* Bottom section */
.section-3 {
  width: 100%;
  min-height: 120px;
  display: flex;
  justify-content: center;
  gap: 2%;
  padding-top: 10px;
  padding-bottom: 10px;
  overflow: hidden;
  user-select: none;
}

.section-4{
  width: 100%;
  min-height: 130px;
  display: flex;
  justify-content: center;
  gap: 5%;
  margin-top: 10px;
  margin-bottom: 10px;
  overflow: hidden;
  user-select: none;
}

.section-5{
  width: 100%;
  min-height: 100px;
  display: flex;
  flex: 1;
}

.divider{
  width: 100%;
  height: 1px;
  background-color: rgb(88, 88, 88);
}
</style>