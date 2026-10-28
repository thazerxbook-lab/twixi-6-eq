# Twixi-6 EQ  (VST3 para FL Studio)

EQ parametrico de 6 bandas con analizador de espectro y estetica alien/OVNI.

## Funciones
- 6 bandas: Bell, Low Shelf, High Shelf, Low Cut, High Cut, Notch
- Cortes con pendiente 12 / 24 / 48 dB/oct (Butterworth)
- Modo por banda: Stereo / Mid / Side
- Nodos arrastrables en el grafico (X = frecuencia, Y = ganancia)
- Rueda del mouse sobre un nodo = Q
- Doble clic en espacio vacio = activa una banda ahi, doble clic en un nodo = la apaga
- Analizador de espectro post-EQ (con compensacion de pendiente) on/off
- Rango del grafico +-6 / 12 / 18 / 24 dB, ganancia de salida
- Automatizable desde FL Studio (todos los parametros)

## Opcion A: compilar en la nube (sin instalar nada)
1. Crea un repo en github.com y sube esta carpeta completa (incluida `.github`).
2. Pestana **Actions** -> "Build Twixi-6 EQ" -> espera ~10 min.
3. Descarga el artifact **Twixi-6-EQ-VST3-Windows** (contiene `Twixi-6 EQ.vst3`).

## Opcion B: compilar en tu PC (Windows)
Requisitos: Visual Studio 2022 (carga "Desarrollo de escritorio con C++"), CMake y Git.
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Resultado: `build\Twixi6EQ_artefacts\Release\VST3\Twixi-6 EQ.vst3`

## Instalar en FL Studio
1. Copia la carpeta/archivo `Twixi-6 EQ.vst3` a `C:\Program Files\Common Files\VST3\`
2. FL Studio -> Options -> Manage plugins -> **Find installed plugins** (Verify plugins).
3. Busca "Twixi-6 EQ" en el Plugin database / Effects.

## Licencia de JUCE
JUCE es GPLv3 o licencia comercial/personal. Si distribuyes el plugin, publica este codigo fuente (GPL) o usa una licencia de JUCE.
