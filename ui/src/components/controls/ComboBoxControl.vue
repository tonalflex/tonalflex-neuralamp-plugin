<template>
    <div class="juce-combobox">
      <span class="text-sm font-medium text-center w-full">
        {{ label }}
      </span>
      <select
        :value="modelValue"
        @change="onChange"
        class="w-48 p-1"
      >
        <option
          v-for="(option, index) in options"
          :key="index"
          :value="index"
        >
          {{ option }}
        </option>
      </select>
    </div>
  </template>
  
  <script setup lang="ts">
  defineProps<{
    label: string;
    modelValue: number;     // ComboBox uses selected index
    options: string[];       // List of choices
  }>();
  
  const emit = defineEmits<{
    (e: "update:modelValue", value: number): void;
  }>();
  
  function onChange(e: Event) {
  const target = e.target as HTMLSelectElement;
  const newValue = parseInt(target.value, 10);
  console.log("ComboBox changed:", newValue);
  emit("update:modelValue", newValue);
}
  </script>
  
  <style scoped>
  .juce-combobox {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
  }
  </style>
  