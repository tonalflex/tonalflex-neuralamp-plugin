<template>
  <div class="juce-toggle">
    <!-- LED sprite above toggle -->
    <div
      v-if="ledImage"
      class="led-sprite"
      :style="{
        backgroundImage: `url('${ledImage}')`,
        backgroundPosition: `0px ${!modelValue ? `-${ledFrameHeight}px` : '0px'}`,
        backgroundSize: '100% auto'
      }"
    ></div>

    <!-- Toggle graphic -->
    <div
      v-if="toggleImage"
      class="toggle-sprite"
      :style="{
        backgroundImage: `url('${toggleImage}')`,
        backgroundPosition: `0px ${modelValue ? `-${frameHeight}px` : '0px'}`,
        backgroundSize: `100% auto`
      }"
      @click="toggle"
    ></div>

    <!-- Fallback checkbox -->
    <input
      v-else
      type="checkbox"
      :checked="modelValue"
      @change="onChange"
      class="toggle-input"
    />

    <!-- Label text under -->
    <span class="label-text">
      {{ label }}
    </span>
  </div>
</template>

<script setup lang="ts">

const props = defineProps<{
  label: string;
  modelValue: boolean;
  toggleImage?: string;
  ledImage?: string; 
}>();

const emit = defineEmits<{
  (e: "update:modelValue", value: boolean): void;
}>();

const frameHeight = 50;
const ledFrameHeight = 20;

function onChange(e: Event) {
  const target = e.target as HTMLInputElement;
  emit("update:modelValue", target.checked);
}

function toggle() {
  emit("update:modelValue", !props.modelValue);
}
</script>

<style scoped>
.juce-toggle {
  display: flex;
  min-height: 100px;
  flex-direction: column;
  align-items: center;
  gap: 8px;
}

/* Toggle background sprite */
.toggle-sprite {
  width: 50px; /* one frame width */
  height: 50px; /* one frame height */
  background-repeat: no-repeat;
  background-size: 100% auto;
  cursor: pointer;
  -webkit-tap-highlight-color: transparent;
  -webkit-touch-callout: none;
  outline: none;
}

.led-sprite {
  width: 20px;
  height: 20px;
  background-repeat: no-repeat;
  background-size: 100% auto;
}

/* Fallback checkbox input */
.toggle-input {
  width: 24px;
  height: 24px;
  cursor: pointer;
}

/* Label text under toggle */
.label-text {
  width: 50px;
  font-size: 0.65rem;
  font-weight: 500;
  text-align:center;
}
</style>