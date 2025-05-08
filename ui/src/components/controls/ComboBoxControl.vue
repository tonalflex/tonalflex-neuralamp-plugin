<template>
  <div class="juce-combobox" @click="toggleOpen" tabindex="0" @blur="open = false">
    <div class="combobox-display">
      {{ options[modelValue] || 'Select...' }}
      <span class="arrow">&#9662;</span>
    </div>
    <div class="dropdown-list" v-if="open">
      <div
        v-for="(option, index) in options"
        :key="index"
        class="dropdown-option"
        :class="{ selected: index === modelValue }"
        @click.stop="select(index)"
      >
        {{ option }}
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue';

defineProps<{
  modelValue: number;
  options: string[];
}>();

const emit = defineEmits<{
  (e: 'update:modelValue', value: number): void;
}>();

const open = ref(false);

function toggleOpen() {
  open.value = !open.value;
}

function select(index: number) {
  emit('update:modelValue', index);
  open.value = false;
}
</script>

<style scoped>
.juce-combobox {
  position: relative;
  width: 100%;
  cursor: pointer;
  user-select: none;
  outline: none;
}

.combobox-display {
  padding: 8px;
  height: 30px;
  background: linear-gradient(to bottom, #444, #222);
  box-shadow: inset 0 2px 5px rgba(0, 0, 0, 0.5);
  border: 1px solid #333;
  font-size: 0.9rem;
  display: flex;
  justify-content: space-between;
  align-items: center;
  overflow:hidden;
}

.arrow {
  margin-left: 0.5em;
}

.dropdown-list {
  position: fixed;
  top: 20px;
  width:calc(96% - 2px);
  min-height: calc(96% - 45px);
  left: 2%;
  top: calc(2% + 45px);
  font-size: 0.9rem;
  z-index: 9999;
  background: #222;
  border: 1px solid #333;
  max-height: 200px;
  overflow-y: auto;
}

.dropdown-option {
  padding: 8px;
  background: #222;
}

.dropdown-option:hover,
.dropdown-option.selected {
  background: #444;
}
</style>
