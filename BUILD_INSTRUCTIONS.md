# IndieGuitarFX — Инструкция по сборке VST3

> ⚠️ **ВАЖНО: Сборка возможна ТОЛЬКО на Windows с Visual Studio 2022.**
> JUCE официально не поддерживает кросс-компиляцию (MinGW, Linux→Windows).
> Любые попытки собрать на Linux или через MinGW завершатся ошибкой.

---

## Что нужно установить на Windows

| Что | Где скачать | Примечание |
|-----|-------------|------------|
| Visual Studio 2022 | https://visualstudio.microsoft.com/downloads/ | Community (бесплатно); выбери workload **"Desktop development with C++"** |
| JUCE 7.0.9 | https://github.com/juce-framework/JUCE/releases/tag/7.0.9 | Скачай `juce-7.0.9-windows.zip`, распакуй в `C:\JUCE` |
| CMake 3.22+ | https://cmake.org/download/ | При установке выбери **"Add to system PATH"** |
| Git | https://git-scm.com/ | Нужен только если используешь способ 2 |

---

## Способ 1 — Автоматически через батник (проще всего)

1. Распакуй архив, зайди в папку `IndieGuitarFX\`
2. Запусти **`setup_windows.bat`** от имени Администратора
   (правая кнопка → «Запустить от имени администратора»)
3. Скрипт сам найдёт JUCE, сконфигурирует и предложит собрать
4. После сборки предложит скопировать VST3 в папку плагинов

---

## Способ 2 — Через Projucer (официальный инструмент JUCE)

Projucer идёт в комплекте с JUCE. Плюс: не нужен CMake.

1. Открой `C:\JUCE\extras\Projucer\Builds\VisualStudio2022\Projucer.sln`
   → Собери Projucer (Release x64) — это займёт 2-3 минуты
2. Запусти `Projucer.exe` из `C:\JUCE\extras\Projucer\Builds\VisualStudio2022\x64\Release\App\`
3. В Projucer: **File → Open** → выбери `IndieGuitarFX\IndieGuitarFX.jucer`
4. В Projucer: **File → Global Paths** → укажи путь к JUCE: `C:\JUCE`
5. Нажми **Save and Open in IDE** — откроется Visual Studio
6. В Visual Studio: выбери **Release | x64** → **Build → Build Solution**

---

## Способ 3 — Вручную через CMake (для продвинутых)

```bat
REM Запускай в Developer Command Prompt for VS 2022
cd C:\Projects\IndieGuitarFX
mkdir build_vs2022
cd build_vs2022

cmake .. -G "Visual Studio 17 2022" -A x64

cmake --build . --config Release
```

Если JUCE не в стандартном месте, добавь флаг:
```bat
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=D:\MyJUCE
```

---

## Где окажется готовый .vst3 файл

После успешной сборки:
```
IndieGuitarFX\build_vs2022\IndieGuitarFX_artefacts\Release\VST3\IndieGuitarFX.vst3\
```

---

## Установка в FL Studio

1. Скопируй папку `IndieGuitarFX.vst3` в:
   ```
   C:\Program Files\Common Files\VST3\
   ```
2. Открой FL Studio → **Options → Manage Plugins**
3. Убедись что `C:\Program Files\Common Files\VST3\` в списке путей
4. Нажми **Find more plugins**
5. Плагин появится как **IndieGuitarFX** в категории Effects/VST3

**На канале микшера:** щёлкни на слот эффекта (зелёная кнопка) → найди IndieGuitarFX в списке

---

## Распространённые ошибки и решения

| Ошибка | Причина | Решение |
|--------|---------|---------|
| `X11/extensions/Xrandr.h not found` | Сборка на Linux — это неправильно | Только Windows + VS2022 |
| `Could not find JUCE` | JUCE не в стандартном пути | Запусти батник, он спросит путь |
| `juceaide stuck` | CMake пробует Linux-генератор | Используй `-G "Visual Studio 17 2022"` |
| `LINK : fatal error LNK1181` | Архитектура x86 вместо x64 | Добавь `-A x64` в cmake команду |
| `juce_audio_plugin_client not found` | Старая версия JUCE | Нужен JUCE 7.0.5 или новее |

---

## Структура проекта

```
IndieGuitarFX/
├── CMakeLists.txt              ← главный файл сборки (CMake)
├── IndieGuitarFX.jucer         ← проект для Projucer (без CMake)
├── setup_windows.bat           ← автоматический скрипт сборки
├── BUILD_INSTRUCTIONS.md       ← этот файл
└── Source/
    ├── PluginProcessor.h/cpp   ← аудио обработка, параметры
    ├── PluginEditor.h/cpp      ← главный GUI
    ├── DSP/
    │   ├── Overdrive.h/cpp     ← клипинг (Tube Screamer / Blues Driver / Fuzz)
    │   ├── Chorus.h/cpp        ← хорус с stereo LFO
    │   ├── Reverb.h/cpp        ← ревербератор (Room/Hall/Plate/Spring)
    │   ├── Delay.h/cpp         ← пинг-понг дилэй
    │   ├── Compressor.h/cpp    ← компрессор с makeup gain
    │   ├── EQ.h/cpp            ← 4-полосный параметрический EQ
    │   └── ToneAnalyzer.h/cpp  ← анализ тона по аудио-файлу
    └── GUI/
        ├── KnobLookAndFeel.h/cpp   ← кастомные ручки, тёмная тема
        ├── EffectStrip.h/cpp       ← блок эффекта (заголовок + ручки)
        ├── ToneAnalyzerPanel.h/cpp ← панель анализа с drag & drop
        └── SpectrumAnalyzer.h/cpp  ← FFT-анализатор спектра

```

---

## Гайд по звуку: indie пресеты

### Indie Rock (The Strokes, Interpol, The National)
- **Overdrive:** Tube Screamer, Drive 0.35, Tone 0.55, Level 0.7
- **EQ:** Low +0dB, LowMid +2dB (400Hz), HiMid +1dB (2kHz), High 0dB
- **Compressor:** Threshold -22dB, Ratio 4:1, Attack 10ms, Gain +3dB
- **Reverb:** Room, Size 0.4, Mix 0.2
- **Delay:** 375ms, Feedback 0.35, Mix 0.25

### Indie Pop (Arctic Monkeys clean, Vampire Weekend, Bleachers)
- **Overdrive:** выключен или Tube Screamer Drive 0.1
- **Chorus:** Rate 0.8, Depth 0.5, Mix 0.4, Feedback 0.15
- **EQ:** High +3dB (8kHz), LowMid +1dB
- **Reverb:** Plate, Size 0.5, Mix 0.3, Width 0.9

### Shoegaze (My Bloody Valentine, Slowdive, Beach House)
- **Overdrive:** Fuzz, Drive 0.6, Tone 0.4
- **Chorus:** Rate 0.4, Depth 0.85, Mix 0.65
- **Reverb:** Hall, Size 0.9, Damping 0.3, Width 1.0, Mix 0.5
- **EQ:** Low +4dB, Mid -2dB

### Анализатор тона
1. Перетащи любой WAV/MP3/FLAC/AIFF на панель **TONE ANALYZER**
2. Дождись прогресс-бар до 100%
3. Нажми **APPLY TONE** — EQ, Drive, Chorus и Reverb подстраиваются автоматически
