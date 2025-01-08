# SuperWave
Programowy instrument muzyczny wykorzystujący oscylatory superwave utworzony za pomocą biblioteki JUCE w formatch VST3 oraz aplikacji samodzielnej.

![image](https://github.com/user-attachments/assets/206cd803-dd65-4ed3-bb16-255022b71a0c)

## Funkcjonalność
Syntezator wykorzystuje syntezę SuperWave, opartą na Roland JP-8000 oraz subtraktywną realizowaną w sposób zblizony do analogowych syntezatorów.
Instrument zawiera dwa oscylatory: SuperWave oraz Virtual Analog, dwa filtry będące emulacją analogowych modeli: SVF oraz Ladder, 3 wykładniczne obwiednie, 3 generatory LFO oraz macierz modulacji umożliwająca na jednoczesną modulację 6 parametrów z listy 20 dostępnych.
## Dokumenty
[Adam Szabo - How to Emulate the Super Saw] (https://www.adamszabo.com/internet/adam_szabo_how_to_emulate_the_super_saw.pdf)
## Instalacja
```
  $ git clone --recurse-submodules https://github.com/blaisely/SyntezatorSuperWave.git
  $ cd SyntezatorSuperWave
  $ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  $ cmake --build build --config Release
```
