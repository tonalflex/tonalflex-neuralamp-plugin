<template>
  <div class="juce-combobox" @click="emit('openOverlay')" tabindex="0">
    <div class="combobox-display">
      <span class="label-text">{{ selectedLabel }}</span>
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
  min-height: 25px;
  cursor: pointer;
  user-select: none;
  outline: none;
}

.combobox-display {
  padding: 8px;
  background: linear-gradient(to bottom, #444, #222);
  box-shadow: inset 0 2px 5px rgba(0, 0, 0, 0.5);
  border: 1px solid #333;
  font-size: 0.7rem;
  display: flex;
  justify-content: space-between;
  align-items: center;
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
}

.label-text {
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
  flex: 1; /* take all space except the arrow */
}

.arrow {
  font-size: 1.0rem;
  margin-left: 0.5em;
}
</style>
