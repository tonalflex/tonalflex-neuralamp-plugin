import { ref, watch, onUnmounted, type Ref } from "vue";
import { useAudioBackend } from "@composables/useAudioBackend";
import type { ComboBoxParameter } from "@backend/IAudioBackend";

export function useComboBoxParameter(identifier: string) {
    const backend = useAudioBackend();
    const param = backend.getParameterState(identifier, "comboBox") as ComboBoxParameter;
  
    const value = ref(param.getChoiceIndex()) as Ref<number>;
    const choices = ref<string[]>(param.getChoices());
  
    console.log(`ComboBox ${identifier}:`, {
      initialIndex: param.getChoiceIndex(),
      choices: param.getChoices(),
    });
  
    const id = param.valueChangedEvent?.addListener((v) => {
      console.log(`ComboBox ${identifier} changed:`, v);
      value.value = v;
    });
  
    if (id !== undefined) {
      onUnmounted(() => {
        console.log(`Removing listener for ${identifier}, id: ${id}`);
        param.valueChangedEvent?.removeListener(id);
      });
    }
  
    watch(value, (val, oldVal) => {
      if (val === oldVal) return;
      console.log(`Setting ${identifier} to index:`, val);
      param.setChoiceIndex(val);
    });
  
    return {
      value,
      choices,
    };
  }