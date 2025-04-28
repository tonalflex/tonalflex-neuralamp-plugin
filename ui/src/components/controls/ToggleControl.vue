<template>
    <div class="juce-toggle">
      <span class="text-sm font-medium text-center w-full">
        {{ label }}: {{ modelValue ? "On" : "Off" }}
      </span>
      <input
        type="checkbox"
        :checked="modelValue"
        @change="onChange"
        class="w-6 h-6"
      />
    </div>
  </template>
  
  <script setup lang="ts">
  defineProps<{
    label: string;
    modelValue: boolean;
  }>();
  
  const emit = defineEmits<{
    (e: "update:modelValue", value: boolean): void;
  }>();
  
  function onChange(e: Event) {
    const target = e.target as HTMLInputElement;
    emit("update:modelValue", target.checked);
  }
  </script>
  
  <style scoped>
  .juce-toggle {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
  }
  </style>  