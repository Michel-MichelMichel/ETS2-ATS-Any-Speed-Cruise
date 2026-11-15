<p align="center">
  <img src="../assets/logo.png" width="180" alt="Logo Any Speed Cruise">
</p>

# ETS2 & ATS – Any Speed Cruise

[English](../README.md) · **Français** · [Deutsch](README.de.md) · [Español](README.es.md) · [Polski](README.pl.md) · [Čeština](README.cs.md)

Un régulateur de vitesse qui fonctionne en dessous de 30 km/h (20 mph) dans
**Euro Truck Simulator 2** et **American Truck Simulator**.

## Ce que ça fait

Dans les deux jeux, « conserver » et « reprendre » ne font rien en dessous de
30 km/h. C'est réaliste, mais les camions sont lents à lancer. Avec ce plugin,
vos touches habituelles fonctionnent à toute allure :

| Touche | Sous le seuil du jeu | Au-dessus |
| --- | --- | --- |
| Régulateur (conserver) | Le plugin tient la vitesse actuelle à l'accélérateur | Le jeu fait comme d'habitude |
| Reprendre | Le plugin accélère vers la dernière vitesse enregistrée, même à l'arrêt | idem |
| Augmenter / diminuer | Change la consigne du plugin (5 km/h par défaut) | idem |

Dès que le camion roule assez vite, le plugin enclenche le vrai régulateur du
jeu et lui rend la main. Il relâche l'accélérateur dès que vous freinez,
rappuyez sur la touche du régulateur, serrez le frein de parking, passez la
marche arrière, coupez le moteur ou ouvrez un menu.

## Pourquoi un plugin et pas un mod

Le seuil de 30 km/h est codé dans l'exécutable du jeu : un mod `.scs` ne peut
pas le modifier. Ce plugin n'utilise que le SDK officiel de SCS, sans toucher à
la mémoire ni aux fichiers du jeu.

## Installation

1. Fermer le jeu.
2. Télécharger le dernier zip dans [Releases](../../../releases/latest).
3. Copier `any_speed_cruise.dll`, `any_speed_cruise.ini` et
   `any_speed_cruise_config.exe` dans le dossier `bin\win_x64\plugins` du jeu
   (créer `plugins` s'il n'existe pas).
4. Lancer `any_speed_cruise_config.exe` pour indiquer vos touches.
5. Lancer le jeu et valider l'avertissement sur les fonctionnalités SDK
   (normal avec tout plugin).

Pour désinstaller, supprimer les trois fichiers.

## Choisir les touches

Dans `any_speed_cruise_config.exe`, cliquer sur **Change**, puis appuyer sur
une touche du clavier ou un bouton de volant / manette. La valeur est
enregistrée aussitôt (Échap annule) et le jeu la lit à son prochain lancement.

Le plugin ne fait que surveiller ces touches : **la même touche doit être
affectée à la même fonction dans le jeu** (Options > Touches et boutons).

Par défaut : pavé numérique `.`, `*`, `+` et `-`, reconnus seulement si
Verr. Num est activé.

## Bon à savoir

- Le voyant du régulateur reste éteint tant que c'est le plugin qui tient la
  vitesse.
- Le plugin n'agit que sur l'accélérateur : il ne freine pas en descente.
- « Conserver » à l'arrêt ne fait rien ; « reprendre » fonctionne.
- 32 boutons maximum par périphérique ; pas de croix directionnelle ni de
  bouton de souris.
- Testé sur ETS2 1.62. Conçu pour ATS mais **pas encore testé** sur ce jeu.

Dépannage, compilation et maintenance : voir le [README anglais](../README.md).

## Créé par une IA

Tout le projet – plugin, outil, logo, documentation et traductions – a été
créé par **Claude Fable 5.2** (Anthropic) en novembre 2026, à la demande d'un
joueur qui l'a testé en jeu. À utiliser à vos risques. Licence
[MIT](../LICENSE). Projet non affilié à SCS Software.
