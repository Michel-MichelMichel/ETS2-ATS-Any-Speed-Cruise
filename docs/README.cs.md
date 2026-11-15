<p align="center">
  <img src="../assets/logo.png" width="180" alt="Logo Any Speed Cruise">
</p>

# ETS2 & ATS – Any Speed Cruise

[English](../README.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Español](README.es.md) · [Polski](README.pl.md) · **Čeština**

Tempomat, který funguje i pod 30 km/h (20 mph) v **Euro Truck Simulator 2**
a **American Truck Simulator**.

## Co dělá

V obou hrách „nastavit“ a „obnovit“ pod 30 km/h nic nedělají. Je to
realistické, ale kamiony zrychlují pomalu. S tímto pluginem fungují vaše
obvyklé klávesy tempomatu při jakékoli rychlosti:

| Klávesa | Pod limitem hry | Nad ním |
| --- | --- | --- |
| Tempomat (nastavit) | Plugin drží aktuální rychlost plynem | Hra funguje jako obvykle |
| Obnovit | Plugin zrychlí na naposledy uloženou rychlost, i z místa | stejně |
| Zvýšit / snížit | Mění cílovou rychlost pluginu (výchozí krok 5 km/h) | stejně |

Jakmile kamion jede dost rychle, plugin zapne skutečný tempomat hry a ustoupí.
Plyn pustí hned, jak zabrzdíte, znovu stisknete klávesu tempomatu, zatáhnete
parkovací brzdu, zařadíte zpátečku, vypnete motor nebo otevřete nabídku.

## Proč plugin, a ne mod

Limit 30 km/h je napevno zapsaný ve spustitelném souboru hry; mod `.scs` ho
změnit nemůže. Plugin používá pouze oficiální SDK od SCS a nemění paměť ani
soubory hry.

## Instalace

1. Zavřete hru.
2. Stáhněte nejnovější zip ze stránky [Releases](../../../releases/latest).
3. Zkopírujte `any_speed_cruise.dll`, `any_speed_cruise.ini` a
   `any_speed_cruise_config.exe` do složky hry `bin\win_x64\plugins`
   (složku `plugins` vytvořte, pokud neexistuje).
4. Spusťte `any_speed_cruise_config.exe` a zadejte své klávesy.
5. Spusťte hru a potvrďte upozornění na funkce SDK (u každého pluginu je to
   normální).

Pro odinstalaci tyto tři soubory smažte.

## Výběr kláves

V `any_speed_cruise_config.exe` klikněte na **Change** a stiskněte klávesu
nebo tlačítko volantu / gamepadu. Hodnota se ihned uloží (Esc ruší) a hra ji
načte při příštím spuštění.

Plugin tyto klávesy pouze sleduje: **stejná klávesa musí být ve hře přiřazena
stejné funkci** (Možnosti > Klávesy a tlačítka).

Výchozí: numerická klávesnice `.`, `*`, `+` a `-` – rozpoznají se jen se
zapnutým Num Lockem.

## Dobré vědět

- Kontrolka tempomatu zůstává zhasnutá, dokud rychlost drží plugin.
- Plugin používá jen plyn: z kopce nebrzdí.
- „Nastavit“ na místě nic neudělá; „obnovit“ ano.
- Nejvýše 32 tlačítek na zařízení; bez POV přepínačů a tlačítek myši.
- Otestováno v ETS2 1.62. Připraveno pro ATS, ale tam **zatím netestováno**.

Řešení potíží, kompilace a údržba: viz [anglické README](../README.md).

## Vytvořila umělá inteligence

Celý projekt – plugin, nástroj, logo, dokumentaci i překlady – vytvořil
**Claude Fable 5.2** (Anthropic) v listopadu 2026 na žádost hráče, který jej
otestoval ve hře. Používáte na vlastní riziko. Licence [MIT](../LICENSE).
Projekt není spojen se SCS Software.
