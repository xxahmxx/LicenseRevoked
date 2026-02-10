# 🚛 License Revoked - Hardcore Realism Plugin for ETS2 & ATS

![Platform](https://img.shields.io/badge/Platform-ETS2%20%7C%20ATS-orange)
![Type](https://img.shields.io/badge/Type-Telemetry%20Plugin%20(.dll)-blue)
![License](https://img.shields.io/badge/License-MIT-green)

**[ 🇬🇧 English Description ](#-english) | [ 🇵🇱 Opis po Polsku ](#-polski)**

---

<a name="english"></a>
## 🇬🇧 English

**License Revoked** is a hardcore telemetry plugin for **Euro Truck Simulator 2** and **American Truck Simulator** that introduces real consequences for bad driving. Unlike standard game fines which only cost money, this plugin tracks your traffic offenses and assigns penalty points.

**⚠️ WARNING: This is a "Permadeath" style mod for your driver's license. If you exceed the limit, your profile will become effectively unplayable (locked). Drive carefully!**

### 🌟 Key Features

* **Real Penalty System:** Based on strict European standards. You have a limit of **24 penalty points**.
    * Collision: **10 pts**
    * Driving Wrong Way: **8 pts**
    * Red Light Offence: **6 pts**
    * Speeding: **2 pts**
    * Other offences: **1 pt**
* **Persistent Saving:** Points are saved automatically per truck/profile (based on license plate). Restarting the game won't save you.
* **Time-Based Reset:** To clear your record, you must drive safely for **14 in-game days** without a single violation. Using ferries or sleeping counts towards this time, but you still have to behave!
* **Modern UI:** Features a custom, native overlay notification system that integrates seamlessly with the game's aesthetic.
* **Administrative Decision (GAME OVER):** Once you hit 24 points, an administrative order is issued. Your screen will be blocked by a "License Suspended" modal, forcing you to exit the game.

### 📥 Installation (Easy Method)

This plugin works for both ETS2 and ATS. The zip archive contains the correct folder structure.

1.  Open your game installation folder:
    * **ETS2:** `...\Steam\steamapps\common\Euro Truck Simulator 2\`
    * **ATS:** `...\Steam\steamapps\common\American Truck Simulator\`
    * *Steam Tip:* Right-click game in Steam -> Manage -> **Browse local files**.
2.  Open the downloaded **ZIP archive**.
3.  Drag and drop the **`bin`** folder from the ZIP archive directly into your main game folder.
    * *Note:* If Windows asks to merge folders, click **Yes**.
4.  Launch the game.
5.  You will see a warning: **"Request to use advanced SDK features"**. This is normal for plugins. Click **OK**.

### ❓ FAQ

**Q: I got banned (Game Over). How do I play again?**
A: That's the point of the mod! You lost your license. To play on that save again, you must manually delete the penalty save file located in the `bin/win_x64` folder (e.g., `lr_save_YOURPLATE.txt`).

**Q: Does it work with map mods (Promods, Coast to Coast, etc.)?**
A: Yes, it is map-agnostic. It works with any map and any truck.

---

<a name="polski"></a>
## 🇵🇱 Polski

**License Revoked** to hardcorowy plugin telemetryczny do **Euro Truck Simulator 2** oraz **American Truck Simulator**, który wprowadza realne konsekwencje za łamanie przepisów. W przeciwieństwie do zwykłych mandatów, które kosztują tylko wirtualne pieniądze, ten mod nalicza punkty karne.

**⚠️ OSTRZEŻENIE: To mod typu "Permadeath" dla twojego prawa jazdy. Jeśli przekroczysz limit punktów, Twój profil zostanie zablokowany. Jedź ostrożnie!**

### 🌟 Główne Funkcje

* **Realny taryfikator:** System wzorowany na surowych przepisach drogowych. Twój limit to **24 punkty karne**.
    * Spowodowanie kolizji: **10 pkt**
    * Jazda pod prąd: **8 pkt**
    * Czerwone światło: **6 pkt**
    * Przekroczenie prędkości: **2 pkt**
    * Inne wykroczenia: **1 pkt**
* **System zapisu:** Punkty są zapisywane permanentnie dla każdego profilu/ciężarówki (na podstawie tablicy rejestracyjnej). Restart gry nie wyzeruje licznika.
* **Zasada "Czystego Konta":** Aby wyzerować punkty, musisz jeździć bez żadnego mandatu przez **14 dni w grze**.
* **Nowoczesny Interfejs:** Plugin wyświetla estetyczne powiadomienia na środku ekranu (w stylu gry), informując o mandatach lub resecie punktów.
* **Decyzja Administracyjna (GAME OVER):** Po osiągnięciu 24 punktów otrzymasz natychmiastową decyzję o zatrzymaniu uprawnień. Ekran zostanie zablokowany, zmuszając do wyjścia z gry.

### 📥 Instalacja (Metoda Szybka)

Plugin działa zarówno w ETS2 jak i ATS. Paczka zawiera gotową strukturę katalogów.

1.  Otwórz główny folder instalacyjny gry:
    * **ETS2:** `...\Steam\steamapps\common\Euro Truck Simulator 2\`
    * **ATS:** `...\Steam\steamapps\common\American Truck Simulator\`
    * *Steam:* Prawy przycisk na grę -> Zarządzaj -> **Przeglądaj pliki lokalne**.
2.  Otwórz pobrane archiwum **ZIP**.
3.  Przeciągnij folder **`bin`** z archiwum prosto do głównego folderu gry.
    * *Uwaga:* Jeśli system zapyta o scalenie folderów, kliknij **Tak**.
4.  Uruchom grę.
5.  Zobaczysz komunikat: **"Request to use advanced SDK features"**. To normalne dla pluginów. Kliknij **OK**.

### ❓ FAQ

**P: Dostałem bana (Decyzja Administracyjna). Jak grać dalej?**
O: Taki jest cel moda! Straciłeś prawko. Aby odzyskać dostęp do profilu, musisz ręcznie usunąć plik zapisu punktów w folderze `bin/win_x64` (plik o nazwie np. `lr_save_TABLICA.txt`). Traktuj to jako ostateczność (cheat).

**P: Czy działa z modami na mapy (Promods, C2C itp.)?**
O: Tak, plugin działa niezależnie od mapy i ciężarówki.

---

### 👨‍💻 Development

Built with C++ using SCS Telemetry SDK.
* **Author:** xxahmxx
* **Version:** 1.0 (Release)
