# Generatore Procedurale di Terreno 3D

Questo progetto implementa un motore di rendering 3D in C++ e OpenGL per la generazione e l'esplorazione di terreni. Supporta sia la generazione procedurale (tramite Perlin Noise) sia il caricamento di veri dati satellitari (DEM).

##  Requisiti e Dipendenze
Il progetto fa uso delle seguenti librerie:
* **OpenGL 4.1+** (Core Profile)
* **SFML 3.0** (Gestione finestra, eventi e caricamento texture)
* **ImGui & ImGui-SFML** (Interfaccia grafica utente)
* **GLM** (Matematica per vettori e matrici)
* **GLAD** (OpenGL Loader)

---
## Tappe
* **1. Matrice**
* **2. Movimento**
* **3. Perlin Noise**
* **4. Calcolo normali**
* **5. Luce**
* **6. DEM**
* **7. Biomi**
* **8. Texture**
* **9. Acqua Animata**
* **10. Skybox**

##  Compilazione ed Esecuzione



**1. Compilazione:**
Dal terminale, nella cartella radice del progetto, eseguire:
```bash
cmake -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build

//Esecuzione:
```bash
cd Cartella-TappaN
../build/TappaN


