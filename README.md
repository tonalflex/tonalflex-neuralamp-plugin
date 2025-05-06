# TonalFlex NeuralAmp Plugin

NeuralAmp is a JUCE-based VST3 plugin built for Tonalflex, powered by the NeuralAmpModeler project by @sdatkinson. This plugin brings the core NAM modeling engine into a modern, cross-platform audio plugin format with full support for dynamic .nam model loading, impulse response (IR) handling, and real-time parameter control.

## **⚡ Prerequisites**

Before you begin, ensure you have the following installed:

✅ **Development Tools**:

- [VS Code](https://code.visualstudio.com/) (Recommended IDE)
- [CMake](https://cmake.org/download/) (Build system)
- [Docker](https://www.docker.com/) (For cross-compilation)

✅ **Additional Dependencies**:

- [JUCE Framework](https://juce.com/) (Automatically fetched via CMake)
- [Git](https://git-scm.com/) (Version control)

💡 **Tip**: Use the **CMake Presets** included in this project for an easy, consistent setup!

---

## **📦 Embedded Cross-Compilation** _(ElkOS)_

Build the plugin for ElkOS using the prebuilt docker image specified in `docker-compose.yml`.

```sh
# Hint - You might get permission denied on the bin folder on linux.
# Run "sudo chown -R $USER:$USER ./bin" in the project root and rerun docker!
docker-compose up
```

Once the compilation completes, a compressed headless VST3 plugin for ElkOS running on ARM64 is accessible in the workspace `bin` directory.

_Note! You may need to add exectuable permission to the build script first time._

```
chmod +x ./script/build-arm64-headless.sh
```

---

## 📦 Native WebView UI Build _(Linux/MacOS/Windows)_

From the root of the project, run:

```sh
cd ui
npm install
npm run build
cd ..
```

This compiles the frontend and places the output in the correct directory for the plugin to include.

💡 This step is required for all platforms (Linux, macOS, Windows).

Alternatively, you can automate this step inside a wrapper script or CI pipeline.

---

## 🔨 Native Build Instructions _(Linux/MacOS/Windows)_

If you want to build the plugin for desktop, use:

```sh
# Option 1 - Debug Mode
cmake -B build -S . -DHEADLESS=OFF -DCMAKE_BUILD_TYPE=Debug

# Option 2 - Release Mode
cmake -B build -S . -DHEADLESS=OFF -DCMAKE_BUILD_TYPE=Release

# Note! For linux, you also need to specify the toolchain
-DCMAKE_TOOLCHAIN_FILE=cmake/linux-toolchain.cmake \

# Build Plugin
cmake --build build
```

_Note! Make sure you have selected a CMake Preset before first build, otherwise you may need to delete the build directory, choose preset and build again!_

### Run Desktop Standalone Plugin

After building, run:

```sh
# Debug
cd build/plugin/neuralamp_artefacts/Debug/Standalone

# Release
cd build/plugin/neuralamp_artefacts/Release/Standalone
```

```sh
# MacOS
open ./neuralamp.app

# Linux
chmod +x neuralamp
./neuralamp

# Windows (PowerShell)
./neuralamp.exe
```

### Run Desktop VST3 Plugin

To load and run the **VST3 version** of the plugin, you will need a **plugin host** such as a DAW (**Logic Pro, Reaper, Studio One, Cubase, Ableton Live, etc.**) or a standalone **VST3 plugin host** like **JUCE Audio Plugin Host** (can be built from the [JUCE library](https://juce.com)).

After building, the compiled **VST3 plugin** will be located at:

```sh
# Debug
build/plugin/neuralamp_artefacts/Debug/VST3/neuralamp.vst3

# Release
build/plugin/neuralamp_artefacts/Release/VST3/neuralamp.vst3
```

### Native Install the Plugin

Copy the **VST3 plugin** to the appropriate directory for your OS so that your DAW can detect it:

- **MacOS:** `~/Library/Audio/Plug-Ins/VST3/`
- **Windows:** `C:\Program Files\Common Files\VST3\`
- **Linux:** `~/.vst3/`

After copying, **rescan your plugins** in your DAW's settings and load the plugin.

---

## 🚀 CI/CD (Release) Instructions

### Release VST3 for Embedded System

_This is the headless VST3 plugin (DSP processor) that runs on the target embedded device._

```sh
# Replace v0.0.0 with actual version!
git tag v0.0.0
git push origin v0.0.0
```

_This triggers an NPM publish workflow, which builds the UI package and publishes it to the public `Tonalflex` NPM registry._

### Deploy WebUI to NPM

_This is the frontend bundle used in the embedded system’s remote UI._

```sh
# Replace ui-v0.0.0 with actual version!
git tag ui-v0.0.0
git push origin ui-v0.0.0
```

_This triggers a build workflow, which builds the plugin inside a Docker container and uploads the ZIP file to a GitHub release._

---

## 🔓 Licensing & JUCE Attribution

This project is licensed under the **GNU AGPLv3**.
It uses the **JUCE framework**, which is dual-licensed under AGPLv3 and the JUCE Proprietary License.

For more information, see [JUCE Licensing](https://juce.com/juce-6-licence).

---
