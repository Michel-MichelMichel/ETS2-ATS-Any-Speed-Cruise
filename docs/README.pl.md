<p align="center">
  <img src="../assets/logo.png" width="180" alt="Logo Any Speed Cruise">
</p>

# ETS2 & ATS – Any Speed Cruise

[English](../README.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Español](README.es.md) · **Polski** · [Čeština](README.cs.md)

Tempomat, który działa poniżej 30 km/h (20 mph) w **Euro Truck Simulator 2**
i **American Truck Simulator**.

## Co robi

W obu grach „ustaw” i „wznów” nie działają poniżej 30 km/h. To realistyczne,
ale ciężarówki rozpędzają się powoli. Z tym pluginem zwykłe klawisze tempomatu
działają przy każdej prędkości:

| Klawisz | Poniżej limitu gry | Powyżej |
| --- | --- | --- |
| Tempomat (ustaw) | Plugin utrzymuje bieżącą prędkość pedałem gazu | Gra działa jak zwykle |
| Wznów | Plugin przyspiesza do ostatnio zapamiętanej prędkości, także z postoju | tak samo |
| Zwiększ / zmniejsz | Zmienia prędkość zadaną pluginu (domyślnie o 5 km/h) | tak samo |

Gdy tylko ciężarówka jedzie wystarczająco szybko, plugin włącza prawdziwy
tempomat gry i się wycofuje. Puszcza gaz natychmiast, gdy hamujesz, ponownie
naciskasz klawisz tempomatu, zaciągasz hamulec postojowy, wrzucasz wsteczny
bieg, gasisz silnik lub otwierasz menu.

## Dlaczego plugin, a nie mod

Limit 30 km/h jest zapisany na stałe w pliku wykonywalnym gry; mod `.scs` nie
może go zmienić. Plugin korzysta wyłącznie z oficjalnego SDK firmy SCS i nie
modyfikuje pamięci ani plików gry.

## Instalacja

1. Zamknij grę.
2. Pobierz najnowszy plik zip z zakładki [Releases](../../../releases/latest).
3. Skopiuj `any_speed_cruise.dll`, `any_speed_cruise.ini` i
   `any_speed_cruise_config.exe` do folderu gry `bin\win_x64\plugins`
   (utwórz `plugins`, jeśli nie istnieje).
4. Uruchom `any_speed_cruise_config.exe` i wskaż swoje klawisze.
5. Uruchom grę i zaakceptuj ostrzeżenie o funkcjach SDK (to normalne przy
   każdym pluginie).

Aby odinstalować, usuń te trzy pliki.

## Wybór klawiszy

W `any_speed_cruise_config.exe` kliknij **Change**, a potem naciśnij klawisz
albo przycisk kierownicy / pada. Wartość zapisuje się od razu (Esc anuluje),
a gra odczyta ją przy następnym uruchomieniu.

Plugin tylko obserwuje te klawisze: **ten sam klawisz musi być przypisany do
tej samej funkcji w grze** (Opcje > Klawisze i przyciski).

Domyślnie: klawiatura numeryczna `.`, `*`, `+` i `-` – rozpoznawane tylko przy
włączonym Num Lock.

## Warto wiedzieć

- Kontrolka tempomatu pozostaje zgaszona, dopóki prędkość utrzymuje plugin.
- Plugin używa tylko gazu: nie hamuje na zjazdach.
- „Ustaw” nie działa na postoju; „wznów” działa.
- Maksymalnie 32 przyciski na urządzenie; bez przełączników POV i przycisków
  myszy.
- Przetestowano w ETS2 1.62. Przygotowany dla ATS, ale **jeszcze tam
  nietestowany**.

Rozwiązywanie problemów, kompilacja i utrzymanie: zobacz
[README po angielsku](../README.md).

## Stworzone przez AI

Cały projekt – plugin, narzędzie, logo, dokumentacja i tłumaczenia – został
stworzony przez **Claude Fable 5.2** (Anthropic) w listopadzie 2026 r. na
prośbę gracza, który przetestował go w grze. Używasz na własną
odpowiedzialność. Licencja [MIT](../LICENSE). Projekt niezwiązany z SCS
Software.
