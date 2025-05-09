<template>
  <div class="juce-combobox" @click="emit('openOverlay')" tabindex="0">
    <div class="combobox-display">
      {{ selectedLabel }}
      <span class="arrow">&#9662;</span>
    </div>
  </div>
</template>

<script setup lang="ts">

defineProps<{
  selectedLabel: string;
}>();

const emit = defineEmits<{
  (e: 'openOverlay'): void;
}>();

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
}

.arrow {
  margin-left: 0.5em;
}
</style>
